/** @file src/gui/font.c Font routines. */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "types.h"

#include "font.h"
#include "../os/endian.h"

#include "../config.h"
#include "../file.h"
#include "../string.h"

Font *IntroFontPtr = NULL;
Font *FontNew6Ptr = NULL;
Font *FontNew8Ptr = NULL;

int8 g_fontCharOffset = -1;

Font *g_fontCurrent = NULL;

/**
 * Get the width of a char in pixels.
 *
 * @param c The char to get the width of.
 * @return The width of the char in pixels.
 */
uint16 Char_Pixel_Width(unsigned char c)
{
	return g_fontCurrent->chars[c].width + g_fontCharOffset;
}

/**
 * Get the width of the string in pixels.
 *
 * @param string The string to get the width of.
 * @return The width of the string in pixels.
 */
uint16 String_Pixel_Width(const char *string)
{
	uint16 width = 0;

	if (string == NULL) return 0;

	while (*string != '\0') {
		width += Char_Pixel_Width(*string++);
	}

	return width;
}

/**
 * Load a font file.
 *
 * @param filename The name of the font file.
 * @return The pointer of the allocated memory where the file has been read.
 */
static Font *Load_Font(const char *filename)
{
	uint8 *buf;
	Font *f;
	uint8 i;
	uint16 start;
	uint16 dataStart;
	uint16 widthList;
	uint16 lineList;

	if (!File_Exists(filename)) return NULL;

	buf = (uint8 *)Read_FileWholeFile(filename);

	if (buf[2] != 0x00 || buf[3] != 0x05) {
		free(buf);
		return NULL;
	}

	f = (Font *)calloc(1, sizeof(Font));
	start = READ_LE_UINT16(buf + 4);
	dataStart = READ_LE_UINT16(buf + 6);
	widthList = READ_LE_UINT16(buf + 8);
	lineList = READ_LE_UINT16(buf + 12);
	f->height = buf[start + 4];
	f->maxWidth = buf[start + 5];
	f->count = READ_LE_UINT16(buf + 10) - widthList;
	f->chars = (FontChar *)calloc(f->count, sizeof(FontChar));

	for (i = 0; i < f->count; i++) {
		FontChar *fc = &f->chars[i];
		uint16 dataOffset;
		uint8 x;
		uint8 y;

		fc->width = buf[widthList + i];
		fc->unusedLines = buf[lineList + i * 2];
		fc->usedLines = buf[lineList + i * 2 + 1];

		dataOffset = READ_LE_UINT16(buf + dataStart + i * 2);
		if (dataOffset == 0) continue;

		fc->data = (uint8 *)malloc(fc->usedLines * fc->width);

		for (y = 0; y < fc->usedLines; y++) {
			for (x = 0; x < fc->width; x++) {
				uint8 data = buf[dataOffset + y * ((fc->width + 1) / 2) + x / 2];
				if (x % 2 != 0) data >>= 4;
				fc->data[y * fc->width + x] = data & 0xF;
			}
		}
	}

	free(buf);

	return f;
}

/**
 * Select a font.
 *
 * @param font The pointer of the font to use.
 */
void Font_Select(Font *f)
{
	if (f == NULL) return;

	g_fontCurrent = f;
}

bool Init_Fonts(void)
{
	IntroFontPtr = Load_Font("INTRO.FNT");
	if ((g_config.Language == LANGUAGE_GERMAN) && File_Exists("new6pg.fnt")) {
		FontNew6Ptr = Load_Font("new6pg.fnt");
	} else {
		FontNew6Ptr = Load_Font("new6p.fnt");
	}
	FontNew8Ptr = Load_Font("new8p.fnt");

	return FontNew8Ptr != NULL;
}

static void Font_Unload(Font *f) {
	uint8 i;

	for (i = 0; i < f->count; i++) free(f->chars[i].data);
	free(f->chars);
	free(f);
}

void Font_Uninit(void)
{
	Font_Unload(IntroFontPtr); IntroFontPtr = NULL;
	Font_Unload(FontNew6Ptr); FontNew6Ptr = NULL;
	Font_Unload(FontNew8Ptr); FontNew8Ptr = NULL;
}
