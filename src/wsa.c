/** @file src/wsa.c WSA routines. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "os/math.h"
#include "os/endian.h"
#include "gfx.h"

#include "wsa.h"

#include "codec/format40.h"
#include "codec/format80.h"
#include "file.h"
#include "gui/widget.h"


/**
 * The flags of a WSA Header.
 */

/*
[767] WSAOpenType          typedef enum
[769] WSA_OPEN_FROM_MEM      Value = 0h
[76a] WSA_OPEN_TO_BUFFER     Value = 0h
[76b] WSA_OPEN_FROM_DISK     Value = 1h
[76c] WSA_OPEN_TO_PAGE       Value = 2h

[768] WSAType              typedef enum
[776] WSA_NORMAL             Value = 0h
[777] WSA_GHOST              Value = 1000h
[778] WSA_PRIORITY2          Value = 2000h
[779] WSA_TRANS              Value = 4000h
[77a] WSA_PRIORITY           Value = 8000h
*/
typedef struct WSAFlags {
	BIT_U8 notmalloced:1;                                   /*!< If the WSA is in memory of the caller. */
	BIT_U8 malloced:1;                                      /*!< If the WSA is malloc'd by us. */
	BIT_U8 dataOnDisk:1;                                    /*!< Only the header is in the memory. Rest is on disk. */
	BIT_U8 dataInMemory:1;                                  /*!< The whole WSA is in memory. */
	BIT_U8 displayInBuffer:1;                               /*!< The output display is in the buffer. */
	BIT_U8 noAnimation:1;                                   /*!< If the WSA has animation or not. */
	BIT_U8 hasNoAnimation:1;                                /*!< The WSA has no animation. */
	BIT_U8 isSpecial:1;                                     /*!< Indicates if the WSA has a special buffer. */
}  WSAFlags;

/**
 * The header of a WSA file that is being read.
 */
typedef struct SysAnimHeaderType {
	uint16 frameCurrent;                                    /*!< Current frame displaying. */
	uint16 frames;                                          /*!< Total frames in WSA. */
	uint16 width;                                           /*!< Width of WSA. */
	uint16 height;                                          /*!< Height of WSA. */
	uint16 bufferLength;                                    /*!< Length of the buffer. */
	uint8 *buffer;                                          /*!< The buffer. */
	uint8 *fileContent;                                     /*!< The content of the file. */
	char   filename[13];                                    /*!< Filename of WSA. */
	WSAFlags flags;                                         /*!< Flags of WSA. */
} SysAnimHeaderType;

MSVC_PACKED_BEGIN
/**
 * The header of a WSA file as on the disk.
 */
typedef struct WSA_FileHeaderType {
	/* 0000(2)   */ uint16 frames;                     /*!< Amount of animation frames in this WSA. */
	/* 0002(2)   */ uint16 width;                      /*!< Width of WSA. */
	/* 0004(2)   */ uint16 height;                     /*!< Height of WSA. */
	/* 0006(2)   */ uint16 requiredBufferSize;         /*!< The size the buffer has to be at least to process this WSA. */
	/* 0008(2)   */ uint16 flags;                      /*!< Indicates if the WSA has a special buffer. */
	/* 000A(4)   */ uint32 animationOffsetStart;       /*!< Offset where animation starts. */
	/* 000E(4)   */ uint32 animationOffsetEnd;         /*!< Offset where animation ends. */
} WSA_FileHeaderType;

/**
 * Get the amount of frames a WSA has.
 */
uint16 Animate_Frame_Count(void *handle)
{
	SysAnimHeaderType *sys_header = (SysAnimHeaderType *)handle;

	if (sys_header == NULL) return 0;
	return sys_header->frames;
}

/**
 * Get the offset in the fileContent which stores the animation data for a
 *  given frame.
 * @param header The header of the WSA.
 * @param frame The frame of animation.
 * @return The offset for the animation from the beginning of the fileContent.
 */
static uint32 Get_Resident_Frame_Offset(SysAnimHeaderType *header, uint16 frame)
{
	uint16 lengthAnimation = 0;
	uint32 animationFrame;
	uint32 animation0;

	animationFrame = READ_LE_UINT32(header->fileContent + frame * 4);

	if (animationFrame == 0) return 0;

	animation0 = READ_LE_UINT32(header->fileContent);
	if (animation0 != 0) {
		lengthAnimation = READ_LE_UINT32(header->fileContent + 4) - animation0;
	}

	return animationFrame - lengthAnimation - 10;
}

