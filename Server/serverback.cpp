#include "serverback.hpp"

#include <QJsonDocument>
#include <QJsonParseError>
#include <QByteArray>
#include <QFileInfo>

#include <QMessageBox>
#include <QDebug> // Need to delete later

ServerBack::ServerBack(QObject *parent) :
    QTcpServer(parent),
    m_block_size(0),
    m_database(QSqlDatabase::addDatabase("QSQLITE")),
    m_socket(nullptr)
{
    QString DBName = "messenger.sqlite";
    m_database.setDatabaseName(DBName);

    if (!m_database.open()) {
        qDebug() << "Failed to connect to database: " << m_database.lastError();
    }

    QSqlQuery query("SELECT name FROM sqlite_master WHERE type='table' AND name='users';");
    if (!query.next() && !query.exec("CREATE TABLE users (username TEXT, password TEXT)")) {
        qDebug() << "Failed to create table: " << query.lastError();
    }

    if (listen(QHostAddress::Any, 1326))
        qDebug() << "Start listening port 1326...";
    else
        qDebug() << "Error";
}

void ServerBack::incomingConnection(qintptr socketDescriptor)
{
    m_socket = new QTcpSocket(this);

    // The descriptor is a positive number that identifies
    // the input/output stream
    m_socket->setSocketDescriptor(socketDescriptor);
    connect(m_socket, &QTcpSocket::readyRead, this, &ServerBack::slotReadyRead);
    connect(m_socket, &QTcpSocket::disconnected, this, [=](){
        qDebug() << "disconnected" << socketDescriptor;
        m_socket->deleteLater();
    });

    qDebug() << "Client connected" << socketDescriptor;
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
        feedback = sendMessage(message);
        sendToClient(feedback, Sockets[message["to"].toString()]);
    }
    else if (message["type"] == "login") {
        feedback = login(message);
        if (feedback["isCorrect"].toBool()) {
            // Here need to display a message indicating
            // that there is a new user online

            Sockets[feedback["username"].toString()] = m_socket;
        }
        sendToClient(feedback, m_socket);
    }
    else if (message["type"] == "registration") {
        feedback = registration(message);
        if (feedback["isCorrect"].toBool()) {
            // Here need to display a message indicating
            // that there is a new user online

            Sockets[feedback["username"].toString()] = m_socket;
        }
        sendToClient(feedback, m_socket);
    }
    else
        // For users who want to break the system
        return;
}

void ServerBack::sendToClient(const QJsonObject& message, QTcpSocket *client)
{
    QByteArray data = QJsonDocument(message).toJson(QJsonDocument::Compact);
    QDataStream out(m_socket);
    out.setVersion(QDataStream::Qt_5_15);

    out << quint16(data.size());
    out.writeRawData(data.constData(), data.size());

    // Forcing all data to be sent at once
    // to avoid multithreading problems when data accumulate in the buffer
    client->flush();
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


    // The following validations for the DataBase
    m_query.prepare("SELECT password FROM users WHERE username = :username");
    m_query.bindValue(":username", message["username"].toString());

    // Problem with query
    if (!m_query.exec()) {
        feedback["isCorrect"] = false;
        feedback["feedback"]  = "Data access error";
        return feedback;
    }

    if (m_query.next()) {
        feedback["isCorrect"] = false;
        feedback["feedback"]  = "This username is already taken";
        return feedback;
    }

    m_query.prepare("INSERT INTO users (username, password) values (:username, :password);");
    m_query.bindValue(":username", message["username"].toString());
    m_query.bindValue(":password", message["password"].toString());

    // Problem with query
    if (!m_query.exec()) {
        feedback["isCorrect"] = false;
        feedback["feedback"]  = "Data access error";
        return feedback;
    }


    feedback["isCorrect"] = true;
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

    // The following validations for the DataBase
    m_query.prepare("SELECT password FROM users WHERE username = :username");
    m_query.bindValue(":username", message["username"].toString());

    // Problem with DataBase
    if (!m_query.exec()) {
        feedback["isCorrect"] = false;
        feedback["feedback"]  = "Data access error";
        return feedback;
    }

    if (!m_query.next() || message["password"].toString() != m_query.value(0).toString()) {
        feedback["isCorrect"] = false;
        feedback["feedback"]  = "Incorrect nickname or password";
        return feedback;
    }

    feedback["isCorrect"] = true;
    return feedback;
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
