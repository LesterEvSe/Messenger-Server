#ifndef SERVERBACK_HPP
#define SERVERBACK_HPP

#include "server.hpp"

#include <QTcpServer>
#include <QTcpSocket>
#include <QtSql>

#include <QHash>
#include <QJsonObject>

class Server;
class ServerBack : public QTcpServer
{
    Q_OBJECT

private:
    Server *gui;
    qint64 m_block_size;

    QSqlDatabase m_database;
    QSqlQuery m_query;

    // Here we store the username and its socket
    QHash<QString, QTcpSocket*> m_sockets;
    QTcpSocket *m_socket;

    void sendToClient        (const QJsonObject& message, QTcpSocket *client);

    void successEntry        (const QString& username);
    QJsonObject registration (const QJsonObject& message);
    QJsonObject login        (const QJsonObject& message);
    QJsonObject sendMessage  (const QJsonObject& message);

private slots:
    void incomingConnection(qintptr socketDescriptor);
    void slotReadyRead();

public:
    explicit ServerBack(Server *ui, QObject *parent = nullptr);
};

#endif // SERVERBACK_HPP
