#ifndef STDARG_H
#define STDARG_H

#define va_start(ap, last) ap = ((char *)&(last)) + ((sizeof(last)+3)&~3)
#define va_arg(ap, type) (ap += (sizeof(type)+3)&~3, *(type *)(ap - ((sizeof(type)+3)&~3)))
#define va_copy(dest, src) (dest) = (src)
#define va_end(ap)
typedef char *va_list;

#endif // STDARG_H
