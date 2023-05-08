#include "serverback.hpp"

#include <QJsonDocument>
#include <QByteArray>
#include<QFileInfo>

#include <QDebug> // Need to delete later

ServerBack::ServerBack(QObject *parent) :
    QTcpServer(parent),
    database(QSqlDatabase::addDatabase("QSQLITE")),
    socket(nullptr)
{
    QString DBName = "messenger.sqlite";
    database.setDatabaseName(DBName);

    if (!database.open()) {
        qDebug() << "Failed to connect to database: " << database.lastError();
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
    socket = new QTcpSocket(this);

    // The descriptor is a positive number that identifies
    // the input/output stream
    socket->setSocketDescriptor(socketDescriptor);
    connect(socket, &QTcpSocket::readyRead, this, &ServerBack::slotReadyRead);
    connect(socket, &QTcpSocket::disconnected, this, [=](){
        qDebug() << "disconnected" << socketDescriptor;
        socket->deleteLater();
    });

    qDebug() << "Client connected" << socketDescriptor;
}

void ServerBack::slotReadyRead()
{
    socket = qobject_cast<QTcpSocket*>(sender());
    QByteArray data = socket->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    QJsonObject message = doc.object();

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

            Sockets[feedback["username"].toString()] = socket;
        }
        sendToClient(feedback, socket);
    }
    else if (message["type"] == "registration") {
        feedback = registration(message);
        if (feedback["isCorrect"].toBool()) {
            // Here need to display a message indicating
            // that there is a new user online

            Sockets[feedback["username"].toString()] = socket;
        }
        sendToClient(feedback, socket);
    }
    else
        // For users who want to break the system
        return;
}

void ServerBack::sendToClient(const QJsonObject& message, QTcpSocket *client)
{
    QJsonDocument doc(message);
    QByteArray data = doc.toJson(QJsonDocument::Compact);
    client->write(data);

    /*
    QJsonDocument doc(message);
    QByteArray data = doc.toJson(QJsonDocument::Compact);

    qint64 bytesWritten = 0;
    qint64 bytesTotal = data.size();

    while (bytesWritten < bytesTotal) {
        qint64 bytes = socket->write(data.mid(bytesWritten));
        if (bytes == -1) {
            qDebug() << "Error while sending data: " << socket->errorString();
            return;
        }

        bytesWritten += bytes;
        if (!socket->waitForBytesWritten()) {
            qDebug() << "Error while sending data: " << socket->errorString();
            return;
        }
    }
    */
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
