/* 
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ha Thach (tinyusb.org)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include "tusb.h"
#include <pico/unique_id.h>

#define USB_VID   0xCAFE
#define USB_BCD   0x0200

static char serial_str[16];

/* A combination of interfaces must have a unique product id, since PC will save device driver after the first plug.
 * Same VID/PID with different interface e.g MSC (first), then CDC (later) will possibly cause system error on PC.
 *
 * Auto ProductID layout's Bitmap:
 *   [MSB]       MIDI | HID | MSC | CDC          [LSB]
 */
#define _PID_MAP(itf, n)  ( (CFG_TUD_##itf) << (n) )
#define USB_PID           (0x4000 | _PID_MAP(CDC, 0) | _PID_MAP(MSC, 1) | _PID_MAP(HID, 2) | \
                           _PID_MAP(MIDI, 3) | _PID_MAP(VENDOR, 4) )
#define DEV_MAP(itf, n)         ( (CFG_TUD_##itf) << (n) )

#define USBD_DEV 0x0100
                // ( 0x8000              +         /* 0x8011 */ \
                //                   DEV_MAP(CDC,     0) + \
                //                   DEV_MAP(HID,     4)   )


// String Descriptor Index
enum {
  STRING_LANGID = 0,
  STRING_MANUFACTURER,
  STRING_PRODUCT,
  STRING_SERIAL,
  STRING_CDC,
  STRING_MIDI,
  STRING_VENDOR,
  STRING_MANUFACTURER_MONOME,
  STRING_PRODUCT_MONOME,
  STRING_SERIAL_MONOME,
  STRING_LAST,
};

// array of pointer to string descriptors
// const static char const *string_desc_arr[] = {
char const* string_desc_arr [] = {
    (const char[]){0x09, 0x04}, // 0: supported language is English (0x0409)
    "monome",                 // 1: Manufacturer
    "iii",                    // 2: Product
    serial_str,               // 3: Serial Number
    "neotrellis cdc",         // 4: Serial Port
    "neotrellis midi",        // 5: MIDI
    "neotrellis webusb",      // 6: Vendor
    "monome",                 // 7: Manufacturer Monome
    "grid",                   // 8: Product Monome
    ""
};

enum {
  USB_MODE_III,
  USB_MODE_MONOME,
};
extern uint8_t g_monome_mode;

//--------------------------------------------------------------------+
// Device Descriptors
//--------------------------------------------------------------------+
tusb_desc_device_t const desc_devices[2] = 
{   //// III
    {.bLength = sizeof(tusb_desc_device_t),
    .bDescriptorType    = TUSB_DESC_DEVICE,
    .bcdUSB             = 0x0200,
    .bDeviceClass       = TUSB_CLASS_MISC,
    .bDeviceSubClass    = MISC_SUBCLASS_COMMON,
    .bDeviceProtocol    = MISC_PROTOCOL_IAD,
    .bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,

    .idVendor           = USB_VID,
    .idProduct          = 0x1101,
    .bcdDevice          = 0x0100,

    .iManufacturer      = STRING_MANUFACTURER,
    .iProduct           = STRING_PRODUCT,
    .iSerialNumber      = STRING_SERIAL,

    .bNumConfigurations = 0x01},

    //// MONOME
    {.bLength = sizeof(tusb_desc_device_t),
    .bDescriptorType  = TUSB_DESC_DEVICE,
    .bcdUSB           = 0x0200,
    .bDeviceClass     = TUSB_CLASS_CDC,
    .bDeviceSubClass  = MISC_SUBCLASS_COMMON,
    .bDeviceProtocol  = MISC_PROTOCOL_IAD,
    .bMaxPacketSize0  = CFG_TUD_ENDPOINT0_SIZE,

    .idVendor         = USB_VID,
    .idProduct        = 0x1110,
    .bcdDevice        = 0x0100,
    .iManufacturer    = STRING_MANUFACTURER_MONOME,
    .iProduct         = STRING_PRODUCT_MONOME,
    .iSerialNumber    = STRING_SERIAL_MONOME,
    .bNumConfigurations = 0x01
  }
};

// Invoked when received GET DEVICE DESCRIPTOR
// Application return pointer to descriptor
uint8_t const *tud_descriptor_device_cb(void) { return (uint8_t const *)&desc_devices[g_monome_mode]; }


//--------------------------------------------------------------------+
// Configuration Descriptor
//--------------------------------------------------------------------+

// IAD macro for MIDI on Windows (TinyUSB doesn't provide one like it does for CDC)
#define TUD_MIDI_IAD_DESCRIPTOR(_itfnum, _stridx) \
  8, TUSB_DESC_INTERFACE_ASSOCIATION, _itfnum, 2, TUSB_CLASS_AUDIO, AUDIO_SUBCLASS_CONTROL, AUDIO_FUNC_PROTOCOL_CODE_UNDEF, _stridx

enum
{
  ITF_NUM_CDC = 0,
  ITF_NUM_CDC_DATA,
  ITF_NUM_MIDI,
  ITF_NUM_MIDI_STREAMING,
  ITF_NUM_TOTAL
};

