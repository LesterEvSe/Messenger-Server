#ifndef DATABASE_HPP
#define DATABASE_HPP

#include "serverback.hpp"

#include <QtSql>

class Database
{
private:
    QSqlDatabase m_messenger;

public:
    Database();

    bool loginValidation        (const QJsonObject& message, QJsonObject& feedback);
    bool registrationValidation (const QJsonObject& message, QJsonObject& feedback);

    bool addMessage             (const QJsonObject& message);
    QJsonObject getMessages     (const QString& user1, const QString& user2) const;
    QJsonArray getChats         (const QString& user) const;
};

#endif // DATABASE_HPP
