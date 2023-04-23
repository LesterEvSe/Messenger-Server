#ifndef SERVER_HPP
#define SERVER_HPP

#include <QTcpServer>
#include <QTcpSocket>

class Server : public QTcpServer
{
    Q_OBJECT
public:
    explicit Server(QObject *parent = nullptr);

protected:
    void incomingConnection(qintptr socketDescriptor) override;

private:
    QTcpSocket *socket;
};

#endif // SERVER_HPP
