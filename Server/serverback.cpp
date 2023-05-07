#include "serverback.hpp"

#include <QJsonDocument>
#include <QByteArray>

#include <QDebug> // Need to delete later

ServerBack::ServerBack(QObject *parent) :
    QTcpServer(parent),
    socket(nullptr)
{
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

    } else if (message["type"] == "login") {

    } else if (message["type"] == "registration") {

    } else
        // For users who want to break the system
        return;

    sendToClient(feedback);
}

void ServerBack::sendToClient(const QJsonObject& message)
{
    QJsonDocument doc(message);
    QByteArray data = doc.toJson(QJsonDocument::Compact);
    socket->write(data);

//    QJsonDocument doc(message);
//    QByteArray data = doc.toJson(QJsonDocument::Compact);

//    qint64 bytesWritten = 0;
//    qint64 bytesTotal = data.size();

//    while (bytesWritten < bytesTotal) {
//        qint64 bytes = socket->write(data.mid(bytesWritten));
//        if (bytes == -1) {
//            qDebug() << "Error while sending data: " << socket->errorString();
//            return;
//        }

//        bytesWritten += bytes;
//        if (!socket->waitForBytesWritten()) {
//            qDebug() << "Error while sending data: " << socket->errorString();
//            return;
//        }
//    }
}

QJsonObject ServerBack::registration(const QJsonObject &message)
{
    QJsonObject feedback;
    feedback["type"] = message["type"];
    if (message["username"].toString().isEmpty() || message["password"].toString().isEmpty()) {
        feedback["isCorrect"] = false;
        feedback["feedback"] = "You need to fill in both input fields";
    }
    else if (message["password"].toString().length() < 4) {
        feedback["isCorrect"] = false;
        feedback["feedback"] = "The password must contain at least 4 characters";
    }
    // The following validations for the DataBase


    // Need to thing about whether to send a blank feedback
    // when 'isCorrect' true or not to send it at all

    feedback["isCorrect"] = true;
    return feedback;
}

// Need to implement
QJsonObject ServerBack::login(const QJsonObject &message)
{
    return QJsonObject();
}

// Need to implement
QJsonObject ServerBack::message(const QJsonObject &message)
{
    return QJsonObject();
}
