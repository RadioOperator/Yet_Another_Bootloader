
YAB - Using Internal Flash ROM for MSC Drive

Here is 3 YAB samples for STM32 ICs:
 
Hardware:
 - MCU with a USB full-speed / high-speed port
 - 1 Button/Wire, for YAB mode / APP mode switch
 - 1 LED indicator (optional)

Software: 
 - IDE: ARM MDK/Keil + STM32CubeMX
 
 - Memory Map: 16KB YAB + all rest Flash for APP
    
    STM32F103C8T6     YAB   0x08000000 - 0x08003FFF (16  KB)
                      APP   0x08004000 - 0x0801FFFF (112 KB)
                      
    STM32F407VET6     YAB   0x08000000 - 0x08003FFF (16  KB)
                      APP   0x08004000 - 0x0807FFFF (496 KB)
                      
    STM32F723IEK6     YAB   0x08000000 - 0x08003FFF (16  KB)
                      APP   0x08004000 - 0x0807FFFF (496 KB)
                      
Build/Edit/Flash YAB:
 - Nothing special, all refer to the source code.
 - Porting to other STM32 IC, try to set the Heap_Size to 0x2000 or more.
 - Modify the "YAB_Fxxx_config.h" file.
 - Modify the Flash Erase function in "YAB_fat32.c" file.
 - to get YAB <16KB BIN code size, use -Oz and Link-time Opt. of AC6.

Build and use your APP:
  a. Set Target IROM1 start from 0x08004000.  
  b. Add { SCB->VTOR = 0x08040000; } in the first line of "main()" function.
  c. Build and create the target BIN code.
  d. Connect the GPIO (Button/Wire) to GND.
  e. Restart/RESET the Device into "YAB mode".  
  f. Drag&Drop the BIN file to the "YAB Disk" Drive.
  g. Release the GPIO, Re-power the Device, APP will Running.
  
Note:  
 - For easy to use, no any complcated secuity/identity code applied.
    
    
Declarations:  
  
  The project is for experiments only. Use any parts and info on your own risks. 
  
  