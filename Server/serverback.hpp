#ifndef SERVERBACK_HPP
#define SERVERBACK_HPP

#include "server.hpp"
#include "database.hpp"
#include "encryption.hpp"

#include <QTcpServer>
#include <QTcpSocket>

#include <QHash>
#include <QJsonObject>
#include <QtSql>

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
    std::shared_ptr<Database> m_database;

    // Here we store the username and QPair
    /*
       We also need to store messages on the server.
       That is, the server will act as client and encode/decode each message.
       To avoid bugs like: requested key, saved message,
       another client made to CHANGE this GLOBAL QJsonObject m_message...
       Now each client (key QString in QHash) has its own message buffer
       in addition to the QTcpSocket, which acts as a bridge
    */
    /*
       Why shared_ptr instead of unique_ptr?
       Because when we take a value or iterate, ite temporarily copies the object
       so we can do something with it later.
       For unique_ptr this unacceptable, so the solution is shared_ptr
    */
    QHash<QString, QPair<QTcpSocket*, std::shared_ptr<QJsonObject>>> m_sockets;
    QTcpSocket *m_socket;

    Encryption *m_encryption;
    mutable QJsonObject m_message;

    void sendToClient        (const QJsonObject& message, QTcpSocket *client) const;
    void updatingOnlineUsers (QTcpSocket *client) const;

    void successEntry        (const QString& username);
    QJsonObject registration (const QJsonObject& message);
    QJsonObject login        (const QJsonObject& message);
    QJsonObject sendMessage  (const QJsonObject& message);

    // Full verification of unauthorized access
    bool authorizedAccess    (const QString& username) const;

    // Must be called after slotReadyRead
    void determineMessage    (const QJsonObject& message);

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
