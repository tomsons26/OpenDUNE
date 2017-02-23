/** @file src/codec/format40.h Function signature for decoder of 'format40' files. */

#ifndef CODEC_FORMAT40_H
#define CODEC_FORMAT40_H



/*
Format40 is XORDelta
Not all too sure which function is which here

Known function names
_Apply_XOR_Delta             .text 00049528 0000007F 00000010 00000008 R . . . B . .
Copy_Delta_Buffer            .text 000495A7 000000DB 00000004 00000000 R . . . . T .
XOR_Delta_Buffer             .text 00049682 000000DC 00000004 00000000 R . . . . . .
Copy_Delta_Buffer_No_Recheck .text 0004975E 00000124 00000004 00000000 R . . . . . .
XOR_Delta_Buffer_No_Recheck  .text 00049882 00000124 00000004 00000000 R . . . . . .
_Apply_XOR_Delta_On_Page     .text 000499A8 0000004E 00000010 00000014 R . . . B . .
Apply_XOR_Delta_To_Page_Or_Viewport

//in Dune 2 exe
_Apply_XOR_Delta         LP_ASM.ASM 000000B1 0000007A 00000002 00000008 R F . . B . .
XOR_Delta_Buffer         LP_ASM.ASM 0000012B 000000B9 00000002 00000000 R F . . B . .
_Apply_XOR_Delta_On_Page LP_ASM.ASM 00000299 00000037 00000002 0000000C R F . . B . .
*/

extern void Apply_XOR_Delta(uint8 *dst, uint8 *src);
extern void Format40_Decode_XorToScreen(uint8 *dst, uint8 *src, uint16 width);
extern void Format40_Decode_ToScreen(uint8 *dst, uint8 *src, uint16 width);

#endif /* CODEC_FORMAT40_H */
