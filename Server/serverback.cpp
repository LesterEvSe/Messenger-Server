#include "serverback.hpp"

#include <QDataStream>
#include <QDebug>

ServerBack::ServerBack(QObject *parent) :
    QTcpServer(parent),
    socket(nullptr),
    nextBlockSize(0)
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

    Sockets.push_back(socket);
    qDebug() << "Client connected" << socketDescriptor;
}

void ServerBack::slotReadyRead()
{
    // The socket the request came from
    socket = (QTcpSocket*)sender();
    QDataStream in(socket);

    // Specify the version to avoid errors
    in.setVersion(QDataStream::Qt_5_15);
    if (in.status() != QDataStream::Ok) {
        qDebug() << "DataStream error";
        return;
    }

    // We do not know if the data will come in full or in parts,
    // so we need an endless loop

    // while(true) gives a warning,
    // so change to the for loop
    for (;;) {
        // Block size unknown (0)
        if (nextBlockSize == 0) {
            if (socket->bytesAvailable() < 2)
                break;
            in >> nextBlockSize;
        }
        // if the data is incomplete
        if (socket->bytesAvailable() < nextBlockSize)
            break;

        QString str;
        QTime time;
        in >> time >> str;

        // To be able to receive new messages
        nextBlockSize = 0;
        sendToClient(str);
        break;
    }
}

void ServerBack::sendToClient(QString str)
{
    // Clean it up, because there may be trash in here
    Data.clear();
    QDataStream out(&Data, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_15);

    // until we can determine the size of the block, we write 0
    // HERE IS THE MESSAGE OUTPUT
    out << quint16(0) << QTime::currentTime() << str;
    out.device()->seek(0);

    // The first two bits are separators
    out << quint16(Data.size() - sizeof(quint16));
    socket->write(Data);
}
