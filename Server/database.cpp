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
                        "user_id INTEGER PRIMARY KEY AUTOINCREMENT, "
                        "username TEXT, "
                        "password TEXT)")) {
        qDebug() << "Failed to create users table: " << query.lastError().text();
    }

    QSqlQuery query2("SELECT name FROM sqlite_master WHERE type='table' AND name='data';");
    if (!query2.next() &&
            !query2.exec("CREATE TABLE message ("
                        "message_id    INTEGER PRIMARY KEY AUTOINCREMENT, "
                        "sender_id     INTEGER, "
                        "recipient_id  INTEGER, "
                        "message       TEXT, "
                        "data_time     DATETIME)")) {
        qDebug() << "Failed to create data table: " << query2.lastError().text();
    }
}

bool Database::loginValidation(const QJsonObject& message, QJsonObject& feedback)
{
    QSqlQuery query;
    query.prepare("SELECT password FROM users WHERE username = :username");
    query.bindValue(":username", message["username"].toString());

    // Problem with DataBase
    if (!query.exec()) {
        feedback["isCorrect"] = false;
        feedback["feedback"]  = "Data access error";
        return false;
    }

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
    if (!query.exec()) {
        feedback["isCorrect"] = false;
        feedback["feedback"]  = "Data access error";
        return false;
    }

    if (query.next()) {
        feedback["isCorrect"] = false;
        feedback["feedback"]  = "This username is already taken";
        return false;
    }

    query.prepare("INSERT INTO users (username, password) values (:username, :password);");
    query.bindValue(":username", message["username"].toString());
    query.bindValue(":password", message["password"].toString());

    // Problem with query
    if (!query.exec()) {
        feedback["isCorrect"] = false;
        feedback["feedback"]  = "Data access error";
        return false;
    }

    return true;
}

bool Database::addMessage(const QJsonObject &message)
{
    // Two preparation queries, get sender_id and recipient_id
    QSqlQuery query;
    query.prepare("SELECT user_id FROM users WHERE username = :sender");
    query.bindValue(":sender", message["from"].toString());

    unsigned long long sender_id;
    if (query.exec() && query.next())
        sender_id = query.value(0).toULongLong();
    else {
        qDebug() << "Failed to execute query: " << query.lastError().text();
        return false;
    }
    query.clear();


    query.prepare("SELECT user_id FROM users WHERE username = :recipient");
    query.bindValue(":recipient", message["to"].toString());

    unsigned long long recipient_id;
    if (query.exec() && query.next())
        recipient_id = query.value(0).toULongLong();
    else {
        qDebug() << "Failed to execute query: " << query.lastError().text();
        return false;
    }
    query.clear();



    // main query
    query.prepare("INSERT INTO message (sender_id, recipient_id, message, data_time) "
                    "VALUES (:sender_id, :recipient_id, :message, :data_time)");
    query.bindValue(":sender_id", sender_id);
    query.bindValue(":recipient_id", recipient_id);
    query.bindValue(":message", message["message"].toString());
    query.bindValue(":data_time", QDateTime::currentDateTime());

    if (!query.exec()) {
        qDebug() << "Failed to insert message: " << query.lastError().text();
        return false;
    }
    return true;
}

// Need to implement
QJsonObject Database::getMessages(const QString& user1, const QString& user2)
{
    return QJsonObject();
}

QJsonArray Database::getChats(const QString &user) const
{
    return QJsonArray();
}
