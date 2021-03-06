/** @file src/gui/gui.h Generic GUI definitions. */

#ifndef GUI_GUI_H
#define GUI_GUI_H

#include "../gfx.h"

/**
 * Factory results.
 */
typedef enum FactoryResult {
	FACTORY_RESUME       = 0,
	FACTORY_BUY          = 1,
	FACTORY_UPGRADE      = 2,
	FACTORY_CONTINUE     = 0xFFFF
} FactoryResult;

/**
 * The possible selection types.
 */
typedef enum SelectionType {
	SELECTIONTYPE_MENTAT    = 0,                            /*!< Used in most mentat screens. */
	SELECTIONTYPE_TARGET    = 1,                            /*!< Used when attacking or moving a unit, the target screen. */
	SELECTIONTYPE_PLACE     = 2,                            /*!< Used when placing a structure. */
	SELECTIONTYPE_UNIT      = 3,                            /*!< Used when selecting a Unit. */
	SELECTIONTYPE_STRUCTURE = 4,                            /*!< Used when selecting a Structure or nothing. */
	SELECTIONTYPE_DEBUG     = 5,                            /*!< Used when debugging scenario. */
	SELECTIONTYPE_UNKNOWN6  = 6,                            /*!< ?? */
	SELECTIONTYPE_INTRO     = 7,                            /*!< Used in intro of the game. */

	SELECTIONTYPE_MAX       = 8
} SelectionType;

/**
 * Hall Of Fame data struct.
 */
typedef struct HallOfFameStruct {
	char name[6];                                           /*!< Name of the entry. */
	uint16 score;                                           /*!< Score of the entry. */
	uint16 rank;                                            /*!< Rank of the entry. */
	uint16 campaignID;                                      /*!< Which campaign was reached. */
	uint8  houseID;                                         /*!< Which house was playing. */
	uint8  padding1;                                        /*!< Padding bytes. */
	uint16 padding2;                                        /*!< Padding bytes. */
} HallOfFameStruct;
assert_compile(sizeof(HallOfFameStruct) == 16);

/**
 * Factory Window Item struct.
 */
typedef struct FactoryWindowItem {
	uint16 objectType;                                      /*!< Which object is this item. */
	int8   amount;                                          /*!< How many are available. */
	uint16 credits;                                         /*!< What is the current price. */
	uint16 sortPriority;                                    /*!< The sorting priority. */
	struct ObjectInfo *objectInfo;                          /*!< The ObjectInfo of the item. */
} FactoryWindowItem;

/**
 * Information for the selection type.
 */
typedef struct SelectionTypeStruct {
	 int8  visibleWidgets[20];                              /*!< List of index of visible widgets, -1 terminated. */
	bool   variable_04;                                     /*!< ?? */
	bool   variable_06;                                     /*!< ?? */
	uint16 defaultWidget;                                   /*!< Index of the default Widget. */
} SelectionTypeStruct;

struct Widget;

extern const SelectionTypeStruct g_table_selectionType[SELECTIONTYPE_MAX];

extern uint8 *Palette;
extern uint8 g_remap[256];
extern FactoryWindowItem g_factoryWindowItems[25];
extern uint16 g_factoryWindowOrdered;
extern uint16 g_factoryWindowBase;
extern uint16 g_factoryWindowTotal;
extern uint16 g_factoryWindowSelected;
extern uint16 g_factoryWindowUpgradeCost;
extern bool g_factoryWindowConstructionYard;
extern FactoryResult g_factoryWindowResult;
extern bool g_factoryWindowStarport;
extern uint16 g_productionStringID;
extern bool g_textDisplayNeedsUpdate;
extern uint32 g_strategicRegionBits;

extern uint16 g_viewportMessageCounter;
extern const char *g_viewportMessageText;
extern uint16 g_viewportPosition;
extern uint16 g_minimapPosition;
extern uint16 g_selectionRectanglePosition;
extern uint16 g_selectionPosition;
extern uint16 g_selectionWidth;
extern uint16 g_selectionHeight;
extern int16  g_selectionState;
extern bool g_doQuitHOF;

extern uint16 g_mouseSpriteHotspotX;
extern uint16 g_mouseSpriteHotspotY;
extern uint16 g_mouseWidth;
extern uint16 g_mouseHeight;

extern uint16 g_cursorSpriteID;
extern uint16 g_cursorDefaultSpriteID;

extern bool g_structureHighHealth;
extern bool g_var_37B8;

