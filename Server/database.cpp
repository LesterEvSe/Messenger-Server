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

bool Database::loginValidation(const QJsonObject& message)
{
    QSqlQuery query;
    query.prepare("SELECT password FROM users WHERE username = :username");
    query.bindValue(":username", message["username"].toString());

    if (!query.exec())
        throw QSqlError(query.lastError().text(), QString("'login validation'; "));

    if (!query.next() || message["password"].toString() != query.value(0).toString())
        return false;

    return true;
}

bool Database::registrationValidation(const QJsonObject& message)
{
    QSqlQuery query;
    query.prepare("SELECT password FROM users WHERE username = :username");
    query.bindValue(":username", message["username"].toString());

    // Problem with query
    if (!query.exec())
        throw QSqlError(query.lastError().text(),
                        QString("'registration validation' error 1; "));

    if (query.next())
        return false;

    query.prepare("INSERT INTO users (username, password) values (:username, :password);");
    query.bindValue(":username", message["username"].toString());
    query.bindValue(":password", message["password"].toString());

    // Problem with query
    if (!query.exec())
        throw QSqlError(query.lastError().text(),
                        QString("'registration validation' error 2; "));

    return true;
}



bool Database::findUser(const QString &user) const
{
    QSqlQuery query;
    query.prepare("SELECT username FROM users WHERE username = :user");
    query.bindValue(":user", user);

    if (!query.exec())
        throw QSqlError(query.lastError().text(),
                        QString("'find user' search error; "));

    // If it exists it returns true, otherwise false
    return query.next();
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

// Don't work if user1 == user2
QJsonObject Database::getMessages(const QString& user1, const QString& user2) const
{
    QSqlQuery query;
    query.prepare("SELECT sender, message FROM message "
                  "WHERE (sender = :user1 AND recipient = :user2) OR "
                  "      (sender = :user2 AND recipient = :user1) "
                  "ORDER BY message_id ASC");
    query.bindValue(":user1", user1);
    query.bindValue(":user2", user2);

    if (!query.exec())
        throw QSqlError(query.lastError().text(), QString("'get messages'; "));

    QJsonObject json;
    QJsonArray  chat_array;
    QJsonArray  our_messages_id;

    for (int coun = 0; query.next(); ++coun) {
        if (query.value(0).toString() == user1)
            our_messages_id.append(coun);
        chat_array.append(query.value(1).toString());
    }

    json["chat array"] = chat_array;
    json["our messages_id"] = our_messages_id;
    return json;
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

