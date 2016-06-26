#-------------------------------------------------
#
# Project created by QtCreator 2015-01-06T13:13:43
#
#-------------------------------------------------

QT       += core network script sql
QT       += serialport
QT       -= gui

CONFIG += c++11

TARGET = SmartHomeServer
CONFIG   += console
CONFIG   -= app_bundle


TEMPLATE = app




#Uncomment for RaspberryPI
#LIBS += -lwiringPi
#LIBS += -L../../liboic -loic



LIBS += -L../../liboic -loic
INCLUDEPATH += ../../liboic

LIBS += -L../../libcoap -lcoap
INCLUDEPATH += ../../libcoap

INCLUDEPATH += ../../std
INCLUDEPATH += ../../rfm69-driver


SOURCES += main.cpp \
    SmartHomeServer.cpp \
    BleScanner.cpp \
    IotEventSetting.cpp \
    IotEvent.cpp \
    DjangoInterface.cpp \
    Settings.cpp \
    Device.cpp \
    OcfDeviceController.cpp \
    IotDevice.cpp \
    IPv4OcfDeviceController.cpp \
    Rfm69OcfDeviceController.cpp \
    ../../rfm69-driver/rfm69.cpp \
    ../../rfm69-driver/rfm69hal.c

HEADERS += \
    SmartHomeServer.h \
    BleScanner.h \
    IotEventSetting.h \
    IotEvent.h \
    DjangoInterface.h \
    Settings.h \
    Device.h \
    OcfDeviceController.h \
    IotDevice.h \
    IPv4OcfDeviceController.h \
    Rfm69OcfDeviceController.h \
    ../../rfm69-driver/rfm69.h \
    ../../rfm69-driver/rfm69hal.h
