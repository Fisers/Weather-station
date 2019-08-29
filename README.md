# Weather Station
## Included sensors
- Wind Direction (NRG #200P)
- Anenometer (NRG #40)
- Humidity (HIH8131)
- Temperature (HIH8131)
- Atmospheric pressure (BMP280)
## Interface methods
- Ethernet TCP
  - Website interface
  - TCP packets to the server
- Nokia 5110 display
- SD card data logging in CSV format
- ~~USB to UART (For debugging and programming with bootloader)~~ See issues #1
## Power supply
- 100V - 240V AC
- 3.3V DC
## Extras
- Real Time Clock
- Easy calibration using buttons
- Space Invaders minigame
- No easter eggs
## Free PINs for future upgrades
- PB0 (Timer 0) | PA3 (ADC) | PA4 (ADC) | PA5 (ADC) | PA6 (ADC) | PA7 (ADC)
- Motherboard also has free connections to 3.3v and GND aswell as SPI.
