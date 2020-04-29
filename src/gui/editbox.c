/** @file src/gui/editbox.c Editbox routines. */

#include <stdio.h>
#include "types.h"
#include "../os/sleep.h"

#include "font.h"
#include "gui.h"
#include "widget.h"
#include "../gfx.h"
#include "../input/input.h"
#include "../timer.h"

/**
 * Draw a blinking cursor, used inside the EditBox.
 *
 * @param positionX Where to draw the cursor on the X position.
 * @param resetBlink If true, the blinking is reset and restarted.
 */
static void GUI_EditBox_BlinkCursor(uint16 positionX, bool resetBlink)
{
	static uint32 tickEditBox = 0;           /* Ticker for cursor blinking. */
	static bool   editBoxShowCursor = false; /* Cursor is active. */

	if (resetBlink) {
		tickEditBox = 0;
		editBoxShowCursor = true;
	}

	if (tickEditBox > g_timerGUI) return;
	if (!resetBlink) {
		tickEditBox = g_timerGUI + 20;
	}

	editBoxShowCursor = !editBoxShowCursor;

	Hide_Mouse();
	Fill_Rect(positionX, WinY, positionX + Char_Pixel_Width('W'), WinY + WinH - 1, (editBoxShowCursor) ? g_curWidgetFGColourBlink : g_curWidgetFGColourNormal);
	Show_Mouse();
}

/**
 * Show an EditBox and handles the input.
 * @param text The text to edit. Uses the pointer to make the modifications.
 * @param maxLength The maximum length of the text.
 * @param widgetID the widget in which the EditBox is displayed.
 * @param w The widget this editbox is attached to (for input events).
 * @param tickProc The function to call every tick, for animation etc.
 * @param paint Flag indicating if the widget need to be repainted.
 * @return Key code / Button press code.
 */
uint16 GUI_EditBox(char *text, uint16 maxLength, uint16 widgetID, Widget *w, uint16 (*tickProc)(void), bool paint)
{
	Screen oldScreenID;
	uint16 oldWidgetID;
	uint16 positionX;
	uint16 maxWidth;
	uint16 textWidth;
	uint16 textLength;
	uint16 returnValue;
	char *t;

	/* Initialize */
	{
		Input_Flags_SetBits(INPUT_FLAG_NO_TRANSLATE);
		Input_Flags_ClearBits(INPUT_FLAG_UNKNOWN_2000);

		oldScreenID = Set_LogicPage(SCREEN_0);

		oldWidgetID = Change_Window(widgetID);

		returnValue = 0x0;
	}

	positionX = WinX << 3;

	textWidth = 0;
	textLength = 0;
	maxWidth = (WinW << 3) - Char_Pixel_Width('W') - 1;
	t = text;

	/* Calculate the length and width of the current string */
	for (; *t != '\0'; t++) {
		textWidth += Char_Pixel_Width(*t);
		textLength++;

		if (textWidth >= maxWidth) break;
	}
	*t = '\0';

	Hide_Mouse();

	if (paint) New_Window();

	Fancy_Text_Print(text, positionX, WinY, g_curWidgetFGColourBlink, g_curWidgetFGColourNormal, 0);

	GUI_EditBox_BlinkCursor(positionX + textWidth, false);

	Show_Mouse();

	for (;; sleepIdle()) {
		uint16 keyWidth;
		uint16 key;

		if (tickProc != NULL) {
			returnValue = tickProc();
			if (returnValue != 0) break;
		}

		key = GUI_Widget_HandleEvents(w);

		GUI_EditBox_BlinkCursor(positionX + textWidth, false);

		if (key == 0x0) continue;

		if ((key & 0x8000) != 0) {
			returnValue = key;
			break;
		}
		if (key == 0x2B) {
			returnValue = 0x2B;
			break;
		}
		if (key == 0x6E) {
			*t = '\0';
			returnValue = 0x6B;
			break;
		}

		/* Handle backspace */
		if (key == 0x0F) {
			if (textLength == 0) continue;

			GUI_EditBox_BlinkCursor(positionX + textWidth, true);

			textWidth -= Char_Pixel_Width(*(t - 1));
			textLength--;
			*(--t) = '\0';

			GUI_EditBox_BlinkCursor(positionX + textWidth, false);
			continue;
		}

		key = KN_To_KA(key) & 0xFF;

		/* Names can't start with a space, and should be alpha-numeric */
		if ((key == 0x20 && textLength == 0) || key < 0x20 || key > 0x7E) continue;

		keyWidth = Char_Pixel_Width(key & 0xFF);

		if (textWidth + keyWidth >= maxWidth || textLength >= maxLength) continue;

		/* Add char to the text */
		*t = key & 0xFF;
		*(++t) = '\0';
		textLength++;

		Hide_Mouse();

		GUI_EditBox_BlinkCursor(positionX + textWidth, true);

		/* Draw new character */
		Fancy_Text_Print(text + textLength - 1, positionX + textWidth, WinY, g_curWidgetFGColourBlink, g_curWidgetFGColourNormal, 0x020);

		Show_Mouse();

		textWidth += keyWidth;

		GUI_EditBox_BlinkCursor(positionX + textWidth, false);
	}

	/* Deinitialize */
	{
		Input_Flags_ClearBits(INPUT_FLAG_NO_TRANSLATE);
		Input_Flags_SetBits(INPUT_FLAG_UNKNOWN_2000);

		Change_Window(oldWidgetID);

		Set_LogicPage(oldScreenID);
	}

	return returnValue;
}
