## Yet_Another_Bootloader  
This Yet Another Bootloader (YAB) is for STM32, STM32F4xx, STM32F7xx, STM32F723, STM32H7xx, STLINK-V3, STLINK-V3MINI, USB MSC, CMSIS-DAP, DIY, Educations.  
      
"YAB": Free to use / Easy to use / Intelligent  
  
  
### 1.General Info of this project:  

>  Hardware Board: STLINK-V3MINI (STM factory firmware erased)  
    
>  IDE: MDK / Keil v5.xx  
  
>  MCU:        STM32F723IEK6  
>  RAM:        256KB  
>  Flash ROM:  512KB  
    
>  RAM MSC Drive:  0x20010000 - 0x2003FFFF  192KB  
      
>  Flash Memory Map:  
>>    Bootloader    0x08000000 - 0x0800FFFF  64KB, Bootloader   
  
>>    User APP_1    0x08010000 - 0x0803A000  ~170KB, MSC upload  
>>    User APP_2    0x08040000 - 0x0806A000  ~170KB, MSC upload  
      
>>    Reserved      0X0803A000 - 0x0803FFFF  ~24KB, APP can use  
>>    Reserved      0x0806A000 - 0x0807FFFF  ~88KB, APP can use  
    
>  Bootloader GPIO:  
>>    Suggest to add 3 Buttons(to GND):  
>>>      IC Reset Pin, CN4 on-board connector, Hardware RESET.  
>>>      PA6/nRESET Pin, Entry the Bootloader mode when grounding it before power-on/RESET.  
>>>      PA7 Pin, Toggle to running APP_1/APP_2, grounding it before power-on/RESET.  
      
>  Bootloader used LED status:  
>>    Green/Red slow blinking, Bootloader mode, waiting upload APP_1/APP_2.  
>>    Green/Red fast blinking, VCP Tx/Rx transfering data.  
>>    Green color, upload APP_1/APP_2 successful.  
>>    Red   color, a fault/error happened.  
      
>  Bootloader USB VID/PID:  0x0483/0x5729.  
    
>  Bootloader VCP Function: USART6 (PG9_Tx / PG14_Rx).  
    
    
### 2.Build/Edit a new Bootloader:  
    
>  a.Set Target IROM1 start from 0x08000000 size 0x10000(64KB)  
>  b.Set RAM1 start from 0x20010000 192KB size 0x30000(192KB), name "RAM_Drive"  
>  c.Set IRAM1 start from 0x20000000 size 0x10000(64KB)  
>  d.Other settings, refer to the Bootloader project files.  
>  e.Build and Load the Bootloader into F723 IC via SWD port.  
    
>  f.The Scatter file to link the Bootloader:  
<pre>
    ;--------------------------------------  
    LR_IROM1 0x08000000 0x00010000  {    ; load region size_region  
      ER_IROM1 0x08000000 0x00010000  {  ; load address = execution address  
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
      END_OF_BLOCK 0x08010000 FIXED EMPTY 0x0 {} ; dummy area to fill 0xFF  
    }  
    ;--------------------------------------  
</pre>
    
### 3.Build and use your APP_1, filename: f723app1.bin  
    
>  a.Set Target IROM1 start from 0x08010000 size 0x30000(use 170KB max).  
>  b.Set IRAM1 start from 0x20000000 size 0x40000(256KB).  
>  c.Modify #define VECT_TAB_OFFSET 0x00 to (0x10000) in "system_stm32f7xx.c" file.  
>  d.Other settings, refer to the sample APP_1 Project files.  
>  e.Build and create the target BIN code, name must be "f723app1.bin", file size <=170KB.  
>  f.Grounding PA6, Restart/RESET the Device into "MSC Bootloader mode".  
>  g.Drag&Drop the "f723app1.bin" file to the "APP_HERE" Drive.  
>  h.Restart/RESET the Device, APP_1 will Running.  
    
    
### 4.Build and use your APP_2, filename: f723app2.bin  
    
>  a.Set Target IROM1 start from 0x08040000 size 0x30000(use 170KB max).  
>  b.Set IRAM1 start from 0x20000000 size 0x40000(256KB).  
>  c.Modify #define VECT_TAB_OFFSET 0x00 to (0x40000) in "system_stm32f7xx.c" file.  
>  d.Other settings, refer to the sample APP_2 Project files.  
>  e.Build and create the target BIN code, name must be "f723app2.bin", file size <=170KB.  
>  f.Grounding PA6, Restart/RESET the Device into "MSC Bootloader mode".  
>  g.Drag&Drop the "f723app2.bin" file to the "APP_HERE" Drive.  
>  h.Restart/RESET the Device, APP_2 will Running.  
      
      
### 5.Use the Startup Mode:  
  
>  a.MSC Bootloader mode  
>>    Grounding the PA6/nRESET Pin before power-on.  
>>    The LED is blinking and a MSC drive on Host PC auto generated, name is "APP_HERE".  
>>    If APP_1 or APP_2 not loaded, the device always into Bootloader mode.  
      
>  b.APP_1 mode  
>>    If the last loaded BIN file is f723app1.bin, after RESET, in APP_1 mode auto.  
>  c.APP_2 mode  
>>    If the last loaded BIN file is f723app2.bin, after RESET, in APP_2 mode auto.  
      
>  d.Toggle APP_1/APP_2 mode  
>>    Grounding the PA7 Pin before power-on/RESET.  
      
      
### 6.Note:  
    
>  a.Do not upload(Copy&Paste or Drag&Drop) two APP_1 and APP_2 files at the same time.  
    
>  b.If APP BIN code size is near to ~170KB,   
>>    you could delete this Readme.txt file from the APP_HERE drive.  
      
>  c.For a big APP user code,   
>>    suggust develop/debug it under SWD mode with an ARM debugger(STLINK/JLINK/CMSIS-DAP...),  
    
    
### 7.Declarations:  
  
>  The project is for experiments only. Use any parts and info on/at your own risks.  
  
