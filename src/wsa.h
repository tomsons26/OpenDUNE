/** @file src/wsa.h WSA definitions. */

#ifndef WSA_H
#define WSA_H

extern uint16 Animate_Frame_Count(void *handle);
extern void *Open_Animation(const char *filename, void *wsa, uint32 wsaSize, bool reserveDisplayFrame);
extern void Close_Animation(void *wsa);
extern bool WSA_DisplayFrame(void *wsa, uint16 frameNext, uint16 posX, uint16 posY, Screen screenID);

#endif /* WSA_H */
