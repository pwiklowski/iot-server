#include "rfm69hal.h"


int rfm69hal_init(){
#ifdef RPI
    return wiringPiSPISetup(SPI_DEVICE, SPI_SPEED);
#endif
}


void rfm69hal_delay_ms(uint32_t ms){


}

uint32_t rfm69hal_get_timer_ms(){

}

void rfm69hal_enable(bool enable){

}

uint8_t rfm69hal_transfer(uint8_t* bytes, uint16_t size){
#ifdef RPI
    wiringPiSPIDataRW(SPI_DEVICE, bytes,size);
    usleep(5);
    return byte;
#endif
}