/**
 * Get the offset in the file which stores the animation data for a given
 *  frame.
 * @param fileno The fileno of an opened WSA.
 * @param frame The frame of animation.
 * @return The offset for the animation from the beginning of the file.
 */
static uint32 Get_File_Frame_Offset(uint8 fileno, uint16 frame)
{
	uint32 offset;

	Seek_File(fileno, frame * 4 + 10, 0);
	offset = Read_File_LE32(fileno);

	return offset;
}

/**
 * Go to the next frame in the animation.
 * @param wsa WSA pointer.
 * @param frame Frame number to go to.
 * @param dst Destination buffer to write the animation to.
 * @return 1 on success, 0 on failure.
 */
static uint16 Apply_Delta(void *wsa, uint16 frame, uint8 *dst)
{
	SysAnimHeaderType *header = (SysAnimHeaderType *)wsa;
	uint16 lengthSpecial;
	uint8 *buffer;

	lengthSpecial = 0;
	if (header->flags.isSpecial) lengthSpecial = 0x300;

	buffer = header->buffer;

	if (header->flags.dataInMemory) {
		uint32 positionStart;
		uint32 positionEnd;
		uint32 length;
		uint8 *positionFrame;

		positionStart = Get_Resident_Frame_Offset(header, frame);
		positionEnd = Get_Resident_Frame_Offset(header, frame + 1);
		length = positionEnd - positionStart;

		positionFrame = header->fileContent + positionStart;
		buffer += header->bufferLength - length;

		memmove(buffer, positionFrame, length);
	} else if (header->flags.dataOnDisk) {
		uint8 fileno;
		uint32 positionStart;
		uint32 positionEnd;
		uint32 length;
		uint32 res;

		fileno = File_Open(header->filename, FILE_MODE_READ);

		positionStart = Get_File_Frame_Offset(fileno, frame);
		positionEnd = Get_File_Frame_Offset(fileno, frame + 1);
		length = positionEnd - positionStart;

		if (positionStart == 0 || positionEnd == 0 || length == 0) {
			Close_File(fileno);
			return 0;
		}

		buffer += header->bufferLength - length;

		Seek_File(fileno, positionStart + lengthSpecial, 0);
		res = Read_File(fileno, buffer, length);
		Close_File(fileno);

		if (res != length) return 0;
	}

	LCW_Uncomp(header->buffer, buffer, header->bufferLength);

	if (header->flags.displayInBuffer) {
		Apply_XOR_Delta(dst, header->buffer);
	} else {
		XOR_Delta_Buffer(dst, header->buffer, header->width);
	}

	return 1;
}

/**
 * Load a WSA file.
 * @param filename Name of the file.
 * @param wsa Data buffer for the WSA.
 * @param wsaSize Current size of buffer.
 * @param reserveDisplayFrame True if we need to reserve the display frame.
 * @return Address of loaded WSA file, or NULL.
 */
