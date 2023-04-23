#include "widget.h"
#include "server.hpp"

#include <QCoreApplication>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    Server server;
    return a.exec();
}
