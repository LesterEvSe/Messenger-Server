#include "server.hpp"

#include <QCoreApplication>
#include <QDebug>

Server::Server(QObject *parent) :
    QTcpServer(parent), socket(nullptr)
{
    listen(QHostAddress::Any, 12345);
    qDebug() << "Server started. Listening on port 12345...";
}

void Server::incomingConnection(qintptr socketDescriptor)
{
    socket = new QTcpSocket(this);
    socket->setSocketDescriptor(socketDescriptor);

    qDebug() << "Client connected";
    connect(socket, &QTcpSocket::readyRead, this, [this]() {
        QByteArray data = socket->readAll();
        qDebug() << "Message received from client: " << data.constData();

        // Send the message back to the client,
        // otherwise it will not work
        socket->write(data);
    });

    connect(socket, &QTcpSocket::disconnected, this, [this]() {
        qDebug() << "Client disconnected";
        socket->deleteLater();
    });
}
