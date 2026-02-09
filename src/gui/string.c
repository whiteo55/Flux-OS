#include "gui.h"

// Memory allocation - simplified for kernel use
void* malloc(size_t size) {
    // Simple bump allocator - returns a fixed buffer
    static char heap[1024 * 1024];  // 1MB heap
    static size_t heap_offset = 0;
    
    if (heap_offset + size > sizeof(heap)) {
        return 0;  // Out of memory
    }
    
    void* ptr = &heap[heap_offset];
    heap_offset = (heap_offset + size + 3) & ~3;  // Align to 4 bytes
    return ptr;
}

void free(void* ptr) {
    // No-op in simple allocator
    (void)ptr;
}

// String length
size_t strlen(const char* str) {
    size_t len = 0;
    while (str[len]) len++;
    return len;
}

// String comparison
int strcmp(const char* s1, const char* s2) {
    while (*s1 && *s2) {
        if (*s1 != *s2) return *s1 - *s2;
        s1++;
        s2++;
    }
    return *s1 - *s2;
}

// String copy
char* strcpy(char* dest, const char* src) {
    char* d = dest;
    while ((*d++ = *src++) != '\0');
    return dest;
}

// Simple number to string
static char* itoa(int value, char* buffer, int base) {
    char* p = buffer;
    char* p1 = buffer;
    char tmp_char;
    int tmp_value;
    
    if (base < 2 || base > 16) return buffer;
    
    if (value < 0) {
        *p++ = '-';
        buffer++;
        value = -value;
    }
    
    do {
        tmp_value = value;
        value /= base;
        *p++ = "0123456789ABCDEF"[tmp_value - value * base];
    } while (value);
    
    *p = '\0';
    
    // Reverse
    p1 = buffer;
    while (p1 < p) {
        tmp_char = *p1;
        *p1++ = *--p;
        *p = tmp_char;
    }
    
    return buffer;
}

// Simple snprintf (limited implementation)
int snprintf(char* buf, size_t size, const char* fmt, ...) {
    // Use variadic helper
    int* args = (int*)&fmt;
    args++;  // Skip format string
    
    const char* p = fmt;
    char* out = buf;
    size_t written = 0;
    
    if (size == 0) return 0;
    
    while (*p && written < size - 1) {
        if (*p == '%' && *(p + 1)) {
            p++;
            char num_buf[32];
            
            switch (*p) {
                case 'd':
                case 'i':
                    itoa(*args++, num_buf, 10);
                    for (char* q = num_buf; *q && written < size - 1; q++) {
                        *out++ = *q;
                        written++;
                    }
                    break;
                case 'x':
                case 'X':
                    itoa(*args++, num_buf, 16);
                    for (char* q = num_buf; *q && written < size - 1; q++) {
                        *out++ = *q;
                        written++;
                    }
                    break;
                case 's':
                    {
                        const char* s = (const char*)*args++;
                        while (*s && written < size - 1) {
                            *out++ = *s++;
                            written++;
                        }
                    }
                    break;
                case 'c':
                    if (written < size - 1) {
                        *out++ = (char)*args++;
                        written++;
                    }
                    break;
                case '%':
                    if (written < size - 1) {
                        *out++ = '%';
                        written++;
                    }
                    break;
                default:
                    if (written < size - 1) {
                        *out++ = *p;
                        written++;
                    }
                    break;
            }
        } else {
            *out++ = *p;
            written++;
        }
        p++;
    }
    
    *out = '\0';
    
    return written;
}

