/*------------------------------------------------------------------------------
 * MDK Middleware - Component ::File System:Drive
 * Copyright (c) 2004-2019 Arm Limited (or its affiliates). All rights reserved.
 *------------------------------------------------------------------------------
 * Name:    FS_Config_RAM_0.h
 * Purpose: File System Configuration for RAM Drive
 * Rev.:    V6.2.0
 *----------------------------------------------------------------------------*/

//-------- <<< Use Configuration Wizard in Context Menu >>> --------------------

// <h>RAM Drive 0
// <i>Configuration for RAM assigned to drive letter "R0:"
#define RAM0_ENABLE             1

//   <o>Device Size <0x4C00-0xFFFFF000:0x400>
//   <i>Define the size of RAM device in bytes
//   <i>Default: 0x8000
#define RAM0_SIZE               0x030000

//   <e>Locate Drive Cache and Drive Buffer
//   <i>Locate RAM drive buffer at a specific address.
//   <i>If not enabled, the linker selects base address.
#define RAM0_RELOC              0

//     <o>Base address <0x0-0xFFFFF000:0x1000>
//     <i>Define the target device Base address.
//     <i>Default: 0x80000000
#define RAM0_BASE_ADDR          0x80000000

//   </e>

// </h>
