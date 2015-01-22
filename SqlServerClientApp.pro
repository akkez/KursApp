#-------------------------------------------------
#
# Project created by QtCreator 2014-12-23T16:08:39
#
#-------------------------------------------------

QT       += core gui sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = SqlServerClientApp
TEMPLATE = app


SOURCES += main.cpp \
        mainwindow.cpp \
        infowindow.cpp \
    editwindow.cpp

HEADERS  += mainwindow.h \
    infowindow.h \
    config.h \
    editwindow.h

FORMS    += mainwindow.ui \
    infowindow.ui \
    editwindow.ui

RESOURCES += \
    resources.qrc
