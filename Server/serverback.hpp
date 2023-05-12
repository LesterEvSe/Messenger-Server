#ifndef SERVERBACK_HPP
#define SERVERBACK_HPP

#include "server.hpp"
#include "database.hpp"

#include <QTcpServer>
#include <QTcpSocket>

#include <QHash>
#include <QJsonObject>

#include <memory>

class Server;
class Database;

class ServerBack : public QTcpServer
{
    Q_OBJECT

private:
    Server *gui;
    qint64 m_block_size;

    // Working with the Database will be in a separate class
    std::unique_ptr<Database> m_database;

    // Here we store the username and its socket
    QHash<QString, QTcpSocket*> m_sockets;
    QTcpSocket *m_socket;

    void sendToClient        (const QJsonObject& message, QTcpSocket *client) const;
    void updatingOnlineUsers (QTcpSocket *client) const;

    void successEntry        (const QString& username);
    QJsonObject registration (const QJsonObject& message);
    QJsonObject login        (const QJsonObject& message);
    QJsonObject sendMessage  (const QJsonObject& message);

private slots:
    void incomingConnection(qintptr socketDescriptor);
    void slotReadyRead();

public:
    /// WHY WE DO NOT NEED A DESTRUCTOR
    /*
       Do not delete 'gui', because it is created on the stack
       A pointer is passed here

       m_sockets and m_socket delete by itself,
       because the QTcpServer class destructor will be called,
       which will clean up all the child

       m_database is unique_ptr
    */

    explicit ServerBack(Server *ui, QObject *parent = nullptr);
};

#endif // SERVERBACK_HPP
