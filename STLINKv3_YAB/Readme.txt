Yet_Another_Bootloader(YAB) - STLINKv3_YAB
 
This Yet Another Bootloader (YAB) is for STM32, STM32F4xx, STM32F7xx, STM32F723, STM32H7xx, STLINK-V3, STLINK-V3MINI, USB MSC, CMSIS-DAP, DIY, Educations.  

YAB, Free to use / Easy to use / Intelligent  
YAB, a PC MSC drive to Drag&Drop your App BIN code to flash the STM32F723 IC, no other Host software needed.  
YAB, easy to port to any other MCU with USB port.  


This YAB (project: STLINKv3_YAB) uses on a STLINK-V3 device, co-exist with STLINK-V3 Factory bootloader.

  - YAB start address from 0x08020000, as an application of the Factory bootloader.
  - APP_1/APP_2 max file size ~128KB
  

The STLINK-V3 with this YAB, there are 4 modes in it:

  - Factory bootloader mode, to flash YAB/apps or recover to STLINK-V3. (plug USB, then RESET).
  - YAB mode, to flash user APP_1/APP_2.    (PA6/nRESET to GND, then plug USB, APP_HERE on PC).
  - APP_1 mode, running APP_1.  (PA7 to GND, plug USB can toggle APP_1/APP_2 mode).
  - APP_2 mode, running APP_2.  ( ;;                                             ).

  
1. General Info of STLINKv3_YAB project:  

  Hardware Board: STLINK-V3MINI (Do not erase the Factory Bootloader!!!)  
  please visit https://github.com/RadioOperator/CMSIS-DAP_for_STLINK-V3MINI for hardware details.
    
  IDE: ARM MDK/Keil v5.xx, STM32CubeMX, ARM RTE  
  
  MCU:        STM32F723IEK6  
  RAM:        256KB  
  Flash ROM:  512KB  (we have a large Flash ROM.)
    
  RAM MSC Drive:                   0x20010000 - 0x2003FFFF  192KB  
      
  Flash Memory Map:  
  
    STLINK-V3 Factory bootloader:  0x08000000 - 0x0801FFFF  128KB
    YAB:                           0x08020000 - 0x0803FFFF  128KB 
    User APP_1                     0x08040000 - 0x0805FFFF  128KB
    User APP_2                     0x08060000 - 0x0807FFFF  128KB
      
  YAB GPIO and hardware RESET:  
    Suggest to add 3 Buttons(to GND):  
      IC Reset Pin,   CN4 on-board connector, Hardware RESET.  
      PA6/nRESET Pin, to entry YAB mode, short PA6 to GND before power-on/RESET.  
      PA7 Pin,        to toggle APP_1/APP_2, short PA7 to GND before power-on/RESET.  
      
  YAB LED status:  
    Green/Red slow blinking, YAB mode, waiting download APP_1/APP_2.  
    Green/Red fast blinking, VCP Tx/Rx transfering data.  
    Green color, download APP_1/APP_2 successful.  
    Red   color, a fault/error happened.  
      
  YAB USB VID/PID:  0x0483/0x5729.  
    
  YAB VCP Function: USART6 (PG9_Tx / PG14_Rx).  
    
    
2. Edit/Build/Flash a new YAB for STLINK-V3:  
    
  a. Set Target IROM1 start from 0x08020000 size 0x20000(64KB)  
  b. Set RAM1 start from 0x20010000 192KB size 0x30000(192KB), name "RAM_Drive"  
  c. Set IRAM1 start from 0x20000000 size 0x10000(64KB)  
  d. Other settings, refer to YAB project files.  
  e. The Scatter file:  
    ;--------------------------------------  
    LR_IROM1 0x08020000 0x00020000  {    ; load region size_region  
      ER_IROM1 0x08020000 0x00020000  {  ; load address = execution address  
       *.o (RESET, +First)  
       *(InRoot$$Sections)  
       .ANY (+RO)  
       .ANY (+XO)  
      }  
      RW_RAM1 0x20010000 UNINIT 0x00030000  {  ; MSC RAM drive  
       .ANY (RAM_Drive)  
      }  
      RW_IRAM1 0x20000000 0x00010000  {  
       .ANY (+RW +ZI)  
      }  
      END_OF_BLOCK 0x08040000 FIXED EMPTY 0x0 {} ; dummy area to fill 0xFF  
    }  
    ;--------------------------------------  
    
  f. Build YAB project, get the F3_1.bin and F3_2.bin files automatically.  
  g. Manually replace the same name files in STLinkUpgrade.jar.
  h. Plug STLINK-V3 USB, RESET to the Factory bootloader Mode.
  i. run Write-to-stlink-v3.bat to flash YAB into the device.
  j. note: your PC Windows/Java have to be setup correctly for all.
     note: before you modify YAB, you could try and test the pre-built code.  
    
    
3. Build and use your APP_1, filename: slv3app1.bin  
    
  a. Set Target IROM1 start from 0x08040000 size 0x20000(128KB max).  
  b. Set IRAM1 start from 0x20000000 size 0x40000(256KB).  
  c. Add `SCB-VTOR = 0x08040000;` in the first line of "main()" function.  
  d. Other settings, refer to the sample APP_1 Project files.  
  e. Build and create the target BIN code, name must be "slv3app1.bin".  
  f. Grounding PA6, Restart/RESET the Device into "MSC YAB mode".  
  g. Drag&Drop the "slv3app1.bin" file to the "APP_HERE" Drive.  
  h. Restart/RESET the Device if needed, APP_1 will Running.  
    
    
4. Build and use your APP_2, filename: slv3app2.bin  
    
  a. Set Target IROM1 start from 0x08060000 size 0x20000(128KB max).  
  b. Set IRAM1 start from 0x20000000 size 0x40000(256KB).  
  c. Add `SCB-VTOR = 0x08060000;` in the first line of "main()" function.  
  d. Other settings, refer to the sample APP_2 Project files.  
  e. Build and create the target BIN code, name must be "slv3app2.bin".  
  f. Grounding PA6, Restart/RESET the Device into "MSC YAB mode".  
  g. Drag&Drop the "slv3app2.bin" file to the "APP_HERE" Drive.  
  h. Restart/RESET the Device if needed, APP_2 will Running.  
      
      
5. Use the Startup Mode:  
  
  a. MSC YAB mode  
    Grounding the PA6/nRESET Pin before power-on/RESET.  
    The LED is blinking and a MSC drive on Host PC auto generated, name is "APP_HERE".  
    If APP_1 or APP_2 not loaded, the device always into YAB mode.  
      
  b. APP_1 mode  
    If the last loaded BIN file is slv3app1.bin, after RESET, in APP_1 mode auto.  

  c. APP_2 mode  
    If the last loaded BIN file is slv3app2.bin, after RESET, in APP_2 mode auto.  
      
  d. Toggle APP_1/APP_2 mode  
    Grounding the PA7 Pin before power-on/RESET.  
      
      
6. Note:  
    
  a. Do not download(Copy&Paste or Drag&Drop) two APP_1 and APP_2 files at same time.  
    
  b. For a complicated APP user code,   
    suggust develop/debug it under SWD mode with an ARM debugger(STLINK/JLINK/CMSIS-DAP...).  
    
    
7. Declarations:  
  
  The project is for experiments only. Use any parts and info on your own risks.  
  
