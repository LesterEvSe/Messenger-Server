#ifndef DATABASE_HPP
#define DATABASE_HPP

#include "serverback.hpp"

#include <QtSql>

class Database
{
    friend class ServerBack;
private:
    QSqlDatabase m_messenger;

    bool loginValidation        (const QJsonObject& message, QJsonObject& feedback);
    bool registrationValidation (const QJsonObject& message, QJsonObject& feedback);

    bool addMessage             (const QJsonObject& message);
    QJsonObject getMessages     (const QString& user1, const QString& user2);
    QJsonArray getChats         (const QString& user) const;



public:
    Database();
};

#endif // DATABASE_HPP
