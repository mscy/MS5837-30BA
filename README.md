# MS5837-30BA driver for ESP-IDF
The MS5837-30BA is a high-resolution pressure and temperature sensor 
from TE Connectivity (TE) with I2C bus interface. This sensor is optimized 
for water depth measurement systems with a resolution of 0.2 cm. The
sensor module includes a high linearity pressure sensor and an ultra-low 
power 24-bit ΔΣ ADC with internal factory calibrated coefficients. It 
provides a precise digital 24-bit pressure and temperature value and 
different operation modes that allow the user to optimize for conversion 
speed and current consumption. A high-resolution temperature output 
allows the implementation in depth measurement systems and 
thermometer function without any additional sensor. The MS5807-30BA 
can be interfaced to virtually any microcontroller. The communication 
protocol is simple, without the need of programming internal registers in the 
device. The gel protection and antimagnetic stainless-steel cap make the 
module water resistant.

The driver was verified on an esp32-c6 dev kit, sensor was connected to the I2C bus, please review main.c to undersand the logic and I added void i2c_scan() to help you find out how many devices connected to the bus.
<img width="731" height="396" alt="Screenshot 2026-01-19 at 00 43 18" src="https://github.com/user-attachments/assets/75380670-b6f1-40e8-b29b-c45d6ddf30b5" />

https://www.mouser.com/datasheet/2/418/6/ENG_DS_MS5837_30BA_C2-1130109.pdf


![Debug log](https://github.com/mscy/MS5837-30BA/blob/main/Screenshot%202026-01-19%20at%2000.22.16.png?raw=true)
![Alt text](https://github.com/mscy/MS5837-30BA/blob/main/Screenshot%202026-01-19%20at%2000.08.16.png?raw=true)
![Connections](https://github.com/mscy/MS5837-30BA/blob/main/Screenshot%202026-01-19%20at%2000.14.25.png?raw=true)


