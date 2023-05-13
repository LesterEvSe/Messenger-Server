#include "database.hpp"

#include <QDebug>
#include <QTime>

Database::Database() :
    m_messenger(QSqlDatabase::addDatabase("QSQLITE"))
{
    QString DBName = "../messenger.sqlite";
    m_messenger.setDatabaseName(DBName);

    if (!m_messenger.open()) {
        qDebug() << "Failed to connect to log_pass database: " << m_messenger.lastError().text();
    }

    QSqlQuery query("SELECT name FROM sqlite_master WHERE type='table' AND name='users';");
    if (!query.next() &&
            !query.exec("CREATE TABLE users ("
                        "username TEXT, "
                        "password TEXT)")) {
        qDebug() << "Failed to create users table: " << query.lastError().text();
    }

    QSqlQuery query2("SELECT name FROM sqlite_master WHERE type='table' AND name='message';");
    if (!query2.next() &&
            !query2.exec("CREATE TABLE message ("
                        "message_id  INTEGER PRIMARY KEY AUTOINCREMENT, "
                        "sender      TEXT, "
                        "recipient   TEXT, "
                        "message     TEXT, "
                        "data_time   DATETIME)")) {
        qDebug() << "Failed to create data table: " << query2.lastError().text();
    }
}

bool Database::loginValidation(const QJsonObject& message, QJsonObject& feedback)
{
    QSqlQuery query;
    query.prepare("SELECT password FROM users WHERE username = :username");
    query.bindValue(":username", message["username"].toString());

    if (!query.exec())
        throw QSqlError(query.lastError().text(), QString("'login validation'; "));

    if (!query.next() || message["password"].toString() != query.value(0).toString()) {
        feedback["isCorrect"] = false;
        feedback["feedback"]  = "Incorrect nickname or password";

        return false;
    }

    return true;
}

bool Database::registrationValidation(const QJsonObject& message, QJsonObject& feedback)
{
    QSqlQuery query;
    query.prepare("SELECT password FROM users WHERE username = :username");
    query.bindValue(":username", message["username"].toString());

    // Problem with query
    if (!query.exec())
        throw QSqlError(query.lastError().text(),
                        QString("'registration validation' error 1; "));

    if (query.next()) {
        feedback["isCorrect"] = false;
        feedback["feedback"]  = "This username is already taken";
        return false;
    }

    query.prepare("INSERT INTO users (username, password) values (:username, :password);");
    query.bindValue(":username", message["username"].toString());
    query.bindValue(":password", message["password"].toString());

    // Problem with query
    if (!query.exec())
        throw QSqlError(query.lastError().text(),
                        QString("'registration validation' error 2; "));

    return true;
}

bool Database::addMessage(const QJsonObject &message)
{
    QSqlQuery query;
    query.prepare("INSERT INTO message (sender, recipient, message, data_time) "
                    "VALUES (:sender, :recipient, :message, :data_time)");
    query.bindValue(":sender", message["from"].toString());
    query.bindValue(":recipient", message["to"].toString());
    query.bindValue(":message", message["message"].toString());
    query.bindValue(":data_time", QDateTime::currentDateTime());

    if (!query.exec())
        throw QSqlError(query.lastError().text(),
                        QString("'add message' failed to insert; "));
    return true;
}

// Need to implement
QJsonObject Database::getMessages(const QString& user1, const QString& user2)
{
    return QJsonObject();
}

QJsonArray Database::getChats(const QString &user) const
{
    // To get all chats, 1. select all users to whom we wrote
    // 2. Select all users who wrote to us
    // 3. Union and delete repetitions
    QSqlQuery query;
    query.prepare("SELECT DISTINCT recipient FROM message WHERE sender = :user UNION "
                  "SELECT DISTINCT sender from message WHERE recipient = :user");
    query.bindValue(":user", user);

    if (!query.exec())
        throw QSqlError(query.lastError().text(), QString("'get chats'; "));

    QJsonArray arr;
    while (query.next())
        arr.append(query.value(0).toString());
    return arr;
}

