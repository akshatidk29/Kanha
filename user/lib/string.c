#include "string.h"

size_t strlen(const char *str){
    if(str == NULL)
        return 0;

    size_t len = 0;
    while(str[len] != '\0')
        len++;

    return len;
}

int strcmp(const char *str1, const char *str2){
    if(str1 == NULL && str2 == NULL)
        return 0;

    if(str1 == NULL)
        return -1;

    if(str2 == NULL)
        return 1;

    size_t i = 0;
    while(1){
        unsigned char c1 = (unsigned char)str1[i];
        unsigned char c2 = (unsigned char)str2[i];

        if(c1 != c2)
            return c1 - c2;

        if(c1 == '\0')
            return 0;

        i++;
    }
}

int strncmp(const char *str1, const char *str2, size_t n){
    if(n == 0 || (str1 == NULL && str2 == NULL))
        return 0;

    if(str1 == NULL)
        return -1;

    if(str2 == NULL)
        return 1;

    for(size_t i = 0; i < n; i++){
        unsigned char c1 = (unsigned char)str1[i];
        unsigned char c2 = (unsigned char)str2[i];

        if(c1 != c2)
            return c1 - c2;

        if(c1 == '\0')
            return 0;
    }

    return 0;
}

char *strcpy(char *dest, const char *src){
    if(dest == NULL || src == NULL)
        return NULL;

    size_t len = strlen(src);
    for(size_t i = 0; i < len; i++){
        dest[i] = src[i];
    }
    dest[len] = '\0';

    return dest;
}

char *strcat(char *dest, const char *src) {
    if (dest == NULL)
        return NULL;
    if (src == NULL)
        return dest;

    char *d = dest;
    while (*d != '\0')
        d++;
    while ((*d++ = *src++) != '\0')
        ;
    return dest;
}

char *strncpy(char *dest, const char *src, size_t n) {
    if (!dest || !src) 
        return NULL;

    size_t i = 0;
    for (; i < n && src[i] != '\0'; i++) 
        dest[i] = src[i];

    for (; i < n; i++) 
        dest[i] = '\0'; 
    
    return dest;
}

void *memcpy(void *dest, const void *src, size_t n){
    if(dest == NULL || src == NULL)
        return NULL;
    
    uint8_t *s = (uint8_t*)src;
    uint8_t *d = (uint8_t*)dest;
    
    for(size_t i = 0; i < n; i++){
        d[i] = s[i];
    }

    return dest;
}

void *memset(void *dest, int c, size_t n){
    if(dest == NULL)
        return NULL;

    uint8_t *d = (uint8_t*)dest;
    for(size_t i = 0; i < n; i++){
        d[i] = (uint8_t)c;
    }

    return dest;
}
