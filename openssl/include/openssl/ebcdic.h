/*
libclasspack 1.0 - (c) Roel Verdult 2008

If you use libclasspack you agree to use it only non-commercially.
Non-commercial means you are not getting compensated in any form
for the products and services you develop using this library.

There is no warranty for this software. If you use libclasspack
you agree to take complete responsibility. This means no claims 
can be filed to the original author, Roel Verdult.

Using parts from libclasspack also means you are using libclasspack.
Academic use is allowed as long as it is part of an academic research
and no product or service.

For commercial license, please contact roel[at]vfwebdesign[dot]nl
*/

/* crypto/ebcdic.h */

#ifndef HEADER_EBCDIC_H
#define HEADER_EBCDIC_H

#include <sys/types.h>

/* Avoid name clashes with other applications */
#define os_toascii   _openssl_os_toascii
#define os_toebcdic  _openssl_os_toebcdic
#define ebcdic2ascii _openssl_ebcdic2ascii
#define ascii2ebcdic _openssl_ascii2ebcdic

extern const unsigned char os_toascii[256];
extern const unsigned char os_toebcdic[256];
void *ebcdic2ascii(void *dest, const void *srce, size_t count);
void *ascii2ebcdic(void *dest, const void *srce, size_t count);

#endif
