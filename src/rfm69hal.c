#include "rfm69hal.h"
#include <unistd.h>
#include <sys/time.h>

uint64_t base;


uint64_t get_current_ms(){
    struct timeval te; 
    gettimeofday(&te, NULL); // get current time
    long long milliseconds = te.tv_sec*1000LL + te.tv_usec/1000; // caculate milliseconds
    // printf("milliseconds: %lld\n", milliseconds);
    return milliseconds;
}



int rfm69hal_init(){
    base = get_current_ms();
#ifdef RPI
    return wiringPiSPISetup(SPI_DEVICE, SPI_SPEED);
#endif

}


void rfm69hal_delay_ms(uint32_t ms){
	usleep(ms*1000);


}

uint32_t rfm69hal_get_timer_ms(){
    return get_current_ms()-base;
}

void rfm69hal_enable(bool enable){

}

uint8_t rfm69hal_transfer(uint8_t* bytes, uint16_t size){
#ifdef RPI
    wiringPiSPIDataRW(SPI_DEVICE, bytes,size);
#endif
}
