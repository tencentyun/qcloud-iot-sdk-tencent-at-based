/**
  ******************************************************************************
  * Copyright (c) 2019 Tencent. 
  * All rights reserved.
  ******************************************************************************

  ******************************************************************************
  * @file           : at_utils.c
  * @brief          : at send and debug api
  ******************************************************************************
*/
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include "hal_export.h"
#include "at_client.h"

#define __is_print(ch)       ((unsigned int)((ch) - ' ') < 127u - ' ')
#define WIDTH_SIZE           32

static char send_buf[AT_CMD_MAX_LEN];
static int  last_cmd_len = 0;

/**
 * dump hex format data to console device
 *
 * @param name name for hex object, it will show on log header
 * @param buf hex buffer
 * @param size buffer size
 */
void at_print_raw_cmd(const char *name, const char *buf, int size)
{
    int i, j;

    for (i = 0; i < size; i += WIDTH_SIZE)
    {
        HAL_Printf("%s: %04X-%04X: ", name, i, i + WIDTH_SIZE);
        for (j = 0; j < WIDTH_SIZE; j++)
        {
            if (i + j < size)
            {
                HAL_Printf("%02X ", buf[i + j]);
            }
            else
            {
                HAL_Printf("   ");
            }
            if ((j + 1) % 8 == 0)
            {
                HAL_Printf(" ");
            }
        }
        HAL_Printf("  ");
        for (j = 0; j < WIDTH_SIZE; j++)
        {
            if (i + j < size)
            {
                HAL_Printf("%c", __is_print(buf[i + j]) ? buf[i + j] : '.');
            }
        }
        HAL_Printf("\n\r");
    }
}

const char *at_get_last_cmd(int *cmd_size)
{
    *cmd_size = last_cmd_len;
    return send_buf;
}

int at_vprintf(const char *format, va_list args)
{
    last_cmd_len = vsnprintf(send_buf, sizeof(send_buf), format, args);

#ifdef AT_PRINT_RAW_CMD
    at_print_raw_cmd("send", send_buf, last_cmd_len);
#endif

    return at_send_data((uint8_t *)send_buf, last_cmd_len);
}

int at_vprintfln(const char *format, va_list args)
{
    int len;

    len = at_vprintf(format, args);

    at_send_data("\r\n", 2);

    return len + 2;
}

