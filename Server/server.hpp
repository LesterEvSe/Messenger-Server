#ifndef SERVER_HPP
#define SERVER_HPP

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui { class Server; }
QT_END_NAMESPACE

class Server : public QWidget
{
    Q_OBJECT
private:
    Ui::Server *ui;

public:
    Server(QWidget *parent = nullptr);
    ~Server();
};
#endif // SERVER_HPP
