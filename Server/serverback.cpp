#include "serverback.hpp"

#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonArray>

#include <QByteArray>

ServerBack::ServerBack(Server *ui, QObject *parent) :
    QTcpServer(parent),
    gui(ui),
    m_block_size(0),
    m_database(Database::get_instance()),
    m_encryption(Encryption::get_instance())
{
    if (!listen(QHostAddress::Any, 1326))
        gui->showErrorAndExit("Port listening error");
    gui->show();
}


void ServerBack::incomingConnection(qintptr socketDescriptor)
{
    QTcpSocket *socket = new QTcpSocket(this);
    m_messages[socket] = nullptr;

    // The descriptor is a positive number that identifies
    // the input/output stream
    socket->setSocketDescriptor(socketDescriptor);
    connect(socket, &QTcpSocket::readyRead,    this, &ServerBack::slotReadyRead);
    connect(socket, &QTcpSocket::disconnected, this, &ServerBack::disconnectClient);
}

void ServerBack::disconnectClient()
{
    QTcpSocket *curr_socket = qobject_cast<QTcpSocket*>(sender());
    m_messages.remove(curr_socket);

    auto it  = m_sockets.begin();
    for (; it != m_sockets.end(); ++it)
        if (it.value() == curr_socket)
            break;

    if (it != m_sockets.end()) {
        gui->offline_user(it.key());
        m_sockets.erase(it);
    }
    curr_socket->deleteLater();
}

void ServerBack::sendToClient(const QJsonObject& message, QTcpSocket *client) const
{
    QByteArray data;
    if (message["type"] == "request a key") {
        QJsonObject answer;
        answer["type"] = "key";
        answer["key"]  = QString::fromUtf8(m_encryption->get_n());
        data = QJsonDocument(answer).toJson(QJsonDocument::Compact);
    }
    else if (message["type"] == "key") {
        QByteArray cipher_key = message["key"].toString().toUtf8();
        data = QJsonDocument(*m_messages[client]).toJson(QJsonDocument::Compact);

        data = m_encryption->encode(data, cipher_key);
        m_messages[client] = nullptr;
    }

    // Executed when m_messages[client] is empty
    else {
        m_messages[client] = std::make_shared<QJsonObject>(message);
        QJsonObject request;
        request["type"] = "request a key";
        data = QJsonDocument(request).toJson(QJsonDocument::Compact);
    }
    // There is no fourth. Because if the message is NOT empty,
    // then we have an encryption key (second block)

    QDataStream out(client);

    // To avoid errors, as it is constantly updated
    out.setVersion(QDataStream::Qt_5_15);

    // Write the size of transferred data in the SAME TYPE AS m_block_size,
    // otherwise data will not be transferred correct!!!
    out << decltype(m_block_size)(data.size());

    // Writing data as "raw bytes" with size 'data.size()'
    out.writeRawData(data.constData(), data.size());

    // Forcing all data to be sent at once
    // to avoid multithreading problems when data accumulate in the buffer
    client->flush();
}

void ServerBack::slotReadyRead()
{
    QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());

    // First of all we read the size of the message to be transmitted
    QDataStream in(socket);
    in.setVersion(QDataStream::Qt_5_15);
    if (in.status() != QDataStream::Ok)
        gui->showErrorAndExit("DataStream error");

    if (m_block_size == 0) {
        if (socket->bytesAvailable() < static_cast<qint64>(sizeof(m_block_size)))
            return;
        in >> m_block_size;
    }

    // if the data came in less than agreed
    if (socket->bytesAvailable() < m_block_size)
        return;

    // when we got the size, then we get our data
    QByteArray data = socket->read(m_block_size);

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);

    // If we caught the first error, then our data came in encrypted form
    // Otherwise we get the key
    if (error.error != QJsonParseError::NoError) {
        data = m_encryption->decode(data);
        doc = QJsonDocument::fromJson(data, &error);

        // If after decryption we get an error
        if (error.error != QJsonParseError::NoError)
            gui->showErrorAndExit("Data reading error or key error");
    }
    QJsonObject message = doc.object();

    // Reset the variable to zero
    // so that we can read the following message
    m_block_size = 0;
    try {
        determineMessage(message, socket);
    }
    catch(const QSqlError& error) {
        gui->showErrorAndExit("Caught SQL error in func " + error.text());
    }
    catch (const std::exception& error) {
        gui->showErrorAndExit("Caught exception: " + QString(error.what()));
    }
    catch(...) {
        gui->showErrorAndExit("Caught unknown exception");
    }
}

