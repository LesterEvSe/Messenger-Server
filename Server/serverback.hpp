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

    // about m_sockets and m_messages
    // The socket from m_sockets relates to the key socket here
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
       Because when we take a value or iterate, it temporarily copies the object
       so we can do something with it later.
       For unique_ptr this unacceptable, so the solution is shared_ptr
    */

    // Here we store the username and its socket
    QHash<QString, QTcpSocket*> m_sockets;
    mutable QHash<QTcpSocket*, std::shared_ptr<QJsonObject>> m_messages;
    Encryption *m_encryption;

//    QTcpSocket *m_socket; // Need to delete later
    mutable QJsonObject m_message; // Need to delete later

    void sendToClient        (const QJsonObject& message, QTcpSocket *client) const;
    void updatingOnlineUsers (QTcpSocket *client) const;

    void successEntry               (const QString& username, QTcpSocket *socket);
    QJsonObject regLogValidation    (const QJsonObject& message);
    QJsonObject sendMessage         (const QJsonObject& message);

    // Full verification of unauthorized access
    bool authorizedAccess    (const QString& username, const QTcpSocket *socket) const;

    // Must be called after slotReadyRead
    void determineMessage    (const QJsonObject& message, QTcpSocket *socket);

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
