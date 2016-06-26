#ifndef RFM69HAL_H
#define RFM69HAL_H

#include <inttypes.h>
#include <stdbool.h>

#define RPI_

#ifdef RPI
#include <wiringPiSPI.h>
#endif


#define SPI_SPEED 2000000
#define SPI_DEVICE 0

#ifdef __cplusplus
extern "C" {
#endif

        int rfm69hal_init();
        void rfm69hal_delay_ms(uint32_t ms);
        uint32_t rfm69hal_get_timer_ms();
        void rfm69hal_enable(bool enable);
        uint8_t rfm69hal_transfer(uint8_t* bytes, uint16_t size);

#ifdef __cplusplus
}
#endif


#endif // RFM69HAL_H
