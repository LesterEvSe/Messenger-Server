#ifndef SERVERBACK_HPP
#define SERVERBACK_HPP

#include <QTcpServer>
#include <QTcpSocket>
#include <QtSql>

#include <QHash>
#include <QJsonObject>

class ServerBack : public QTcpServer
{
    Q_OBJECT

private:
    quint16 m_block_size;
    QSqlDatabase m_database;
    QSqlQuery m_query;

    // Here we store the username and its socket
    QHash<QString, QTcpSocket*> Sockets;
    void sendToClient        (const QJsonObject& message, QTcpSocket *client);

    QJsonObject registration (const QJsonObject& message);
    QJsonObject login        (const QJsonObject& message);
    QJsonObject sendMessage  (const QJsonObject& message);

private slots:
    void incomingConnection(qintptr socketDescriptor);
    void slotReadyRead();

public:
    QTcpSocket *m_socket;
    explicit ServerBack(QObject *parent = nullptr);
};

#endif // SERVERBACK_HPP
