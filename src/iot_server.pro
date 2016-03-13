#-------------------------------------------------
#
# Project created by QtCreator 2015-01-06T13:13:43
#
#-------------------------------------------------

QT       += core network script sql
QT += serialport
QT       -= gui
CONFIG += c++11

TARGET = SmartHomeServer
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


LIBS += -L../../liboic-build -loic
INCLUDEPATH += ../../liboic

LIBS += -L../../libcoap -lcoap
INCLUDEPATH += ../../libcoap

INCLUDEPATH += ../../std

SOURCES += main.cpp \
    SmartHomeServer.cpp \
    IotEventSetting.cpp \
    IotEvent.cpp \
    DjangoInterface.cpp \
    Settings.cpp \
    Device.cpp

HEADERS += \
    SmartHomeServer.h \
    IotEventSetting.h \
    IotEvent.h \
    DjangoInterface.h \
    Settings.h \
    Device.h