void *Open_Animation(const char *filename, void *wsa, uint32 wsaSize, bool reserveDisplayFrame)
{
	WSAFlags flags;
	WSA_FileHeaderType fileheader;
	SysAnimHeaderType *header;
	uint32 bufferSizeMinimal;
	uint32 bufferSizeOptimal;
	uint16 lengthHeader;
	uint8 fileno;
	uint16 lengthSpecial;
	uint16 lengthAnimation;
	uint32 lengthFileContent;
	uint32 displaySize;
	uint8 *buffer;

	memset(&flags, 0, sizeof(flags));

	fileno = File_Open(filename, FILE_MODE_READ);
	fileheader.frames = Read_File_LE16(fileno);
	fileheader.width = Read_File_LE16(fileno);
	fileheader.height = Read_File_LE16(fileno);
	fileheader.requiredBufferSize = Read_File_LE16(fileno);
	fileheader.flags = Read_File_LE16(fileno);
	fileheader.animationOffsetStart = Read_File_LE32(fileno);
	fileheader.animationOffsetEnd = Read_File_LE32(fileno);

	lengthSpecial = 0;
	if (fileheader.flags) {
		flags.isSpecial = true;

		lengthSpecial = 0x300;
	}

	lengthFileContent = Seek_File(fileno, 0, 2);

	lengthAnimation = 0;
	if (fileheader.animationOffsetStart != 0) {
		lengthAnimation = fileheader.animationOffsetEnd - fileheader.animationOffsetStart;
	} else {
		flags.hasNoAnimation = true;
	}

	lengthFileContent -= lengthSpecial + lengthAnimation + 10;

	displaySize = 0;
	if (reserveDisplayFrame) {
		flags.displayInBuffer = true;
		displaySize = fileheader.width * fileheader.height;
	}

	bufferSizeMinimal = displaySize + fileheader.requiredBufferSize - 33 + sizeof(SysAnimHeaderType);
	bufferSizeOptimal = bufferSizeMinimal + lengthFileContent;

	if (wsaSize > 1 && wsaSize < bufferSizeMinimal) {
		Close_File(fileno);

		return NULL;
	}
	if (wsaSize == 0) wsaSize = bufferSizeOptimal;
	if (wsaSize == 1) wsaSize = bufferSizeMinimal;

	if (wsa == NULL) {
		if (wsaSize == 0) {
			wsaSize = bufferSizeOptimal;
		} else if (wsaSize == 1) {
			wsaSize = bufferSizeMinimal;
		} else if (wsaSize >= bufferSizeOptimal) {
			wsaSize = bufferSizeOptimal;
		} else {
			wsaSize = bufferSizeMinimal;
		}

		wsa = calloc(1, wsaSize);
		flags.malloced = true;
	} else {
		flags.notmalloced = true;
	}

	header = (SysAnimHeaderType *)wsa;
	buffer = (uint8 *)wsa + sizeof(SysAnimHeaderType);

	header->flags = flags;

	if (reserveDisplayFrame) {
		memset(buffer, 0, displaySize);
	}

	buffer += displaySize;

	if ((fileheader.frames & 0x8000) != 0) {
		fileheader.frames &= 0x7FFF;
	}

	header->frameCurrent = fileheader.frames;
	header->frames       = fileheader.frames;
	header->width        = fileheader.width;
	header->height       = fileheader.height;
	header->bufferLength = fileheader.requiredBufferSize + 33 - sizeof(SysAnimHeaderType);
	header->buffer       = buffer;
	strncpy(header->filename, filename, sizeof(header->filename));

	lengthHeader = (fileheader.frames + 2) * 4;

	if (wsaSize >= bufferSizeOptimal) {
		header->fileContent = buffer + header->bufferLength;

		Seek_File(fileno, 10, 0);
		Read_File(fileno, header->fileContent, lengthHeader);
		Seek_File(fileno, lengthAnimation + lengthSpecial, 1);
		Read_File(fileno, header->fileContent + lengthHeader, lengthFileContent - lengthHeader);

		header->flags.dataInMemory = true;
		if (Get_Resident_Frame_Offset(header, header->frames + 1) == 0) header->flags.noAnimation = true;
	} else {
		header->flags.dataOnDisk = true;
		if (Get_File_Frame_Offset(fileno, header->frames + 1) == 0) header->flags.noAnimation = true;
	}

	{
		uint8 *b;
		b = buffer + header->bufferLength - lengthAnimation;

		Seek_File(fileno, lengthHeader + lengthSpecial + 10, 0);
		Read_File(fileno, b, lengthAnimation);
		Close_File(fileno);

		LCW_Uncomp(buffer, b, header->bufferLength);
	}
	return wsa;
}

/**
 * Unload the WSA.
 * @param wsa The pointer to the WSA.
 */
void Close_Animation(void *wsa)
{
	SysAnimHeaderType *header = (SysAnimHeaderType *)wsa;

	if (wsa == NULL) return;
	if (!header->flags.malloced) return;

	free(wsa);
}

/**
 * Draw a frame on the buffer.
 * @param x The X-position to start drawing.
 * @param y The Y-position to start drawing.
 * @param width The width of the image.
 * @param height The height of the image.
 * @param windowID The windowID.
 * @param screenID the screen to write to
 * @param src The source for the frame.
 */

