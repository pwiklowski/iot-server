#ifndef RFM69HAL_H
#define RFM69HAL_H

#include <inttypes.h>
#include <stdbool.h>
#include <wiringPiSPI.h>


#define SPI_SPEED 2000000
#define SPI_DEVICE 0

void rfm69hal_init(){
    // Initialize SPI device 0
    if(wiringPiSPISetup(SPI_DEVICE, SPI_SPEED) < 0) {
      fprintf(stderr, "Unable to open SPI device\n\r");
      exit(1);
    }
}


void rfm69hal_delay_ms(uint32_t ms){


}

uint32_t rfm69hal_get_timer_ms(){

}

void rfm69hal_enable(bool enable){

}

uint8_t rfm69hal_transfer(uint8_t byte){
    wiringPiSPIDataRW(SPI_DEVICE, byte, 1);
    return byte;
}



#endif // RFM69HAL_H
