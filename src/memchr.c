#include <stddef.h>

void * memchr(const void *s, unsigned char c,size_t n ){
    if (n != 0) {
        const unsigned char *p = (unsigned char *) s;

        do {
            if (*p++ == c)
                return ((void *)(p - 1));
        } while (--n != 0);
    }
    return (NULL);
}