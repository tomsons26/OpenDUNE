/** @file src/gui/font.h Font definitions. */

#ifndef GUI_FONT_H
#define GUI_FONT_H

typedef struct FontChar {
	uint8 width;
	uint8 unusedLines;
	uint8 usedLines;
	uint8 *data;
} FontChar;

typedef struct Font {
	uint8 height;
	uint8 maxWidth;
	uint8 count;
	FontChar *chars;
} Font;

extern Font *g_fontIntro;
extern Font *FontNew6Ptr;
extern Font *FontNew8Ptr;

extern int8 g_fontCharOffset;

extern Font *FontPtr;

extern bool Init_Fonts(void);
extern void Font_Uninit(void);
extern uint16 Char_Pixel_Width(unsigned char c);
extern uint16 String_Pixel_Width(const char *string);
extern void Set_Font(Font *f);

#endif /* GUI_FONT_H */
