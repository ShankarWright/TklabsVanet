/**
 *	@file	dlib.c
 *
 *	@brief	The Doug C Library
 */
/*----------------------------------------------------------------------------
 *
 *  The initial developer of the original code is TK Labs, Inc. (tkLABS)
 *
 *  Contains unpublished trade secrets of TK Labs, Inc. Sunrise FL, USA
 *
 *  (C) Copyright 2014 TK Labs, Inc.
 *
 *             ALL RIGHTS RESERVED
 *
 *----------------------------------------------------------------------------
 */

#include <string.h>
#include <asf.h>
#include "vanet.h"

#define PRINTF_MAX_CONVERSION 32

int dlib_vsnprintf(char *str, size_t size, const char *fmt, va_list ap)
{
    size_t sidx;
    long i, number;
    char ch;
    
    char *tmp;
    char tmpbuf[PRINTF_MAX_CONVERSION+1];
    long tmplen;
    long prefix_len;

    char flag_alt;
    char flag_width;
    long width;
    char flag_zero_pad;
    
    /* Index within str[size] */
    sidx = 0;
    *str = '\0';
    
    while (sidx < size)
    {
        ch = *fmt++;
        
        switch (ch)
        {
        case '%':
            /* Begin of format! */
            flag_alt = 0;
            flag_width = 0;
            width = 0;
            flag_zero_pad = 0;
            prefix_len = 0;
            
            restart:
            ch = *fmt++;
            switch (ch)
            {
            case '#':
                /* Alternate Form */
                flag_alt = 1;
                goto restart;
                break;
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                flag_width = 1;
                /* convert string to number */
                width = dlib_string2long(--fmt, &tmp, 10);
                /* move fmt up to the 1st char *after* number */
                fmt = tmp;
                goto restart;
                break;
            case '0':
                /* Zero Pad */
                flag_zero_pad = 1;
                goto restart;
                break;
            case '%':
                str[sidx++] = ch;
                break;
            case 'c':
                ch = va_arg(ap, int);
                str[sidx++] = ch;
                break;
            case 'b':
                number = va_arg(ap, int);
                tmp = dlib_long2string(tmpbuf, sizeof(tmpbuf), number, 2);
                goto copy_out;
                break;
            case 'o':
                if (flag_alt)
                {
                    str[sidx++] = '0';
                    prefix_len = 1;
                }
                number = va_arg(ap, int);
                tmp = dlib_long2string(tmpbuf, sizeof(tmpbuf), number, 8);
                goto copy_out;
                break;
            case 'x':
            case 'X':
                if (flag_alt)
                {
                    str[sidx++] = '0';
                    str[sidx++] = 'x';
                    prefix_len = 2;
                }
                number = va_arg(ap, int);
                tmp = dlib_long2string(tmpbuf, sizeof(tmpbuf), number, 16);
                goto copy_out;
                break;
            case 'u':
                number = va_arg(ap, int);
                tmp = dlib_long2string(tmpbuf, sizeof(tmpbuf), number, 10);
                goto copy_out;
                break;
            case 'd':
            case 'i':
                number = va_arg(ap, int);
                if (number < 0)
                {
                    number = -1 * number;
                    str[sidx++] = '-';
                }
                tmp = dlib_long2string(tmpbuf, sizeof(tmpbuf), number, 10);
                goto copy_out;
                break;
            case 'p':
                str[sidx++] = '0';
                str[sidx++] = 'x';
                number = va_arg(ap, unsigned long);
                tmp = dlib_long2string(tmpbuf, sizeof(tmpbuf), number, 16);
                goto copy_out;
                break;
            case 's':
                /* copy string into output buffer */
                tmp = va_arg(ap, char *);
                if (!tmp)
                {
                    tmp = "(null)";
                }
                copy_out:
                tmplen = strlen(tmp);
                /* handle width & pad */
                if (flag_width && (tmplen + prefix_len) < width)
                {
                    for (i=(tmplen + prefix_len); i<width; i++)
                    {
                        if (flag_zero_pad)
                        {
                            str[sidx++] = '0';
                        }
                        else
                        {
                            str[sidx++] = ' ';
                        }
                    }
                }
                for (i=0; i<tmplen; i++)
                {
                    str[sidx++] = tmp[i];
                }
                break;
                
            default:
                break;
            }
            break;
            
        case '\0':
            /* End of fmt */
            goto end_of_format;
            break;
            
        default:
            /* Just text in the fmt */
            str[sidx++] = ch;
            break;
        }
    }
    
    end_of_format:
    /* terminate returned string */
    str[sidx] = '\0';
    
    /* return length of string */
    return sidx;
}

char* dlib_long2string(char *str, size_t size, uint32_t num, int base)
{
    /* This routine works the output buffer (str) from the back-forward */

    int negative = 0;
    char digit;

    if (base < 2 || base > 36)
    {
        return 0;
    }

    if (num < 0)
    {
        num = -num;
        negative = 1;
    }

    // Terminate return string
    str[--size] = '\0';

    size--;
    do
    {
        digit = num % base;
        if (digit < 10)
        {
            str[size] = digit + '0';
        }
        else
        {
            str[size] = digit - 10 + 'a';
        }
        num = num / base;
    }
    while (size-- > 0 && num != 0);

    if (negative)
    {
        str[size--] = '-';
    }

    return str+size+1;
}

long dlib_string2long(const char *sptr, char **eptr, int base)
{
    unsigned long int num = 0;
    int negative = 0;
    int digit;
    
    /* Skip leading whitespace */
    while (*sptr == ' ' || *sptr == '\t')
    {
        sptr++;
    }
    
    /* Get sign (if present) */
    if (*sptr == '-')
    {
        negative = 1;
        sptr++;
    }
    else if (*sptr == '+')
    {
        sptr++;
    }
    
    /* If base is 0, figure it out */
    if (base == 0)
    {
        if (*sptr == '0') /* octal or hex */
        {
            sptr++;
            if (*sptr == 'x') /* it's hex */
            {
                sptr++;
                base = 16;
            }
            else
            {
                base = 8;
            }
        }
        else
        {
            base = 10;
        }
    }
    else if (base == 16)
    {
        /* skip 0x if present */
        if ((sptr[0] == '0' && sptr[1] == 'x'))
        {
            sptr += 2;
        }
    }
    
    /* Iterate string and generate number */
    while (*sptr)
    {
        digit = *sptr;
        if (digit >= '0' && digit <= '9')
        {
            digit = digit - '0';
        }
        else
        {
            if (digit > 'Z')
            {
                // Lower case 0x61-0x7a
                //PRINTF("uc digit = (%#x)\n", digit);
                digit = digit - 87;
            }
            else
            {
                // Upper case 0x41-0x5a
                //PRINTF("lc digit = (%#x)\n", digit);
                digit = digit - 55;
            }
        }
        if (digit < 0 || digit >= base)
        {
            break;
        }
        num = num * base + digit;
        sptr++;
    }
    
    /* Set end/next pointer if valid */
    if (eptr)
    {
        *eptr = (char *)sptr;
    }
    
    /* return actual number */
    if (negative)
    {
        return -num;
    }
    else
    {
        return num;
    }
}
