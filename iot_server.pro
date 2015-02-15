#-------------------------------------------------
#
# Project created by QtCreator 2015-01-06T13:13:43
#
#-------------------------------------------------

QT       += core network

QT       -= gui

TARGET = SmartHomeServer
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp \
    SmartHomeServer.cpp \
    SmartHomeClient.cpp

HEADERS += \
    SmartHomeServer.h \
    SmartHomeClient.h
