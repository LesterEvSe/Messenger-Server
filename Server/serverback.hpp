#ifndef SERVERBACK_HPP
#define SERVERBACK_HPP

#include <QTcpServer>
#include <QTcpSocket>
#include <QByteArray>
#include <QVector>
#include <QTime>

class ServerBack : public QTcpServer
{
    Q_OBJECT

private:
    // Here we store sockets with clients
    QVector <QTcpSocket*> Sockets;

    // This data is transferred from the server
    // to the client and vice versa
    QByteArray Data;
    quint16 nextBlockSize;

    void sendToClient(QString str);

private slots:
    void incomingConnection(qintptr socketDescriptor);
    void slotReadyRead();

public:
    QTcpSocket *socket;
    explicit ServerBack(QObject *parent = nullptr);
};

#endif // SERVERBACK_HPP
