#-------------------------------------------------
#
# Project created by QtCreator 2015-01-06T13:13:43
#
#-------------------------------------------------

QT       += core network script websockets
QT       += serialport
QT       -= gui

CONFIG += c++11

TARGET = IotAdapter
CONFIG   += console
CONFIG   -= app_bundle


TEMPLATE = app




#Uncomment for RaspberryPI
#LIBS += -lwiringPi


LIBS += -L../../liboic -loic
INCLUDEPATH += ../../liboic

LIBS += -L../../libcoap -lcoap
INCLUDEPATH += ../../libcoap

INCLUDEPATH += ../../std
INCLUDEPATH += ../../rfm69-driver


SOURCES += main.cpp \
    BleScanner.cpp \
    Device.cpp \
    OcfDeviceController.cpp \
    IotDevice.cpp \
    Rfm69OcfDeviceController.cpp \
    ../../rfm69-driver/rfm69.cpp \
    ../../rfm69-driver/rfm69hal.c \
    Rfm69DeviceController.cpp \
    devices/OicBinnarySwitch.cpp \
    devices/OicBaseDevice.cpp \
    BleButtonDeviceControler.cpp

HEADERS += \
    SmartHomeServer.h \
    BleScanner.h \
    Device.h \
    OcfDeviceController.h \
    IotDevice.h \
    Rfm69OcfDeviceController.h \
    ../../rfm69-driver/rfm69.h \
    ../../rfm69-driver/rfm69hal.h \
    Rfm69DeviceController.h \
    devices/OicBinnarySwitch.h \
    devices/OicBaseDevice.h \
    BleButtonDeviceControler.h
