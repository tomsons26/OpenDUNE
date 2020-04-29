/** @file src/input/mouse.h Mouse definitions. */

#ifndef MOUSE_H
#define MOUSE_H

extern uint16 MouseUpdate;
extern bool   g_doubleWidth;
extern uint16 MouseX;
extern uint16 MouseY;
extern uint16 MouseXOld;
extern uint16 MouseYOld;
extern uint8  MouseButton;
extern uint16 MouseQX;
extern uint16 MouseQY;
extern uint16 MCState;
extern uint16 MouseLeft;
extern uint16 MouseRight;
extern uint16 MouseTop;
extern uint16 MouseBottom;
extern uint16 g_regionMinX;
extern uint16 g_regionMinY;
extern uint16 g_regionMaxX;
extern uint16 g_regionMaxY;

extern uint8 MDisabled;
extern uint8 g_mouseHiddenDepth;
extern uint8 g_mouseFileID;
extern bool g_mouseNoRecordedValue;

extern uint16 LogTimeFrame;
extern uint16 g_mouseRecordedTimer;
extern uint16 LogXtraX;
extern uint16 LogXtraY;

extern uint8 RecordMode;
extern uint16 g_inputFlags;

extern void Mouse_Init(void);
extern void Mouse_EventHandler(uint16 mousePosX, uint16 mousePosY, bool mouseButtonLeft, bool mouseButtonRight);
extern void Mouse_SetRegion(uint16 left, uint16 top, uint16 right, uint16 bottom);
extern uint16 Mouse_InsideRegion(int16 left, int16 top, int16 right, int16 bottom);
extern void Mouse_SetMouseMode(uint8 mouseMode, const char *filename);
extern uint16 Mouse_KeyNum(uint16 newButtonState);
extern void Mouse_HandleMovement(uint16 newButtonState, uint16 mouseX, uint16 mouseY);
extern void Mouse_HandleMovementIfMoved(uint16 newButtonState);

#endif /* MOUSE_H */
