#!/bin/sh

cd /iot_server/src/
LD_LIBRARY_PATH=../../liboic:../../libcoap:../../qcron/src ./SmartHomeServer
