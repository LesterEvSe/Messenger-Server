#ifndef SERVER_HPP
#define SERVER_HPP

#include "serverback.hpp"
#include "database.hpp"

#include <QWidget>
#include <QHash>
#include <QTextBrowser>
#include <QListWidgetItem>

class ServerBack;
class Database;

QT_BEGIN_NAMESPACE
namespace Ui { class Server; }
QT_END_NAMESPACE

// *******************************************************************

// This is essentially a normal GUI for easy browsing of the DataBase.

// We can choose which user we are on the left or via the search bar,
// this is our windowTitle.
// After that, all chats on the right will be shown
// that are available to this user.
// currChat Label identifies the user with whom we are communicating

// *******************************************************************
class Server : public QWidget
{
    Q_OBJECT
    friend class ServerBack;
private:
    Ui::Server *ui;
    std::shared_ptr<Database> m_database;

    // The data are stored as follows: the pair username1, username2,
    // their chat window QTextBrowser, and the index in the stackWidget 'int'
    QHash<QPair<QString, QString>, std::pair<QTextBrowser*, int>> m_chats;

    void showError      (const QString& error);
    void online_user    (const QString& username);
    void offline_user   (const QString& username);

    // In the following two functions,
    // we use windowTitle() as m_username
    // and currChatLabel as the user we are communicating with
    void updateChats    ();
    void updateCorrespondence     ();

private slots:
    void on_chatsListWidget_itemClicked(QListWidgetItem *item);

    // Next two functions have the same purpose.
    // Show chats of the selected user
    void on_onlineUsersListWidget_itemClicked(QListWidgetItem *item);
    void on_findUserLineEdit_returnPressed();

public:
    Server(QWidget *parent = nullptr);
    ~Server();
};

#endif // SERVER_HPP
