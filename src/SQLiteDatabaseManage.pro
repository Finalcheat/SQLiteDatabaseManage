#-------------------------------------------------
#
# Project created by QtCreator 2012-03-23T14:09:02
#
#-------------------------------------------------

QT       += core gui

TARGET = SQLiteDatabaseManage
TEMPLATE = app
CONFIG       += qaxcontainer

SOURCES += main.cpp\
        mainwindow.cpp \
    newdialog.cpp \
    tabwidget.cpp \
    finddialog.cpp \
    codeeditor.cpp \
    newtablewidget.cpp \
    datatypedelegate.cpp \
    highlighter.cpp \
    mydockwidget.cpp \
    exportwizard.cpp \
    querywidget.cpp \
    tableview.cpp \
    systemtableview.cpp \
    modifytablefieldwidget.cpp \
    createindexwidget.cpp \
    importwizard.cpp \
    mylistmodel.cpp \
    workthread.cpp

HEADERS  += mainwindow.h \
    newdialog.h \
    tabwidget.h \
    finddialog.h \
    codeeditor.h \
    newtablewidgett.h \
    datatypedelegate.h \
    highlighter.h \
    mydockwidget.h \
    exportwizard.h \
    querywidget.h \
    tableview.h \
    systemtableview.h \
    modifytablefieldwidget.h \
    createindexwidget.h \
    importwizard.h \
    mylistmodel.h \
    workthread.h

RESOURCES += \
    SQLiteDatabaseManage.qrc

QT += sql

QT += xml

TRANSLATIONS = SQLiteDatabaseManage.ts
