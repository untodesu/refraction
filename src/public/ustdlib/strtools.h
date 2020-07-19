#ifndef USTDLIB_STRTOOLS_H
#define USTDLIB_STRTOOLS_H
#include <stddef.h>

namespace ustd
{
    class StrTools {
    public:
        static void *       memset(void *dest, int fill, size_t count);
        static void *       memcpy(void *dest, const void *src, size_t count);
        static int          memcmp(const void *a, const void *b, size_t count);

        static size_t       strlen(const char *str);
        static size_t       strnlen(const char *str, size_t max);
        static char *       strcpy(char *dest, const char *src);
        static char *       strncpy(char *dest, const char *src, size_t count);
        static int          strcmp(const char *a, const char *b);
        static int          stricmp(const char *a, const char *b);
        static const char * strchr(const char *str, char ch);
        static const char * strrchr(const char *str, char ch);
    };
}

#endif
