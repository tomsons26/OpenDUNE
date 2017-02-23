/** @file src/gfx.h Graphics definitions. */

#ifndef GFX_H
#define GFX_H

extern uint8 g_paletteActive[256 * 3];
extern uint8 *g_palette1;
extern uint8 *g_palette2;
extern uint8 *g_paletteMapping1;
extern uint8 *g_paletteMapping2;

enum {
	SCREEN_WIDTH  = 320, /*!< Width of the screen in pixels. */
	SCREEN_HEIGHT = 200  /*!< Height of the screen in pixels. */
};

/*Maybe
[586] GraphicModeType      typedef enum
[5be] CGAMODE                Value = 0h
[5bf] TGAMODE                Value = 1h
[5c0] EGAMODE                Value = 2h
[5c1] MCGAMODE               Value = 3h
[5c2] VGAMODE                Value = 4h
[5c3] EEGAMODE               Value = 5h
[5c4] ETGAMODE               Value = 6h
[5c5] HGAMODE                Value = 7h
[5c6] TXTMODE                Value = 8h
[5c7] RESETMODE              Value = 9h
[5c8] LASTGRAPHICMODE        Value = Ah

or

[437] PageType             typedef enum
[43b] SEENPAGE               Value = 0h
[43c] HIDPAGE                Value = 2h
[43d] BACKPAGE               Value = 4h
[43e] AUX1PAGE               Value = 6h
[43f] AUX2PAGE               Value = 8h
[440] AUX3PAGE               Value = Ah
[441] AUX4PAGE               Value = Ch
[442] AUX5PAGE               Value = Eh
[443] AUX6PAGE               Value = 10h
[444] AUX7PAGE               Value = 12h
[445] AUX8PAGE               Value = 14h
[446] AUX9PAGE               Value = 16h
[447] AUX10PAGE              Value = 18h
[448] AUX11PAGE              Value = 1Ah
[449] AUX12PAGE              Value = 1Ch
[44a] AUX13PAGE              Value = 1Eh
[44b] ILLEGALPAGE            Value = 20h

?
*/
typedef enum Screen {
	SCREEN_0 = 0,
	SCREEN_1 = 1,
	SCREEN_2 = 2,
	SCREEN_3 = 3,
	SCREEN_ACTIVE = -1
} Screen;

extern void GFX_Init(void);
extern void GFX_Uninit(void);
extern Screen GFX_Screen_SetActive(Screen screenID);
extern bool GFX_Screen_IsActive(Screen screenID);
extern void *GFX_Screen_GetActive(void);
extern uint16 Get_Buff(Screen screenID);
extern void *Get_Page(Screen screenID);

extern void GFX_DrawSprite(uint16 spriteID, uint16 x, uint16 y, uint8 houseID);
extern void GFX_Init_SpriteInfo(uint16 widthSize, uint16 heightSize);
extern void GFX_PutPixel(uint16 x, uint16 y, uint8 colour);
extern void GFX_Screen_Copy2(int16 xSrc, int16 ySrc, int16 xDst, int16 yDst, int16 width, int16 height, Screen screenSrc, Screen screenDst, bool skipNull);
extern void GFX_Screen_Copy(int16 xSrc, int16 ySrc, int16 xDst, int16 yDst, int16 width, int16 height, Screen screenSrc, Screen screenDst);
extern void GFX_ClearScreen(Screen screenID);
extern void GFX_ClearBlock(Screen index);
extern void Set_Palette(uint8 *palette);
extern uint8 GFX_GetPixel(uint16 x, uint16 y);
extern uint16 GFX_GetSize(int16 width, int16 height);
extern void GFX_CopyFromBuffer(int16 left, int16 top, uint16 width, uint16 height, uint8 *buffer);
extern void GFX_CopyToBuffer(int16 left, int16 top, uint16 width, uint16 height, uint8 *buffer);

#endif /* GFX_H */
