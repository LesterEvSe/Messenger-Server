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
    QSqlDatabase database;
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
    QTcpSocket *socket;
    explicit ServerBack(QObject *parent = nullptr);
};

#endif // SERVERBACK_HPP
