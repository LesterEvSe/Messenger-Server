#include "serverback.hpp"

#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonArray>
#include <QByteArray>
#include <QFileInfo>

#include <QMessageBox>
#include <QDebug> // Need to delete later

ServerBack::ServerBack(Server *ui, QObject *parent) :
    QTcpServer(parent),
    gui(ui),
    m_block_size(0),
    m_socket(nullptr)
{
    m_database = std::make_unique<Database>();

    if (listen(QHostAddress::Any, 1326))
        qDebug() << "Start listening port 1326...";
    else
        qDebug() << "Error";

    ui->show();
}


void ServerBack::incomingConnection(qintptr socketDescriptor)
{
    m_socket = new QTcpSocket(this);

    // The descriptor is a positive number that identifies
    // the input/output stream
    m_socket->setSocketDescriptor(socketDescriptor);
    connect(m_socket, &QTcpSocket::readyRead, this, &ServerBack::slotReadyRead);
    connect(m_socket, &QTcpSocket::disconnected, this, [=](){
        auto it = m_sockets.begin();
        QTcpSocket *curr_socket = qobject_cast<QTcpSocket*>(sender());

        for (; it != m_sockets.end(); ++it)
            if (it.value() == curr_socket)
                break;

        if (it != m_sockets.end()) {
            gui->offline_user(it.key());
            m_sockets.erase(it);
        }
        curr_socket->deleteLater();
    });
}

void ServerBack::sendToClient(const QJsonObject& message, QTcpSocket *client) const
{
    QByteArray data = QJsonDocument(message).toJson(QJsonDocument::Compact);
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
    m_socket = qobject_cast<QTcpSocket*>(sender());

    // First of all we read the size of the message to be transmitted
    QDataStream in(m_socket);
    in.setVersion(QDataStream::Qt_5_15);
    if (in.status() != QDataStream::Ok) {
        qDebug() << "DataStream error";
//        QMessageBox::critical(this, "Error", "DataStream error");
        return;
    }

    if (m_block_size == 0) {
        if (m_socket->bytesAvailable() < static_cast<qint64>(sizeof(m_block_size)))
            return;
        in >> m_block_size;
    }

    // if the data came in less than agreed
    if (m_socket->bytesAvailable() < m_block_size)
        return;

    // when we got the size, then we get our data
    QByteArray data = m_socket->read(m_block_size);
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);

    if (error.error != QJsonParseError::NoError) {
        qDebug() << "Json parse error: " << error.errorString();
//        QMessageBox::critical(this, "Error", error.errorString());
        return;
    }
    QJsonObject message = doc.object();

    // Reset the variable to zero
    // so that we can read the following message
    m_block_size = 0;

    QJsonObject feedback;
    if (message["type"] == "message") {
        if (message["message"].toString().isEmpty()) return;
        feedback = sendMessage(message);

        /// The acknowledgment pattern has to be applied here
        /// to confirm that the message is written to the DB,
        /// so we have to send the JSON after all. DO IT!!!

        // If sender == recipient or recipient offline
        if (message["from"] == message["to"] ||
                m_sockets.find(message["to"].toString()) == m_sockets.end());
        // ; is plug for future
            /// Here acknowledgment

        // Recepient online
        else
            /// and here acknowledgment
            sendToClient(feedback, m_sockets[message["to"].toString()]);

        m_database.get()->addMessage(message);
    }
    else if (message["type"] == "update online user") {
        updatingOnlineUsers(m_socket);
    }
    else if (message["type"] == "login") {
        feedback = login(message);
        if (feedback["isCorrect"].toBool())
            successEntry(message["username"].toString());
        sendToClient(feedback, m_socket);
    }
    else if (message["type"] == "registration") {
        feedback = registration(message);
        if (feedback["isCorrect"].toBool())
            successEntry(message["username"].toString());
        sendToClient(feedback, m_socket);
    }
}

// Here need to display a message indicating
// that there is a new user online
void ServerBack::successEntry(const QString& username) {
    gui->online_user(username);
    m_sockets[username] = m_socket;
}



QJsonObject ServerBack::registration(const QJsonObject &message)
{
    QJsonObject feedback;
    feedback["type"] = message["type"];
    if (message["username"].toString().isEmpty() || message["password"].toString().isEmpty()) {
        feedback["isCorrect"] = false;
        feedback["feedback"] = "You need to fill in both input fields";
        return feedback;
    }
    if (message["password"].toString().length() < 4) {
        feedback["isCorrect"] = false;
        feedback["feedback"] = "The password must contain at least 4 characters";
        return feedback;
    }

    if (message["username"].toString().length() > 64) {
        feedback["isCorrect"] = false;
        feedback["feedback"] = "Username is too long";
        return feedback;
    }

    if (message["password"].toString().length() > 64) {
        feedback["isCorrect"] = false;
        feedback["feedback"] = "Password is too long";
        return feedback;
    }

    if (!m_database.get()->registrationValidation(message, feedback))
        return feedback;

    QJsonArray arr;
    for (auto it = m_sockets.begin(); it != m_sockets.end(); ++it)
        arr.append(it.key());

    feedback["isCorrect"] = true;
    feedback["feedback"]  = arr;
    return feedback;
}

// Explanation of the error in the json line "feedback"
QJsonObject ServerBack::login(const QJsonObject &message)
{
    QJsonObject feedback;
    feedback["type"] = message["type"];
    if (message["username"].toString().isEmpty() || message["password"].toString().isEmpty()) {
        feedback["isCorrect"] = false;
        feedback["feedback"] = "You need to fill in both input fields";
        return feedback;
    }
    if (message["password"].toString().length() < 4) {
        feedback["isCorrect"] = false;
        feedback["feedback"] = "The password must contain at least 4 characters";
        return feedback;
    }

    if (m_sockets.find(message["username"].toString()) != m_sockets.end()) {
        feedback["isCorrect"] = false;
        feedback["feedback"]  = "User is online now";
        return feedback;
    }

    if (!m_database.get()->loginValidation(message, feedback))
        return feedback;

    feedback["isCorrect"] = true;
    return feedback;
}

void ServerBack::updatingOnlineUsers(QTcpSocket *client) const
{
    QJsonObject json;
    json["type"] = "update online user";

    QJsonArray arr;
    for (auto it = m_sockets.begin(); it != m_sockets.end(); ++it)
        arr.append(it.key());

    json["user array"] = arr;
    sendToClient(json, client);
}

// Need to implement
QJsonObject ServerBack::sendMessage(const QJsonObject &message)
{
    QJsonObject feedback;
    feedback["type"]    = message["type"];
    feedback["from"]    = message["from"];
    feedback["message"] = message["message"];
    return feedback;
}
