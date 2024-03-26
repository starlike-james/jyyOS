#include <klib-macros.h>
#include <klib.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s) {
    int i = 0;
    while (s[i] != '\0') {
        i++;
    }
    return i;
    /** panic("Not implemented"); */
}

char *strcpy(char *dst, const char *src) {
    size_t len = strlen(src);
    int i;
    for (i = 0; i < len; i++) {
        dst[i] = src[i];
    }
    dst[i] = '\0';
    return dst;
    /** panic("Not implemented"); */
}

char *strncpy(char *dst, const char *src, size_t n) {
    int i;
    for (i = 0; i < n && src[i] != '\0'; i++) {
        dst[i] = src[i];
    }
    for (; i < n; i++) {
        dst[i] = '\0';
    }
    return dst;
    /** panic("Not implemented"); */
}

char *strcat(char *dst, const char *src) {
    size_t dst_len = strlen(dst);
    size_t src_len = strlen(src);
    int i;
    for (i = 0; i < src_len; i++) {
        dst[dst_len + i] = src[i];
    }
    dst[dst_len + i] = '\0';
    return dst;
    /** panic("Not implemented"); */
}

int strcmp(const char *s1, const char *s2) {
    int i;
    size_t s1_len = strlen(s1);
    size_t s2_len = strlen(s2);
    size_t min_len = (s1_len < s2_len) ? s1_len : s2_len;
    for (i = 0; i <= min_len; i++) {
        if (s1[i] == s2[i]) {
            continue;
        } else if ((unsigned char)s1[i] > (unsigned char)s2[i]) {
            return 1;
        } else if ((unsigned char)s1[i] < (unsigned char)s2[i]) {
            return -1;
        }
    }
    return 0;
    /** panic("Not implemented"); */
}

int strncmp(const char *s1, const char *s2, size_t n) {
    int i;
    size_t s1_len = strlen(s1);
    size_t s2_len = strlen(s2);
    size_t min_len = (s1_len < s2_len) ? s1_len : s2_len;
    for (i = 0; i <= min_len && i < n; i++) {
        if (s1[i] == s2[i]) {
            continue;
        } else if ((unsigned char)s1[i] > (unsigned char)s2[i]) {
            return 1;
        } else if ((unsigned char)s1[i] < (unsigned char)s2[i]) {
            return -1;
        }
    }
    return 0;
    /** panic("Not implemented"); */
}

void *memset(void *s, int c, size_t n) {
    uint8_t *p = (uint8_t *)s;
    uint8_t *pc = (uint8_t *)&c;
    int i;
    for (i = 0; i < n; i++) {
        p[i] = *pc;
    }
    return s;
    /** panic("Not implemented"); */
}

void *memmove(void *dst, const void *src, size_t n) {
    uint8_t *pd = (uint8_t *)dst;
    uint8_t *ps = (uint8_t *)src;
    int i;
    // putch('b');
    if (dst < src) {
        putch('a');
        for (i = 0; i < n; i++) {
            pd[i] = ps[i];
        }
    } else {
        for (i = n - 1; i >= 0; i--) {
            pd[i] = ps[i];
        }
    }
    return dst;
    /** panic("Not implemented"); */
}

void *memcpy(void *out, const void *in, size_t n) {
    uint8_t *pd = (uint8_t *)out;
    uint8_t *ps = (uint8_t *)in;
    int i;
    for (i = 0; i < n; i++) {
        pd[i] = ps[i];
    }
    return out;
    // panic("Not implemented");
}

int memcmp(const void *s1, const void *s2, size_t n) {
    uint8_t *p1 = (uint8_t *)s1;
    uint8_t *p2 = (uint8_t *)s2;
    int i;
    for (i = 0; i < n; i++) {
        if (p1[i] == p2[i]) {
            continue;
        } else if ((unsigned char)p1[i] > (unsigned char)p2[i]) {
            return 1;
        } else if ((unsigned char)p1[i] < (unsigned char)p2[i]) {
            return -1;
        }
    }
    return 0;
    /** panic("Not implemented"); */
}

#endif
