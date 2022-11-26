// Copyright 2020 Espressif Systems (Shanghai) Co. Ltd.
// Copyright 2020 Mike Dunston (https://github.com/atanisoft)
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once
#include "sdkconfig.h"

#ifdef __cplusplus
extern "C" {
#endif

//--------------------------------------------------------------------
// COMMON CONFIGURATION
//--------------------------------------------------------------------

#define CFG_TUSB_RHPORT0_MODE OPT_MODE_DEVICE
#define CFG_TUSB_OS OPT_OS_FREERTOS

// CFG_TUSB_DEBUG is defined by compiler in DEBUG build
#define CFG_TUSB_DEBUG CONFIG_ESPUSB_DEBUG

/* USB DMA on some MCUs can only access a specific SRAM region with restriction on alignment.
 * TinyUSB use follows macros to declare transferring memory so that they can be put
 * into those specific section.
 * e.g
 * - CFG_TUSB_MEM SECTION : __attribute__ (( section(".usb_ram") ))
 * - CFG_TUSB_MEM_ALIGN   : __attribute__ ((aligned(4)))
 */
#ifndef CFG_TUSB_MEM_SECTION
#define CFG_TUSB_MEM_SECTION
#endif

#ifndef CFG_TUSB_MEM_ALIGN
#define CFG_TUSB_MEM_ALIGN __attribute__((aligned(4)))
#endif

//--------------------------------------------------------------------
// ENDPOINT FIFO SIZE CONFIGURATION
//--------------------------------------------------------------------
#ifndef CFG_TUD_ENDPOINT0_SIZE
#define CFG_TUD_ENDPOINT0_SIZE 64
#endif

//--------------------------------------------------------------------
// KCONFIG DEFAULT CONFIGURATION
//--------------------------------------------------------------------
#ifndef CONFIG_ESPUSB_CDC
#define CONFIG_ESPUSB_CDC 0
#endif

#ifndef CONFIG_ESPUSB_MSC
#define CONFIG_ESPUSB_MSC 0
#endif

#ifndef CONFIG_ESPUSB_HID
#define CONFIG_ESPUSB_HID 0
#endif

#ifndef CONFIG_ESPUSB_MIDI
#define CONFIG_ESPUSB_MIDI 0
#endif

#ifndef CONFIG_ESPUSB_VENDOR
#define CONFIG_ESPUSB_VENDOR 0
#endif

#ifndef CONFIG_ESPUSB_DFU
#define CONFIG_ESPUSB_DFU 0
#endif

#ifndef CONFIG_ESPUSB_CUSTOM_CLASS
#define CONFIG_ESPUSB_CUSTOM_CLASS 0
#endif

#ifndef CONFIG_ESPUSB_CDC_RX_BUFSIZE
#define CONFIG_ESPUSB_CDC_RX_BUFSIZE 64
#endif

#ifndef CONFIG_ESPUSB_CDC_TX_BUFSIZE
#define CONFIG_ESPUSB_CDC_TX_BUFSIZE 64
#endif

#ifndef CONFIG_ESPUSB_MSC_BUFSIZE
#define CONFIG_ESPUSB_MSC_BUFSIZE 512
#endif

#ifndef CONFIG_ESPUSB_HID_BUFSIZE
#define CONFIG_ESPUSB_HID_BUFSIZE 16
#endif

#ifndef CONFIG_ESPUSB_VENDOR_RX_BUFSIZE
#define CONFIG_ESPUSB_VENDOR_RX_BUFSIZE 64
#endif

#ifndef CONFIG_ESPUSB_VENDOR_TX_BUFSIZE
#define CONFIG_ESPUSB_VENDOR_TX_BUFSIZE 64
#endif

#ifndef CONFIG_ESPUSB_MIDI_RX_BUFSIZE
#define CONFIG_ESPUSB_MIDI_RX_BUFSIZE 64
#endif

#ifndef CONFIG_ESPUSB_MIDI_TX_BUFSIZE
#define CONFIG_ESPUSB_MIDI_TX_BUFSIZE 64
#endif

#ifndef CONFIG_ESPUSB_DFU_BUFSIZE
#define CONFIG_ESPUSB_DFU_BUFSIZE 1024
#endif

//--------------------------------------------------------------------
// DEVICE CONFIGURATION
//--------------------------------------------------------------------
#define CFG_TUD_CDC 2
#define CFG_TUD_MSC CONFIG_ESPUSB_MSC
#define CFG_TUD_HID CONFIG_ESPUSB_HID
#define CFG_TUD_MIDI CONFIG_ESPUSB_MIDI
#define CFG_TUD_VENDOR 1
#define CFG_TUD_CUSTOM_CLASS CONFIG_ESPUSB_CUSTOM_CLASS
#define CFG_TUD_DFU_RT CONFIG_ESPUSB_DFU

//--------------------------------------------------------------------
// CDC FIFO CONFIGURATION
//--------------------------------------------------------------------
#define CFG_TUD_CDC_RX_BUFSIZE CONFIG_ESPUSB_CDC_RX_BUFSIZE
#define CFG_TUD_CDC_TX_BUFSIZE CONFIG_ESPUSB_CDC_TX_BUFSIZE

//--------------------------------------------------------------------
// MSC BUFFER CONFIGURATION
//
// NOTE: This is the block size for read/write operations via all
// defined callbacks.
//--------------------------------------------------------------------
#define CFG_TUD_MSC_BUFSIZE CONFIG_ESPUSB_MSC_BUFSIZE

//--------------------------------------------------------------------
// HID BUFFER CONFIGURATION
//
// NOTE: This should be sufficient to hold ID (if any) + Data
//--------------------------------------------------------------------
#define CFG_TUD_HID_BUFSIZE CONFIG_ESPUSB_HID_BUFSIZE

//--------------------------------------------------------------------
// VENDOR FIFO CONFIGURATION
//--------------------------------------------------------------------
#define CFG_TUD_VENDOR_RX_BUFSIZE (CONFIG_ESPUSB_VENDOR_RX_BUFSIZE * 2)
#define CFG_TUD_VENDOR_TX_BUFSIZE (CONFIG_ESPUSB_VENDOR_TX_BUFSIZE * 2)

//--------------------------------------------------------------------
// MIDI FIFO CONFIGURATION
//--------------------------------------------------------------------
#define CFG_TUD_MIDI_RX_BUFSIZE CONFIG_ESPUSB_MIDI_RX_BUFSIZE
#define CFG_TUD_MIDI_TX_BUFSIZE CONFIG_ESPUSB_MIDI_TX_BUFSIZE

#ifdef __cplusplus
}
#endif