#define GUI_ClearScreen(screenID) Clear_Screen(screenID)
extern void GUI_DrawScreen(Screen screenID);
extern void Draw_Shape(Screen screenID, const uint8 *sprite, int16 posX, int16 posY, uint16 windowID, uint16 flags, ...);
extern void GUI_DrawInterfaceAndRadar(Screen screenID);
extern void GUI_Palette_RemapScreen(uint16 left, uint16 top, uint16 width, uint16 height, Screen screenID, const uint8 *remap);
extern void GUI_Screen_Copy(int16 xSrc, int16 ySrc, int16 xDst, int16 yDst, int16 width, int16 height, Screen screenSrc, Screen screenDst);
extern void GUI_Screen_FadeIn(uint16 xSrc, uint16 ySrc, uint16 xDst, uint16 yDst, uint16 width, uint16 height, Screen screenSrc, Screen screenDst);
extern void Bit_It_In(int16 x, int16 y, int16 width, int16 height, Screen screenSrc, Screen screenDst, uint16 delay, bool skipNull);

extern void GUI_DrawBlockedRectangle(int16 left, int16 top, int16 width, int16 height, uint8 colour);
extern void _Hilite_Box(uint16 left, uint16 top, uint16 width, uint16 height, uint16 colourSchemaIndex, bool fill);
extern void GUI_DrawCredits(uint8 houseID, uint16 mode);
extern void _Fill_Rect(int16 left, int16 top, int16 right, int16 bottom, uint8 colour);
extern void _Draw_Line(int16 x1, int16 y1, int16 x2, int16 y2, uint8 color);
extern void GUI_DrawProgressbar(uint16 current, uint16 max);
extern void Text_Print_Monospace(char *string, uint16 left, uint16 top, uint8 fgColour, uint8 bgColour, uint16 charWidth);
extern void _Draw_Rect(uint16 left, uint16 top, uint16 right, uint16 bottom, uint8 colour);
extern void GUI_DrawXorFilledRectangle(int16 left, int16 top, int16 right, int16 bottom, uint8 colour);

extern void GUI_Palette_CreateMapping(const uint8 *palette, uint8 *colors, uint8 reference, uint8 intensity);
extern void GUI_Palette_CreateRemap(uint8 houseID);
extern void _Fade_Palette_To(uint8 *palette, int16 ticksOfAnimation);
extern void Color_Cycle(void);

extern void GUI_DisplayText(const char *str, int16 importance, ...);
extern void Text_Print(const char *string, int16 left, int16 top, uint8 fgColour, uint8 bgColour);
extern void Fancy_Text_Print(const char *string, int16 left, int16 top, uint8 fgColour, uint8 bgColour, uint16 flags, ...);
extern uint16 GUI_DisplayModalMessage(const char *str, uint16 stringID, ...);
extern uint16 GUI_DisplayHint(uint16 stringID, uint16 spriteID);

extern void GUI_UpdateProductionStringID(void);
extern uint16 GUI_SplitText(char *str, uint16 maxwidth, char delimiter);
extern void GUI_EndStats_Show(uint16 killedAllied, uint16 killedEnemy, uint16 destroyedAllied, uint16 destroyedEnemy, uint16 harvestedAllied, uint16 harvestedEnemy, int16 score, uint8 houseID);
extern uint8 Choose_Side(void);
extern void GUI_ChangeSelectionType(uint16 selectionType);
extern void Set_Font_Palette_Range(const uint8 *colors, uint8 first, uint8 last);
extern void GUI_SetClippingArea(uint16 left, uint16 top, uint16 right, uint16 bottom);
extern FactoryResult GUI_DisplayFactoryWindow(bool isConstructionYard, bool isStarPort, uint16 upgradeCost);
extern char *GUI_Extract_String(int16 stringID);
extern uint16 Map_Selection(uint16 campaignID, bool win);
extern uint16 GUI_Get_Scrollbar_Position(struct Widget *w);
extern void GUI_FactoryWindow_B495_0F30(void);
extern struct FactoryWindowItem *GUI_FactoryWindow_GetItem(int16 offset);
extern void GUI_FactoryWindow_DrawDetails(void);
extern void GUI_FactoryWindow_DrawCaption(const char *caption);
extern void GUI_FactoryWindow_UpdateDetails(void);
extern void GUI_FactoryWindow_UpdateSelection(bool selectionChanged);
extern void GUI_FactoryWindow_PrepareScrollList(void);
extern void Low_Show_Mouse(void);
extern void Low_Hide_Mouse(void);
extern void Show_Mouse(void);
extern void Hide_Mouse(void);
extern void Conditional_Show_Mouse(void);
extern void Conditional_Hide_Mouse(uint16 left, uint16 top, uint16 right, uint16 bottom);
extern void Window_Show_Mouse(void);
extern void Window_Hide_Mouse(uint16 widgetIndex);
extern void _Set_Mouse_Pos(uint16 x, uint16 y);
extern uint16 GUI_HallOfFame_Tick(void);
extern void GUI_HallOfFame_Show(uint16 score);
extern uint16 GUI_HallOfFame_DrawData(HallOfFameStruct *data, bool show);

/* editbox.c */
extern uint16 GUI_EditBox(char *text, uint16 maxLength, uint16 widgetID, struct Widget *w, uint16 (*tickProc)(void), bool paint);

#endif /* GUI_GUI_H */