// #define CONFIG_TOTAL_LEN  (TUD_CONFIG_DESC_LEN + TUD_MIDI_DESC_LEN)
#define CONFIG_TOTAL_LEN_CDC  (TUD_CONFIG_DESC_LEN + TUD_CDC_DESC_LEN)
#define CONFIG_TOTAL_LEN_MIDI  (TUD_CONFIG_DESC_LEN + TUD_MIDI_DESC_LEN + TUD_CDC_DESC_LEN + 8)

#if CFG_TUSB_MCU == OPT_MCU_LPC175X_6X || CFG_TUSB_MCU == OPT_MCU_LPC177X_8X || CFG_TUSB_MCU == OPT_MCU_LPC40XX
  // LPC 17xx and 40xx endpoint type (bulk/interrupt/iso) are fixed by its number
  // 0 control, 1 In, 2 Bulk, 3 Iso, 4 In etc ...
  #define EPNUM_MIDI_OUT   0x04
  #define EPNUM_MIDI_IN   0x04

  #define EPNUM_CDC_NOTIF   0x81
  #define EPNUM_CDC_OUT     0x02
  #define EPNUM_CDC_IN      0x82

#else
  #define EPNUM_CDC_NOTIF   0x81
  #define EPNUM_CDC_OUT     0x02
  #define EPNUM_CDC_IN      0x82
  

  #define EPNUM_MIDI_OUT   0x05
  #define EPNUM_MIDI_IN   0x85
#endif

uint8_t const desc_fs_configuration_0[] =
{
  // Config number, interface count, string index, total length, attribute, power in mA
  TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL, STRING_LANGID, CONFIG_TOTAL_LEN_MIDI, TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP, 100),

  // 1st CDC: Interface number, string index, EP notification address and size, EP data address (out, in) and size.
  TUD_CDC_DESCRIPTOR(ITF_NUM_CDC, STRING_CDC, EPNUM_CDC_NOTIF, 8, EPNUM_CDC_OUT, EPNUM_CDC_IN, 64),

  // Interface Association Descriptor for MIDI (audio control + MIDI streaming)
  TUD_MIDI_IAD_DESCRIPTOR(ITF_NUM_MIDI, 0),

  // Interface number, string index, EP Out & EP In address, EP size
  TUD_MIDI_DESCRIPTOR(ITF_NUM_MIDI, STRING_MIDI, EPNUM_MIDI_OUT, EPNUM_MIDI_IN, 64)
  // TUD_MIDI_DESCRIPTOR(ITF_NUM_MIDI, STRING_MIDI, EPNUM_MIDI_OUT, (0x80 | EPNUM_MIDI_IN), 64)
};
uint8_t const desc_fs_configuration_1[] =
{
  // Config number, interface count, string index, total length, attribute, power in mA
  TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_CDC_DATA + 1 , STRING_LANGID, CONFIG_TOTAL_LEN_CDC, 0x00, 100),

  // 1st CDC: Interface number, string index, EP notification address and size, EP data address (out, in) and size.
  TUD_CDC_DESCRIPTOR(ITF_NUM_CDC, STRING_CDC, EPNUM_CDC_NOTIF, 8, EPNUM_CDC_OUT, EPNUM_CDC_IN, 64),
};


// Invoked when received GET CONFIGURATION DESCRIPTOR
// Application return pointer to descriptor
// Descriptor contents must exist long enough for transfer to complete
uint8_t const *tud_descriptor_configuration_cb(uint8_t index)
{
  (void) index; // for multiple configurations
  return g_monome_mode == 0 ? desc_fs_configuration_0 : desc_fs_configuration_1;
}

//--------------------------------------------------------------------+
// String Descriptors
//--------------------------------------------------------------------+


static uint16_t _desc_str[32 + 1];

static uint8_t fill_serial_str(const char *fmt) {
  pico_unique_board_id_t id;
  pico_get_unique_board_id(&id);
  uint64_t idx;
  memcpy(&idx, id.id, sizeof(idx));
  int serialnum = (int)((idx + 1) % 10000000ull);
  if (serialnum < 1000000)
    serialnum += 1000000; // ensure 7 digits
  char temp[16];
  uint8_t chr_count = (uint8_t)snprintf(temp, sizeof(temp), fmt, serialnum);
  for (uint8_t i = 0; i < chr_count; i++)
    _desc_str[1 + i] = temp[i];
  return chr_count;
}

// Invoked when received GET STRING DESCRIPTOR request
// Application return pointer to descriptor, whose contents must exist long enough for transfer to complete

uint16_t const *tud_descriptor_string_cb(uint8_t index, uint16_t langid)
{
  (void)langid;
  uint8_t chr_count;
  memset(_desc_str, 0, sizeof(_desc_str));
  if (index == 0) {
    memcpy(&_desc_str[1], string_desc_arr[0], 2);
    chr_count = 1;
  } else if (index == STRING_SERIAL_MONOME) {
    chr_count = fill_serial_str("m%07d");
  } else if (index == STRING_SERIAL) {
    chr_count = fill_serial_str("%07d");
  } else if (index < STRING_LAST) {
    const char *str = string_desc_arr[index];
    chr_count = strlen(str);
    if (chr_count > 31)
      chr_count = 31;
    for (uint8_t i = 0; i < chr_count; i++) {
      _desc_str[1 + i] = str[i];
    }
  } else
  return NULL;
  _desc_str[0] = (TUSB_DESC_STRING << 8) | (2 * chr_count + 2);
  return _desc_str;
}
