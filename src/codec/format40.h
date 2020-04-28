/** @file src/codec/format40.h Function signature for decoder of 'format40' files. */

#ifndef CODEC_FORMAT40_H
#define CODEC_FORMAT40_H

extern void Apply_XOR_Delta(uint8 *dst, uint8 *src);
extern void XOR_Delta_Buffer(uint8 *dst, uint8 *src, uint16 width);
extern void Copy_Delta_Buffer(uint8 *dst, uint8 *src, uint16 width);

#endif /* CODEC_FORMAT40_H */
