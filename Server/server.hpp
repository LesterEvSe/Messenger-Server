#ifndef SERVER_HPP
#define SERVER_HPP

#include "serverback.hpp"

#include <QWidget>

class ServerBack;

QT_BEGIN_NAMESPACE
namespace Ui { class Server; }
QT_END_NAMESPACE

class Server : public QWidget
{
    Q_OBJECT
    friend class ServerBack;
private:
    Ui::Server *ui;
    void online_user    (const QString& username);
    void offline_user   (const QString& username);

public:
    Server(QWidget *parent = nullptr);
    ~Server();
};

#endif // SERVER_HPP
