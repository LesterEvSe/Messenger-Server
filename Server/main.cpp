#include "server.hpp"
#include "serverback.hpp"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Server w;
    ServerBack sb(&w);
    return a.exec();
}
