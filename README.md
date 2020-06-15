| PinNr |    PCB    |  Display  |                Note   |
| -----:| ---------:| ---------:| --------------------- |
|     1 | (VSS) GND |        31 |                       |
|     2 | (VCC) 3v3 |        32 |                       |
|     3 |        NC |        NC |                       |
|     4 |      /RST |         2 | -> NodeMCU D4         |
|     5 |       /SS |         1 | -> NodeMCU D3         |
|     6 |       D/C |         3 | -> NodeMCU D2         |
|     7 |       R/W |         4 | -> GND (perm write)   |
|     8 |        /E |         5 | -> GND (perm enabled) |
|     9 |        D0 |         6 | NC                    |
|    10 |        D1 |         7 | NC                    |
|    11 |        D2 |         8 | NC                    |
|    12 |        D3 |         9 | NC                    |
|    13 |        D4 |        10 | NC                    |
|    14 |        D5 |        11 | NC                    |
|    15 |  SCK - D6 |        12 | -> NodeMCU D5         |
|    16 | MOSI - D7 |        13 | -> NodeMCU D7         |
|    17 |    BL VCC |    VCC BL |                       |
|    18 |    BL VSS |    VSS BL |                       |


   J2 ->  pos 2 for SPI serial interface

