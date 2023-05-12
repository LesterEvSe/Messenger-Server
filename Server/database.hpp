#ifndef DATABASE_HPP
#define DATABASE_HPP

#include "serverback.hpp"

#include <QtSql>

class Database
{
    friend class ServerBack;
private:
    QSqlDatabase m_log_reg;
    QSqlDatabase m_messanges; // need to initialize
    QSqlQuery m_query;

    bool loginValidation        (const QJsonObject& message, QJsonObject& feedback);
    bool registrationValidation (const QJsonObject& message, QJsonObject& feedback);

public:
    Database();
};

#endif // DATABASE_HPP
