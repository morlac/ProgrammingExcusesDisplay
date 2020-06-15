           PCB | Display |                  Note
-------------- | ------- | -----------------------
(VSS) GND -  1 |      31 |  
(VCC) 3v3 -  2 |      32 |
       NC -  3 |      NC |
     /RST -  4 |       2 | -> NodeMCU D4
      /SS -  5 |       1 | -> NodeMCU D3
      D/C -  6 |       3 | -> NodeMCU D2
      R/W -  7 |       4 | -> GND (perm write)
       /E -  8 |       5 | -> GND (perm enabled)
       D0 -  9 |       6 | NC
       D1 - 10 |       7 | NC
       D2 - 11 |       8 | NC
       D3 - 12 |       9 | NC
       D4 - 13 |      10 | NC
       D5 - 14 |      11 | NC
 SCK - D6 - 15 |      12 | -> NodeMCU D5
MOSI - D7 - 16 |      13 | -> NodeMCU D7
   BL VCC - 17 |  VCC BL |
   BL VSS - 18 |  VSS BL |


   J2 ->  pos 2 for SPI serial interface

