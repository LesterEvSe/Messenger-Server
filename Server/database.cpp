#include "database.hpp"

#include <QDebug>

Database::Database() :
    m_log_reg(QSqlDatabase::addDatabase("QSQLITE"))
{
    QString DBName = "../messenger.sqlite";
    m_log_reg.setDatabaseName(DBName);

    if (!m_log_reg.open()) {
        qDebug() << "Failed to connect to database: " << m_log_reg.lastError();
    }

    QSqlQuery query("SELECT name FROM sqlite_master WHERE type='table' AND name='users';");
    if (!query.next() && !query.exec("CREATE TABLE users (username TEXT, password TEXT)")) {
        qDebug() << "Failed to create table: " << query.lastError();
    }
}

bool Database::loginValidation(const QJsonObject& message, QJsonObject& feedback)
{
    m_query.prepare("SELECT password FROM users WHERE username = :username");
    m_query.bindValue(":username", message["username"].toString());

    // Problem with DataBase
    if (!m_query.exec()) {
        feedback["isCorrect"] = false;
        feedback["feedback"]  = "Data access error";
        return false;
    }

    if (!m_query.next() || message["password"].toString() != m_query.value(0).toString()) {
        feedback["isCorrect"] = false;
        feedback["feedback"]  = "Incorrect nickname or password";
        return false;
    }

    return true;
}

bool Database::registrationValidation(const QJsonObject& message, QJsonObject& feedback)
{
    m_query.prepare("SELECT password FROM users WHERE username = :username");
    m_query.bindValue(":username", message["username"].toString());

    // Problem with query
    if (!m_query.exec()) {
        feedback["isCorrect"] = false;
        feedback["feedback"]  = "Data access error";
        return false;
    }

    if (m_query.next()) {
        feedback["isCorrect"] = false;
        feedback["feedback"]  = "This username is already taken";
        return false;
    }

    m_query.prepare("INSERT INTO users (username, password) values (:username, :password);");
    m_query.bindValue(":username", message["username"].toString());
    m_query.bindValue(":password", message["password"].toString());

    // Problem with query
    if (!m_query.exec()) {
        feedback["isCorrect"] = false;
        feedback["feedback"]  = "Data access error";
        return false;
    }

    return true;
}
