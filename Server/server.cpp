#include "server.hpp"
#include "ui_server.h"

#include <QScreen>

Server::Server(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Server)
{
    ui->setupUi(this);
    setWindowTitle("Server");
    QRect screenGeometry = QApplication::primaryScreen()->geometry();
    int w = screenGeometry.width();
    int h = screenGeometry.height();
    move((w - width())/2, (h - height())/2);
}

Server::~Server() {
    delete ui;
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
