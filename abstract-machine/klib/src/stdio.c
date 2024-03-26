#include <am.h>
#include <klib-macros.h>
#include <klib.h>
#include <stdarg.h>

enum { NONE = 0, ZERO };
enum { LONG = 1, LLONG };

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

static int itoa(uint64_t n, char *str, bool sig, uint32_t radix) {
    // uint64_t t;
    int is_neg = 0, nlen = 0, strlen = 0;
    char strtmp[65] = "";
    if (sig && (int64_t)n < 0) {
        n = -n;
        str[0] = '-';
        is_neg = 1;
    }
    do {
        uint64_t mod = n % radix;
        strtmp[nlen] = mod > 9 ? mod - 10 + 'a' : mod + '0';
        nlen++;
        n = n / radix;
    } while (n);
    strlen = is_neg + nlen;
    for (int i = is_neg, j = nlen - 1; i < strlen; i++, j--) {
        str[i] = strtmp[j];
    }
    str[strlen] = '\0';
    return strlen;
}
int printf(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    // int count=vsprintf(NULL,fmt,ap);
#define PRINT_WRITE
#ifndef PRINT_WRITE
#define WRITE_STR(d, s) strcpy(d, s);
#define WRITE_CH(d, c) *(d) = c;
#else
#define WRITE_STR(d, s) putstr(s)
#define WRITE_CH(d, c) putch(c)
#endif
    int i = 0;
    size_t aplen = strlen(fmt);
    // char *s=NULL;
    char buf[128];
    // char c;
    // uint64_t d;
    int count = 0;
    int flag = NONE;
    int modifier = NONE;
    while (i < aplen) {
        if (fmt[i] != '%') {
            WRITE_CH(out + count, fmt[i]);
            i++;
            count += 1;
        } else if (fmt[i] == '%') {
            i++;

            // flag
            switch (fmt[i]) {
                case '0': {
                    flag = ZERO;
                    i++;
                    break;
                }
                default: {
                    flag = NONE;
                    break;
                }
            }

            // field width
            char strwid[32] = " ";
            int wlen = 0;
            while ('0' < fmt[i] && fmt[i] < '9') {
                strwid[wlen] = fmt[i];
                wlen++;
                i++;
            }
            strwid[wlen] = '\0';
            int wid = atoi(strwid);

            // length modifier
            int lcnt = 0;
            while (fmt[i] == 'l') {
                lcnt++;
                i++;
            }
            switch (lcnt) {
                case 0: {
                    modifier = NONE;
                    break;
                }
                case 1: {
                    modifier = LONG;
                    break;
                }
                case 2: {
                    modifier = LLONG;
                    break;
                }
            }

            // conversion specifiers
            switch (fmt[i++]) {
                case 's': {
                    char *s = va_arg(ap, char *);
                    WRITE_STR(out + count, s);
                    count += strlen(s);
                    break;
                }
                case 'd': {
                    int len = 0;
                    if (modifier == NONE) {
                        int d = va_arg(ap, int);
                        len = itoa(d, buf, 1, 10);
                    } else if (modifier == LONG) {
                        long int d = va_arg(ap, long int);
                        len = itoa(d, buf, 1, 10);
                    } else if (modifier == LLONG) {
                        long long int d = va_arg(ap, long long int);
                        len = itoa(d, buf, 1, 10);
                    }
                    int pad = wid - len;
                    count += pad > 0 ? pad : 0;
                    while (pad > 0) {
                        if (flag == ZERO)
                            putch('0');
                        else if (flag == NONE)
                            putch(' ');
                        pad--;
                    }
                    WRITE_STR(out + count, buf);
                    count += len;
                    break;
                }
                case 'c': {
                    char c = va_arg(ap, int);
                    WRITE_CH(out + count, c);
                    count += 1;
                    break;
                }
                case 'x': {
                    int len = 0;
                    if (modifier == NONE) {
                        unsigned int x = va_arg(ap, unsigned int);
                        len = itoa(x, buf, 0, 16);
                    } else if (modifier == LONG) {
                        unsigned long int x = va_arg(ap, unsigned long int);
                        len = itoa(x, buf, 0, 16);
                    } else if (modifier == LLONG) {
                        unsigned long long int x =
                            va_arg(ap, unsigned long long int);
                        len = itoa(x, buf, 0, 16);
                    }
                    int pad = wid - len;
                    count += pad > 0 ? pad : 0;
                    while (pad > 0) {
                        if (flag == ZERO)
                            putch('0');
                        else if (flag == NONE)
                            putch(' ');
                        pad--;
                    }
                    WRITE_STR(out + count, buf);
                    count += len;
                    break;
                }
                case 'p': {
                    uintptr_t p = va_arg(ap, uintptr_t);
                    int len = itoa(p, buf, 0, 16);
                    WRITE_STR(out + count, "0x");
                    WRITE_STR(out + count, buf);
                    count += (len + 2);
                    break;
                }
                default: {
                    putstr("NOT IMPLEMENTED SPECIFIER\n");
                    break;
                }
            }
        }
    }
    WRITE_CH(out + count, fmt[i]);
#undef PRINT_WRITE
#undef WRITE_STR
#undef WRITE_CH

    va_end(ap);
    return count;
}

int vsprintf(char *out, const char *fmt, va_list ap) {
    panic("Not implemented");
}

int sprintf(char *out, const char *fmt, ...) {
    panic("Not implemented");
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
    panic("Not implemented");
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
    panic("Not implemented");
}

#endif