void ServerBack::determineMessage(const QJsonObject& message, QTcpSocket *socket)
{
    // EVERYWHERE THERE IS A socket, IT USED TO BE m_socket!!!
    QJsonObject feedback;
    if (message["type"] == "message" && authorizedAccess(message["from"].toString(), socket)) {
        if (message["message"].toString().isEmpty()) return;
        feedback = sendMessage(message);

        if (message["from"] != message["to"] &&
                m_sockets.contains(message["to"].toString()))
            sendToClient(feedback, m_sockets[message["to"].toString()]);

        m_database->addMessage(message);
    }

    // This is where we supposedly send the key back.
    // However, in this function we encrypt m_message
    // and send fully encrypted bytes
    else if (message["type"] == "key" ||
             message["type"] == "request a key")
        sendToClient(message, socket);

    else if (message["type"] == "download correspondence" &&
             authorizedAccess(message["username"].toString(), socket)) {

        feedback = m_database->getMessages(
                    message["username"].toString(), message["with"].toString());

        feedback["type"] = message["type"];
        feedback["with"] = message["with"];
        sendToClient(feedback, socket);
    }

    else if (message["type"] == "update online users" &&
             authorizedAccess(message["username"].toString(), socket))
        updatingOnlineUsers(socket);

    else if (message["type"] == "download chats" &&
             authorizedAccess(message["username"].toString(), socket)) {
        feedback["type"] = message["type"];
        feedback["array of users"] = m_database->getChats(message["username"].toString());
        sendToClient(feedback, socket);
    }

    else if (message["type"] == "login" ||
             message["type"] == "registration") {
        feedback = regLogValidation(message);
        if (feedback["isCorrect"].toBool())
            successEntry(message["username"].toString(), socket);
        sendToClient(feedback, socket);
    }
}

// If such a user is not on the network or his socket is different,
// then it is unauthorized access
bool ServerBack::authorizedAccess(const QString& username, const QTcpSocket *socket) const {
    if (!m_sockets.contains(username)) return false;

    // User logged in illegally, so the socket in memory for him is different
    if (m_sockets[username] != socket) return false;
    return true;
}

// Here need to display a message indicating
// that there is a new user online
void ServerBack::successEntry(const QString& username, QTcpSocket *socket) {
    gui->online_user(username);
    m_sockets[username] = socket;
}

// Explanation of the error in the json line "feedback"
QJsonObject ServerBack::regLogValidation(const QJsonObject& message)
{
    static constexpr unsigned short min_length {4};
    static constexpr unsigned short max_length {64};

    QJsonObject feedback;
    feedback["type"] = message["type"];
    if (message["username"].toString().isEmpty() || message["password"].toString().isEmpty()) {
        feedback["isCorrect"] = false;
        feedback["feedback"] = "You need to fill in both input fields";
        return feedback;
    }
    if (message["password"].toString().length() < min_length) {
        feedback["isCorrect"] = false;
        feedback["feedback"] = "The password must contain at least 4 characters";
        return feedback;
    }
    if (message["username"].toString().length() > max_length) {
        feedback["isCorrect"] = false;
        feedback["feedback"] = "Username is too long";
        return feedback;
    }
    if (message["password"].toString().length() > max_length) {
        feedback["isCorrect"] = false;
        feedback["feedback"] = "Password is too long";
        return feedback;
    }
    if (m_sockets.find(message["username"].toString()) != m_sockets.end()) {
        feedback["isCorrect"] = false;
        feedback["feedback"]  = "User is online now";
        return feedback;
    }

    // Check for validation type
    if (message["type"] == "registration") {
        if (!m_database->registrationValidation(message)) {
            feedback["isCorrect"] = false;
            feedback["feedback"]  = "This username is already taken";
            return feedback;
        }

        QJsonArray arr;
        for (auto it = m_sockets.begin(); it != m_sockets.end(); ++it)
            arr.append(it.key());

        feedback["isCorrect"] = true;
        feedback["feedback"]  = arr;
        return feedback;
    }

    // Login Validation
    if (!m_database->loginValidation(message)) {
        feedback["isCorrect"] = false;
        feedback["feedback"]  = "Incorrect nickname or password";
        return feedback;
    }
    feedback["isCorrect"] = true;
    return feedback;
}

void ServerBack::updatingOnlineUsers(QTcpSocket *client) const
{
    QJsonObject json;
    json["type"] = "update online users";

    QJsonArray arr;
    for (auto it = m_sockets.begin(); it != m_sockets.end(); ++it)
        arr.append(it.key());

    json["array of users"] = arr;
    sendToClient(json, client);
}

QJsonObject ServerBack::sendMessage(const QJsonObject &message)
{
    QJsonObject feedback;
    feedback["type"]    = message["type"];
    feedback["from"]    = message["from"];
    feedback["message"] = message["message"];
    return feedback;
}
