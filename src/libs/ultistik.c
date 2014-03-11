/*
 * ultistik.c
 *
 *  Created on: Mar 7, 2014
 *      Author: katie
 */

/* C */
#include <stdio.h>
#include <string.h>

/* Unix */
#include <json.h>
#include <libusb-1.0/libusb.h>

/* Local */
#include "ultistik.h"

bool
isULTISTIK (json_object *jobj)
{
  json_object* tmp = NULL;
  if (json_object_object_get_ex(jobj, "version", &tmp))
  {
    if (json_object_get_int(tmp) == USTIK_VERSION)
    {
      if (json_object_object_get_ex(jobj, "map", &tmp) &&
          json_object_object_get_ex(jobj, "map size", &tmp) &&
          json_object_object_get_ex(jobj, "restrictor", &tmp) &&
          json_object_object_get_ex(jobj, "flash", &tmp) &&
          json_object_object_get_ex(jobj, "borders", &tmp) &&
          json_object_object_get_ex(jobj, "device", &tmp) &&
          json_object_object_get_ex(jobj, "product", &tmp))
      {
        /* product needs to be last so we can do this check */
        if (strncmp(json_object_get_string(tmp), USTIK_PRODUCT_STR, 4) == 0)
        {
          return true;
        }
      }
    }
  }

  return false;
}

char
convertULTISTIK (json_object *jobj)
{
  char retval = 0x00;
  const char* str = json_object_get_string(jobj);

  if (strlen(str) > 0)
  {
    if (!strcasecmp(str, "-"))
      retval = 0x00;
    if (!strcasecmp(str, "C"))
      retval = 0x01;
    if (!strcasecmp(str, "N"))
      retval = 0x02;
    if (!strcasecmp(str, "NE"))
      retval = 0x03;
    if (!strcasecmp(str, "E"))
      retval = 0x04;
    if (!strcasecmp(str, "SE"))
      retval = 0x05;
    if (!strcasecmp(str, "S"))
      retval = 0x06;
    if (!strcasecmp(str, "SW"))
      retval = 0x07;
    if (!strcasecmp(str, "W"))
      retval = 0x08;
    if (!strcasecmp(str, "NW"))
      retval = 0x09;
    if (!strcasecmp(str, "*"))
      retval = 0x0A;
  }

    return retval;
}

bool updateBoardULTISTIK (json_object* jobj)
{
  int idx     = 0;
  int itemidx = 0;
  int ret     = false;

  bool result = true;

  unsigned char data[USTIK_DATA_SIZE];

  libusb_context *ctx = NULL;
  struct libusb_device_handle *handle = NULL;

  json_object *innerobj  = NULL;
  json_object *item = NULL;

  /* Open USB communication */
  ret = libusb_init(&ctx);
  if (ret < 0)
  {
    printf("libusb_init failed: %i\n", ret);
    result = false;
    goto error;
  }
  libusb_set_debug(ctx, 3);

  handle = libusb_open_device_with_vid_pid(ctx, USTIK_VENDOR, USTIK_PRODUCT);
  if (!handle)
  {
    printf("Unable to open Ultistik device\n");
    result = false;
    goto error;
  }

  /* detach the kernel driver */
  if (libusb_kernel_driver_active(handle, USTIK_INTERFACE) == 1)
  {
    printf("Kernel Driver Active\n");
    if (libusb_detach_kernel_driver(handle, USTIK_INTERFACE) == 0) //detach it
      printf("Kernel Driver Detached!\n");
  }

  ret = libusb_claim_interface(handle, USTIK_INTERFACE);
  if (ret < 0)
  {
    printf("Unable to claim interface\n");
  }

  memset(data, 0, sizeof(data));
  data[0] = 0x50;

  json_object_object_get_ex(jobj, "map size", &innerobj);
  data[1] = json_object_get_int (innerobj);

  // Restrictor: false off(0x10), true on(0x09)
  json_object_object_get_ex(jobj, "restrictor", &innerobj);
  data[2] = (json_object_get_boolean(innerobj)? 0x09 : 0x10);

  // Flash: false RAM(0xFF), true FLASH(0x00)  (Support in Firmware 2.2)
  json_object_object_get_ex(jobj, "flash", &innerobj);
  data[95] = (json_object_get_boolean(innerobj)? 0x00 : 0xFF);

  // Borders 3 - 10
  itemidx = 3;
  json_object_object_get_ex(jobj, "borders", &innerobj);
  for (idx = 0; idx < json_object_array_length(innerobj); ++ idx)
  {
    item = json_object_array_get_idx(innerobj, idx);
    data[itemidx] = json_object_get_int(item);
    ++itemidx;
  }

  // Map
  itemidx = 11;
  json_object_object_get_ex(jobj, "map", &innerobj);
  for (idx = 0; idx < json_object_array_length(innerobj); ++ idx)
  {
    item = json_object_array_get_idx(innerobj, idx);
    data[itemidx] = convertULTISTIK(item);
    ++itemidx;
  }

  ret = libusb_control_transfer(handle,
                                USTIK_REQUEST_TYPE_1,
                                USTIK_REQUEST_1,
                                1,
                                0,
                                NULL,
                                0,
                                USTIK_TIMEOUT);

  for (idx = 0; idx < 3; ++ idx)
  {
    ret = libusb_control_transfer(handle,
                                  USTIK_REQUEST_TYPE_1,
                                  USTIK_REQUEST_2,
                                  0,
                                  0,
                                  data + (32*idx),
                                  USTIK_MESG_LENGTH,
                                  USTIK_TIMEOUT);

    ret = libusb_control_transfer(handle,
                                  USTIK_REQUEST_TYPE_2,
                                  USTIK_REQUEST_3,
                                  0,
                                  0,
                                  NULL,
                                  0,
                                  USTIK_TIMEOUT);
  }

  ret = libusb_control_transfer(handle,
                                USTIK_REQUEST_TYPE_1,
                                USTIK_REQUEST_1,
                                0,
                                0,
                                NULL,
                                0,
                                USTIK_TIMEOUT);

  libusb_release_interface(handle, USTIK_INTERFACE);

exit:
  libusb_close(handle);
  libusb_exit(ctx);
  return result;

error:
  libusb_exit(ctx);
  return result;
}
