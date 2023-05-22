#ifndef DATABASE_HPP
#define DATABASE_HPP

#include "serverback.hpp"

#include <QtSql>

// Singleton class
class Database
{
private:
    QSqlDatabase m_messenger;

    Database(const Database&) = delete;
    Database(Database&&)      = delete;
    Database& operator=(const Database&) = delete;
    Database& operator=(Database&&)      = delete;
    Database();

public:
    static Database* get_instance();

    bool loginValidation        (const QJsonObject& message);
    bool registrationValidation (const QJsonObject& message);

    bool findUser               (const QString& user) const;
    bool addMessage             (const QJsonObject& message);
    QJsonObject getMessages     (const QString& user1, const QString& user2) const;
    QJsonArray getChats         (const QString& user) const;
};

#endif // DATABASE_HPP
