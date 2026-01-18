# MS5837-30BA
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

this driver was verified on a esp32-c6 dev kit, sensor was connected to the I2C bus, please review main.c to undersand the logic and I added void i2c_scan() to help you find out how many devices connected to the bus.
