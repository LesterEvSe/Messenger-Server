#include "server.hpp"
#include "ui_server.h"

#include <QScreen>
#include <QMessageBox>

Server::Server(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Server)
{
    m_database = std::make_shared<Database>();

    ui->setupUi(this);
    setWindowTitle("Server");
    ui->findUserLineEdit->setPlaceholderText("Find user...");

    QRect screenGeometry = QApplication::primaryScreen()->geometry();
    int w = screenGeometry.width();
    int h = screenGeometry.height();
    move((w - width())/2, (h - height())/2);
}

Server::~Server() {
    delete ui;
}

void Server::showError(const QString &error) {
    QMessageBox::critical(this, "Error", error);
    exit(1);
}


void Server::online_user(const QString &username) {
    ui->onlineUsersListWidget->addItem(username);
}

void Server::offline_user(const QString &username)
{
    for (int i = 0; i < ui->onlineUsersListWidget->count(); ++i) {
        QListWidgetItem *item = ui->onlineUsersListWidget->item(i);
        if (item->text() == username) {
            ui->onlineUsersListWidget->takeItem(i);
            break;
        }
    }
}



void Server::updateChats()
{
    ui->chatsListWidget->clear();
    QJsonArray user_chats = m_database->getChats(windowTitle());
    for (int i = 0; i < user_chats.size(); ++i)
        ui->chatsListWidget->addItem(user_chats[i].toString());
}

void Server::updateCorrespondence()
{
    QPair<QString, QString> pair(windowTitle(), ui->currChatLabel->text());
    m_chats[pair].first->clear();
    QJsonObject chat = m_database->getMessages(pair.first, pair.second);

    QJsonArray chat_array = chat["chat array"].toArray();
    QJsonArray mess_num = chat["our messages_id"].toArray();
    for (int coun = 0, our_mess_coun = 0; coun < chat_array.size(); ++coun)
    {
        QString nick;
        if (coun == mess_num[our_mess_coun].toInt())
            nick = ui->currChatLabel->text();
        else
            nick = windowTitle();
        m_chats[pair].first->append(nick + ": " + chat_array[coun].toString());
        m_chats[pair].first->append("");
    }
}

void Server::on_chatsListWidget_itemClicked(QListWidgetItem *item)
{
    ui->currChatLabel->setText(item->text());
    QPair<QString, QString> pair(windowTitle(), item->text());
    if (m_chats.find(pair) != m_chats.end()) {
        updateCorrespondence();
        ui->stackedWidget->setCurrentIndex(m_chats[pair].second);
        return;
    }

    QTextBrowser *browser = new QTextBrowser(this);
    m_chats[pair] = { browser, ui->stackedWidget->count() };

    QWidget *widget = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(widget);
    layout->addWidget(browser);

    ui->stackedWidget->addWidget(widget);
    updateCorrespondence();
    ui->stackedWidget->setCurrentIndex(m_chats[pair].second);
}



// Next two functions have the same purpose.
// Show chats of the selected user
void Server::on_onlineUsersListWidget_itemClicked(QListWidgetItem *item)
{
    setWindowTitle(item->text());
    updateChats();
}


void Server::on_findUserLineEdit_returnPressed()
{
    QString user = ui->findUserLineEdit->text();
    if (!m_database->findUser(user)) {
        ui->findUserLineEdit->clear();
        QMessageBox::warning(this, "Warning", "There is no such user");
        return;
    }
    setWindowTitle(user);
    ui->findUserLineEdit->clear();
    updateChats();
}

