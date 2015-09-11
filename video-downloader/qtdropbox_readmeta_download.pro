QT += core network xml
QT -= gui

TARGET = qtdropbox_readmeta_download
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    downloader.cpp

HEADERS += \
    downloader.h \
    ../keys.h

LIBS += -lQtDropbox
