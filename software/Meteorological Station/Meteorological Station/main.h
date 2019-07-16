#include "includes/SPI/SPI.h"
#include "includes/I2C/i2cmaster.h"
#include "includes/BMP280_SPI/BMP280_SPI.h"
#include "includes/WindSpeed/windSpeed.h"
#include "includes/HIH8120/hih8120.h"
#include "includes/Ethernet/test_web_client.h"
#include "includes/RTC/rtc.h"
#include "includes/FatFs/ff.h"
#include "Options.h"
#include "includes/UART/uart.h"

FILE uart_output = FDEV_SETUP_STREAM(uart_putchar, NULL, _FDEV_SETUP_WRITE);
FILE uart_input = FDEV_SETUP_STREAM(NULL, uart_getchar, _FDEV_SETUP_READ);