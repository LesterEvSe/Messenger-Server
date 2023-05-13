#ifndef SERVER_HPP
#define SERVER_HPP

#include "serverback.hpp"

#include <QWidget>
#include <QHash>
#include <QTextBrowser>
#include <QListWidgetItem>

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

    // The data are stored as follows: the pair username1, username2,
    // their chat window QTextBrowser, and the index in the stackWidget 'int'
    QHash<QPair<QString, QString>, std::pair<QTextBrowser*, int>> m_chats;

    void showError      (const QString& error);
    void online_user    (const QString& username);
    void offline_user   (const QString& username);

public:
    Server(QWidget *parent = nullptr);
    ~Server();
private slots:
    void on_chatsListWidget_itemClicked(QListWidgetItem *item);
};

#endif // SERVER_HPP
