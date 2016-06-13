#-------------------------------------------------
#
# Project created by QtCreator 2015-01-06T13:13:43
#
#-------------------------------------------------

QT       += core network bluetooth script sql
QT += serialport bluetooth
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
    BleScanner.cpp \
    IotEventSetting.cpp \
    IotEvent.cpp \
    DjangoInterface.cpp \
    Settings.cpp \
    Device.cpp \
    BleDeviceControler.cpp \
    OcfDeviceController.cpp \
    IPv4OcfDeviceController.cpp \
    Rfm69OcfDeviceController.cpp \
    rfm69.cpp \
    rfm69hal.c

HEADERS += \
    SmartHomeServer.h \
    BleScanner.h \
    IotEventSetting.h \
    IotEvent.h \
    DjangoInterface.h \
    Settings.h \
    Device.h \
    BleDeviceControler.h \
    OcfDeviceController.h \
    IPv4OcfDeviceController.h \
    Rfm69OcfDeviceController.h \
    rfm69.hpp \
    rfm69hal.h
