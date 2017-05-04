#-------------------------------------------------
#
# Project created by QtCreator 2015-01-06T13:13:43
#
#-------------------------------------------------

QT       += core network script websockets
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

LIBS += -L../../qcron/src -lqcron
INCLUDEPATH += ../../qcron/src

INCLUDEPATH += ../../std
INCLUDEPATH += ../../rfm69-driver


SOURCES += main.cpp \
    SmartHomeServer.cpp \
    BleScanner.cpp \
    Settings.cpp \
    Device.cpp \
    OcfDeviceController.cpp \
    IPv4OcfDeviceController.cpp \
    Rfm69OcfDeviceController.cpp \
    ../../rfm69-driver/rfm69.cpp \
    ../../rfm69-driver/rfm69hal.c \
    Rfm69DeviceController.cpp \
    devices/OicBinnarySwitch.cpp \
    devices/OicBaseDevice.cpp \
    BleButtonDeviceControler.cpp \
    WebSocketServer.cpp \
    WebSocketConnection.cpp \
    ScriptRunner.cpp \
    devices/OcfBinnarySwitchWithBattery.cpp \
    nrf24lDeviceController.cpp

HEADERS += \
    SmartHomeServer.h \
    BleScanner.h \
    Settings.h \
    Device.h \
    OcfDeviceController.h \
    IPv4OcfDeviceController.h \
    Rfm69OcfDeviceController.h \
    ../../rfm69-driver/rfm69.h \
    ../../rfm69-driver/rfm69hal.h \
    Rfm69DeviceController.h \
    devices/OicBinnarySwitch.h \
    devices/OicBaseDevice.h \
    BleButtonDeviceControler.h \
    WebSocketServer.h \
    WebSocketConnection.h \
    ScriptRunner.h \
    devices/OcfBinnarySwitchWithBattery.h \
    nrf24lDeviceController.h
