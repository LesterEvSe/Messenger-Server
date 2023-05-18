QT          += core gui network sql
LIBS        += -lgmp
INCLUDEPATH += /usr/include

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    database.cpp \
    encryption.cpp \
    main.cpp \
    server.cpp \
    serverback.cpp

HEADERS += \
    database.hpp \
    encryption.hpp \
    server.hpp \
    serverback.hpp

FORMS += \
    server.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
