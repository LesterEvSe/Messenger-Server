#ifndef SERVERBACK_HPP
#define SERVERBACK_HPP

#include <QTcpServer>
#include <QTcpSocket>

class ServerBack : public QTcpServer
{
    Q_OBJECT
private:
    QTcpSocket *socket;

protected:
    void incomingConnection(qintptr socketDescriptor) override;

public:
    explicit ServerBack(QObject *parent = nullptr);
};

#endif // SERVERBACK_HPP
