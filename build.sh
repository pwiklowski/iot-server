#iotserver
rm tmp/iot_server -rf
rm tmp/liboic -rf
rm tmp/libcoap -rf
rm tmp/std -rf
rm tmp/rfm69-driver -rf
rm tmp/qcron -rf
#
git clone https://github.com/pwiklowski/iot-server.git -b oic tmp/iot_server
git clone https://github.com/pwiklowski/liboic.git tmp/liboic
git clone https://github.com/pwiklowski/libcoap.git tmp/libcoap
git clone https://github.com/pwiklowski/lightstdlib.git tmp/std
git clone https://github.com/pwiklowski/rfm69-driver.git tmp/rfm69-driver
git clone https://github.com/pwiklowski/qcron.git tmp/qcron


docker-compose build
