#include "serverback.hpp"

#include <QDebug>
#include <QByteArray>

ServerBack::ServerBack(QObject *parent) :
    QTcpServer(parent), socket(nullptr)
{
    listen(QHostAddress::Any, 1234);
    qDebug() << "Server started. Listening on port 1234...";
}

void ServerBack::incomingConnection(qintptr socketDescriptor)
{
    socket = new QTcpSocket(this);
    socket->setSocketDescriptor(socketDescriptor);

    qDebug() << "Client connected";
    connect(socket, &QTcpSocket::readyRead, this, [=](){
        QByteArray data = socket->readAll();
        qDebug() << "Message recevied from client: " << data;

        // Send the message bakc to the client,
        // otherwise it will not work
        socket->write(data);
    });

    connect(socket, &QTcpSocket::disconnected, this, [=](){
        qDebug() << "Client disconnected";
        socket->deleteLater();
    });
}
