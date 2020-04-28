/** @file src/string.h String definitions. */

#ifndef STRING_H
#define STRING_H

/**
 * Types of Language available in the game.
 */
typedef enum LanguageType {
	LANGUAGE_ENGLISH     = 0,
	LANGUAGE_FRENCH      = 1,
	LANGUAGE_GERMAN      = 2,
	LANGUAGE_ITALIAN     = 3,
	LANGUAGE_SPANISH     = 4,

	LANGUAGE_MAX         = 5,
	LANGUAGE_INVALID     = 0xFF
} LanguageType;

extern const char * const g_languageSuffixes[LANGUAGE_MAX];

extern uint16 UnDip_Text(const char *source, char *dest, uint16 destLen);
extern const char *Language_Name(const char *name);
extern char *Text_String(uint16 stringID);
extern void Fixup_Text(char *string);
extern void String_Init(void);
extern void String_Uninit(void);
extern uint8 *String_NextString(uint8 *ptr);
extern uint8 *String_PrevString(uint8 *ptr);
extern void String_Trim(char *string);

#endif /* STRING_H */
