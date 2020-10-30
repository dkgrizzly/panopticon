// This is free and unencumbered software released into the public domain.
//
// Anyone is free to copy, modify, publish, use, compile, sell, or
// distribute this software, either in source code form or as a compiled
// binary, for any purpose, commercial or non-commercial, and by any
// means.

#include <stdarg.h>
#include <stdint.h>
#include <strings.h>

extern long insn();

// extern int printf(const char *format, ...);

extern void *memcpy(void *dest, const void *src, long n);
extern char *strcpy(char *dest, const char *src);
extern char *strncpy(char *dest, const char *src, int maxlen);
extern char *strncat(char *dest, const char *src, int maxlen);
extern char *strchr(char *haystack, const char needle);
extern char *strrchr(char *haystack, const char needle);
extern int strncmp(const char *s1, const char *s2, int maxlen);
extern int strcmp(const char *s1, const char *s2);
extern int strlen(const char *s);
extern int isprint(int c);

long insn()
{
   int insns;
   asm volatile ("rdinstret %0" : "=r"(insns));
   // printf("[insn() -> %d]", insns);
   return insns;
}

void *memcpy(void *aa, const void *bb, long n)
{
   // printf("**MEMCPY**\n");
   char *a = aa;
   const char *b = bb;
   while (n--) *(a++) = *(b++);
   return aa;
}

void memset(void *s, int c, long n)
{
    uint8_t *ptr = (uint8_t *)s;
    while (n--) *ptr++ = c; 
}

int isprint(int c) {
    return (c > 0x1f)&&(c < 0x7f);
}

char *strcpy(char* dst, const char* src)
{
   char *r = dst;

   while ((((uint32_t)dst | (uint32_t)src) & 3) != 0)
   {
      char c = *(src++);
      *(dst++) = c;
      if (!c) return r;
   }

   while (1)
   {
      uint32_t v = *(uint32_t*)src;

      if (__builtin_expect((((v) - 0x01010101UL) & ~(v) & 0x80808080UL), 0))
      {
         dst[0] = v & 0xff;
         if ((v & 0xff) == 0)
            return r;
         v = v >> 8;

         dst[1] = v & 0xff;
         if ((v & 0xff) == 0)
            return r;
         v = v >> 8;

         dst[2] = v & 0xff;
         if ((v & 0xff) == 0)
            return r;
         v = v >> 8;

         dst[3] = v & 0xff;
         return r;
      }

      *(uint32_t*)dst = v;
      src += 4;
      dst += 4;
   }
}

char *strncpy(char* dst, const char* src, int maxlen)
{
   char *r = dst;

   for (;;)
   {
      char c = *(src++);
      *(dst++) = c;
      maxlen--;
      if (maxlen <= 1) {
        *dst = 0;
        return r;
      }
      if (!c) return r;
   }
}

int strlen(const char *s)
{
  int i = 0;
  for(;;) {
    char c = *(s++);
    if(!c) return i;
    i++;
  }
}

int strcmp(const char *s1, const char *s2)
{
   while ((((uint32_t)s1 | (uint32_t)s2) & 3) != 0)
   {
      char c1 = *(s1++);
      char c2 = *(s2++);

      if (c1 != c2)
         return c1 < c2 ? -1 : +1;
      else if (!c1)
         return 0;
   }

   while (1)
   {
      uint32_t v1 = *(uint32_t*)s1;
      uint32_t v2 = *(uint32_t*)s2;

      if (__builtin_expect(v1 != v2, 0))
      {
         char c1, c2;

         c1 = v1 & 0xff, c2 = v2 & 0xff;
         if (c1 != c2) return c1 < c2 ? -1 : +1;
         if (!c1) return 0;
         v1 = v1 >> 8, v2 = v2 >> 8;

         c1 = v1 & 0xff, c2 = v2 & 0xff;
         if (c1 != c2) return c1 < c2 ? -1 : +1;
         if (!c1) return 0;
         v1 = v1 >> 8, v2 = v2 >> 8;

         c1 = v1 & 0xff, c2 = v2 & 0xff;
         if (c1 != c2) return c1 < c2 ? -1 : +1;
         if (!c1) return 0;
         v1 = v1 >> 8, v2 = v2 >> 8;

         c1 = v1 & 0xff, c2 = v2 & 0xff;
         if (c1 != c2) return c1 < c2 ? -1 : +1;
         return 0;
      }

      if (__builtin_expect((((v1) - 0x01010101UL) & ~(v1) & 0x80808080UL), 0))
         return 0;

      s1 += 4;
      s2 += 4;
   }
}

int strncmp(const char *s1, const char *s2, int maxlen)
{
   while (maxlen)
   {
      char c1 = *(s1++);
      char c2 = *(s2++);

      if (c1 != c2)
         return c1 < c2 ? -1 : +1;
      else if (!c1)
         return 0;
      maxlen--;
   }
   return 0;
}

char *strstr(const char *searchee,const char *lookfor)
{
   /* Less code size, but quadratic performance in the worst case.  */
   if(*searchee == 0) {
      if(*lookfor)
         return (char *)0;
      return (char *)searchee;
   }

   while(*searchee) {
      size_t i;
      i = 0;

      while(1) {
         if(lookfor[i] == 0) {
            return (char *)searchee;
         }

         if(lookfor[i] != searchee[i]) {
            break;
         }
         i++;
      }
      searchee++;
   }

   return (char *)0;
}

char *strncat(char *dest, const char *src, int maxlen) {
  int offset = strlen(dest);
  maxlen -= offset;
  if(maxlen > 0) {
    return strncpy(&dest[offset], src, maxlen);
  }
  return dest;
}

char *strchr(char *haystack, const char needle) {
  int o = 0;
  while(haystack[o] > 0) {
    if(haystack[o] == needle)
      return &haystack[o];
    o++;
  }
  return (char *)0;
}

char *strrchr(char *haystack, const char needle) {
  int o = strlen(haystack);
  while(o > 0) {
    if(haystack[o] == needle)
      return &haystack[o];
    o--;
  }
  return (char *)0;
}
