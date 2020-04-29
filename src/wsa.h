/** @file src/wsa.h WSA definitions. */

#ifndef WSA_H
#define WSA_H

extern uint16 Animate_Frame_Count(void *wsa);
extern void *Open_Animation(const char *filename, void *wsa, uint32 wsaSize, bool reserveDisplayFrame);
extern void Close_Animation(void *wsa);
extern bool Animate_Frame(void *wsa, uint16 frameNext, uint16 posX, uint16 posY, Screen screenID);

#endif /* WSA_H */
