### Requirement
- Cheep!
- 1 MCU, low power, Dual port

---
### Plan 1
**STM32H743VGT6**
[digikey]()
 - ARM® Cortex®-M7
 - 1.71V ~ 3.6V
 - 150 ma
 - 2 USB ports
 - $15
 Careful there are some models without dual OTG usb

---
### Plan 2
MCU with single USB support
**ESP32-S3** or **STM32F1**

**CH375 / CH376** for USB offloading

---
### Plan 2 - ESP

**ESP32-S3**
[digikey](https://www.digikey.com/en/products/detail/espressif-systems/ESP32-S3/15822445)
 - SPI + I2C + USB + Bluetooth + wifi
 - dual-core XTensa LX7
 - $3 - $5


**CH375B**
ACTS as a USB Interface, offload one of the USB commands
[JLCPCB](https://jlcpcb.com/partdetail/wch_jiangsu_Qin_heng-CH375B/C8962)
[english datasheet???](https://images.100y.com.tw/pdf_file/34-WCH-CH375.pdf)
 - 5v or 3.3v
 - Host or Slave mode, can switch dynamically
 - parallel interface or serial interface
 - $3-5


---
# I like plan 2

---
Next step, build a breakout to test CH37B