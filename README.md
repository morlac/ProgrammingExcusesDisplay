| PinNr |    PCB    |  Display  |          NodeMCU   |  Arduino |
| -----:| ---------:| ---------:| ------------------ | -------- |
|     1 | (VSS) GND |        31 |                    |          |
|     2 | (VCC) 3v3 |        32 |                    |          |
|     3 |        NC |        NC |                    |          |
|     4 |      /RST |         2 | D4                 | D8       |
|     5 |       /SS |         1 | D3                 | D10      |
|     6 |       D/C |         3 | D2                 | D9       |
|     7 |       R/W |         4 | GND (perm write)   | <-       |
|     8 |        /E |         5 | GND (perm enabled) | <-       |
|     9 |        D0 |         6 | NC                 | NC       |
|    10 |        D1 |         7 | NC                 | NC       |
|    11 |        D2 |         8 | NC                 | NC       |
|    12 |        D3 |         9 | NC                 | NC       |
|    13 |        D4 |        10 | NC                 | NC       |
|    14 |        D5 |        11 | NC                 | NC       |
|    15 |  SCK - D6 |        12 | D5                 | D13      |
|    16 | MOSI - D7 |        13 | D7                 | D12      |
|    17 |    BL VCC |    VCC BL |                    |          |
|    18 |    BL VSS |    VSS BL |                    |          |


   J2 ->  pos 2 for SPI serial interface

for this to work the modified U8g2 Library has to be used:
	https://github.com/morlac/U8g2_Arduino

NodeMCU:
```c++
#define DISPLAY_RST   D4     // RST pin is connected to NodeMCU pin D4 (GPIO2)
#define DISPLAY_CS    D3     // CS  pin is connected to NodeMCU pin D4 (GPIO0)
#define DISPLAY_DC    D2     // DC  pin is connected to NodeMCU pin D4 (GPIO4)
// initialize U8g2 library with hardware SPI module
// SCK (CLK) ---> NodeMCU pin D5 (GPIO14)
// MOSI(DIN) ---> NodeMCU pin D7 (GPIO13)

#include <U8g2lib.h>
U8G2_ST7565_PE12864_004_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ DISPLAY_CS, /* dc=*/ DISPLAY_DC, /* reset=*/ DISPLAY_RST);
```

Arduino:

checkout the above mentioned library in the Arduino libraries folder

```
#define DISPLAY_RST   8
#define DISPLAY_CS    10
#define DISPLAY_DC    9
// initialize U8g2 library with hardware SPI module
// SCK (CLK) ----> Arduino pin D13
// MOSI(DIN) ----> Arduino pin D12

#include <U8g2lib.h>
U8G2_ST7565_PE12864_004_1_4W_HW_SPI(U8G2_R0, /* cs=*/ DISPLAY_CS, /* dc=*/ DISPLAY_DC, /* reset=*/ DISPLAY_RST);
```

