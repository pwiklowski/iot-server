#-------------------------------------------------
#
# Project created by QtCreator 2015-01-06T13:13:43
#
#-------------------------------------------------

QT       += core network bluetooth script sql
QT += serialport
QT       -= gui

TARGET = SmartHomeServer
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp \
    SmartHomeServer.cpp \
    SmartHomeClient.cpp \
    BleScanner.cpp \
    IotEventSetting.cpp \
    IotEvent.cpp \
    DjangoInterface.cpp \
    SerialBleScanner.cpp

HEADERS += \
    SmartHomeServer.h \
    SmartHomeClient.h \
    BleScanner.h \
    IotEventSetting.h \
    IotEvent.h \
    DjangoInterface.h \
    SerialBleScanner.h