//This was in BUFF.ASM, nothing to do with WSA but that it's only called by WSA code by Dune 2
//Later version is called Buffer_To_Page which in TD Dos case just calls MCGA_Buffer_To_Page
static void Buffer_Bitblit_To_LogicPage(int16 x, int16 y, int16 width, int16 height, uint16 windowID, uint8 *src, Screen screenID)
{
	int16 left;
	int16 right;
	int16 top;
	int16 bottom;
	int16 skipBefore;
	int16 skipAfter;
	uint8 *dst;

	dst = Get_Page(screenID);

	left   = g_widgetProperties[windowID].xBase << 3;
	right  = left + (g_widgetProperties[windowID].width << 3);
	top    = g_widgetProperties[windowID].yBase;
	bottom = top + g_widgetProperties[windowID].height;

	if (y - top < 0) {
		if (y - top + height <= 0) return;
		height += y - top;
		src += (top - y) * width;
		y += top - y;
	}

	if (bottom - y <= 0) return;
	height = min(bottom - y, height);

	skipBefore = 0;
	if (x - left < 0) {
		skipBefore = left - x;
		x += skipBefore;
		width -= skipBefore;
	}

	skipAfter = 0;
	if (right - x <= 0) return;
	if (right - x < width) {
		skipAfter = width - right + x;
		width = right - x;
	}

	dst += y * SCREEN_WIDTH + x;

	while (height-- != 0) {
		src += skipBefore;
		memcpy(dst, src, width);
		src += width + skipAfter;
		dst += SCREEN_WIDTH;
	}
}

/**
 * Display a frame.
 * @param wsa The pointer to the WSA.
 * @param frameNext The next frame to display.
 * @param posX The X-position of the WSA.
 * @param posY The Y-position of the WSA.
 * @param screenID The screenID to draw on.
 * @return False on failure, true on success.
 */
bool Animate_Frame(void *wsa, uint16 frameNext, uint16 posX, uint16 posY, Screen screenID)
{
	SysAnimHeaderType *header = (SysAnimHeaderType *)wsa;
	uint8 *dst;

	int16 frameDiff;
	int16 direction;
	int16 frameCount;

	if (wsa == NULL) return false;
	if (frameNext >= header->frames) return false;

	if (header->flags.displayInBuffer) {
		dst = (uint8 *)wsa + sizeof(SysAnimHeaderType);
	} else {
		dst = Get_Page(screenID);
		dst += posX + posY * SCREEN_WIDTH;
	}

	if (header->frameCurrent == header->frames) {
		if (!header->flags.hasNoAnimation) {
			if (!header->flags.displayInBuffer) {
				Copy_Delta_Buffer(dst, header->buffer, header->width);
			} else {
				Apply_XOR_Delta(dst, header->buffer);
			}
		}

		header->frameCurrent = 0;
	}

	frameDiff = abs(header->frameCurrent - frameNext);
	direction = 1;

	if (frameNext > header->frameCurrent) {
		frameCount = header->frames - frameNext + header->frameCurrent;

		if (frameCount < frameDiff && !header->flags.noAnimation) {
			direction = -1;
		} else {
			frameCount = frameDiff;
		}
	} else {
		frameCount = header->frames - header->frameCurrent + frameNext;

		if (frameCount < frameDiff && !header->flags.noAnimation) {
		} else {
			direction = -1;
			frameCount = frameDiff;
		}
	}

	if (direction > 0) {
		uint16 i;
		uint16 frame = header->frameCurrent;

		for (i = 0; i < frameCount; i++) {
			frame += direction;

			Apply_Delta(wsa, frame, dst);

			if (frame == header->frames) frame = 0;
		}
	} else {
		uint16 i;
		uint16 frame = header->frameCurrent;

		for (i = 0; i < frameCount; i++) {
			if (frame == 0) frame = header->frames;

			Apply_Delta(wsa, frame, dst);

			frame += direction;
		}
	}

	header->frameCurrent = frameNext;

	if (header->flags.displayInBuffer) {
		Buffer_Bitblit_To_LogicPage(posX, posY, header->width, header->height, 0, dst, screenID);
	}

	return true;
}
