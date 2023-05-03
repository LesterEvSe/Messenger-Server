#include "server.hpp"
#include "serverback.hpp"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ServerBack sb;
    Server w;
    w.show();
    return a.exec();
}
