# Panopticon Gateware & Firmware for the Panologic Thin Client

This project creates a RiscV32 based video game console from a Panologic thin client by 
loading a new bit image into the Pano FPGA, no other modifications are required.

Features:

- 25Mhz RiscV32i
- VGA video
  - 4-layer graphics similar to some handheld game systems
  - Two Tilemap layers
  - 32 on-screen sprites
  - One Bitmap layer
- Unfinished due to bootloader issues, likely related to cache flushing

## History

Based heavily on Pano_Z80
Scrapped old README that was filled with irrelevant info for this gateware


## Acknowledgement and Thanks

This project uses code and information from several other projects including:

 - [Tom Verbeure's Panologic efforts](https://github.com/tomverbeure/panologic)
 - [Skip Hansen's Pano Z80](https://github.com/skiphansen/pano_z80)
 - [Wenting Zhang's VerilogBoy](https://github.com/zephray/VerilogBoy)
 - [PicoRV32](https://github.com/cliffordwolf/picorv32)
 - [u-boot](https://github.com/u-boot/u-boot)
 - [unscii 8x8 Font](http://pelulamu.net/unscii/)
 - [Daniel Wallner's Z80 CPU core](https://opencores.org/projects/t80)
 - [ChaN's FatFs](http://elm-chan.org/fsw/ff/00index_e.html)
 - [Martin K. Schröder's VT100](https://github.com/mkschreder/avr-vt100)
 - [Martin Korth's GBAtek](https://problemkaputt.de/gbatek.htm)

## Pano Links

Links to other Panologic information can be found on the [Pano Hacker's Wiki](https://github.com/tomverbeure/panologic-g2/wiki#community)

## LEGAL 

My original work (the video subsystem), Skip Hansen's Pano Z80 project, and Wenting Zhang's Verilog Boy project
which they are based are released under the GNU Public License 2.0.

Wenting further acknowledged the following:

>The PicoRV32 is free and open hardware licensed under the [ISC license](http://en.wikipedia.org/wiki/ISC_license)
(a license that is similar in terms to the MIT license or the 2-clause BSD license).

>Some firmware code of pano-g1 target are released to public domain.

>All other software codes are licensed under GNU GPL 2.0.

>All other HDL codes are licensed under OHDL 1.0.

>All other text documents are licensed under CC BY-SA 4.0


>If you use this material in any way a reference to the author (me :-) ) 
will be appreciated.

