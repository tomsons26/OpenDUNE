/** @file src/audio/sound.c Sound routines. */

#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "../os/common.h"
#include "../os/strings.h"

#include "sound.h"

#include "driver.h"
#include "mt32mpu.h"
#include "../config.h"
#include "../file.h"
#include "../gui/gui.h"
#include "../house.h"
#include "../opendune.h"
#include "../string.h"
#include "../tile.h"
#include "../timer.h"

/*some internal names for sounds left in D2 Genesis version
ROM:00020870 6379 7269 6C73 2063 6F75+                    dc.b 'cyrils council ',0
ROM:00020880 616D 6D6F 6E73 2061 6476+                    dc.b 'ammons advice  ',0
ROM:00020890 7261 646E 6F72 7320 7363+                    dc.b 'radnors scheme ',0
ROM:000208A0 7468 6520 6C65 676F 2074+                    dc.b 'the lego tune  ',0
ROM:000208B0 7475 7262 756C 656E 6365+                    dc.b 'turbulence     ',0
ROM:000208C0 7370 6963 6520 7472 6970+                    dc.b 'spice trip     ',0
ROM:000208D0 636F 6D6D 616E 6420 706F+                    dc.b 'command post   ',0
ROM:000208E0 7472 656E 6368 696E 6720+                    dc.b 'trenching      ',0
ROM:000208F0 7374 6172 706F 7274 2020+                    dc.b 'starport       ',0
ROM:00020900 6576 6173 6976 6520 6163+                    dc.b 'evasive action ',0
ROM:00020910 6368 6F73 656E 2064 6573+                    dc.b 'chosen destiny ',0
ROM:00020920 636F 6E71 7565 7374 2020+                    dc.b 'conquest       ',0
ROM:00020930 736C 6974 6865 7269 6E20+                    dc.b 'slitherin      ',0
ROM:00020940 6861 726B 6F6E 6E65 6E20+                    dc.b 'harkonnen rules',0
ROM:00020950 6174 7265 6469 6573 2064+                    dc.b 'atredies dirge ',0
ROM:00020960 6F72 646F 7320 6469 7267+                    dc.b 'ordos dirge    ',0
ROM:00020970 6861 726B 6F6E 6E65 6E20+                    dc.b 'harkonnen dirge',0
ROM:00020980 6669 6E61 6C65 2020 2020+                    dc.b 'finale         ',0
ROM:00020990 7461 7267 6574 2020 2020+                    dc.b 'target         ',0
ROM:000209A0 6D65 6E75 2073 656C 6563+                    dc.b 'menu select    ',0
ROM:000209B0 706F 7369 7469 7665 2073+                    dc.b 'positive select',0
ROM:000209C0 696E 7661 6C69 6420 7365+                    dc.b 'invalid select ',0
ROM:000209D0 6275 696C 6469 6E67 2020+                    dc.b 'building       ',0
ROM:000209E0 706C 6163 656D 656E 7420+                    dc.b 'placement      ',0
ROM:000209F0 636F 756E 7464 6F77 6E20+                    dc.b 'countdown      ',0
ROM:00020A00 6372 6564 6974 2064 6F77+                    dc.b 'credit down    ',0
ROM:00020A10 6372 6564 6974 2075 7020+                    dc.b 'credit up      ',0
ROM:00020A20 7269 666C 6520 2866 7829+                    dc.b 'rifle (fx)     ',0
ROM:00020A30 7269 666C 6520 2020 2020+                    dc.b 'rifle          ',0
ROM:00020A40 6D61 6368 696E 6567 756E+                    dc.b 'machinegun (fx)',0
ROM:00020A50 6D61 6368 696E 6567 756E+                    dc.b 'machinegun     ',0
ROM:00020A60 6361 6E6E 6F6E 2028 6678+                    dc.b 'cannon (fx)    ',0
ROM:00020A70 6361 6E6E 6F6E 2020 2020+                    dc.b 'cannon         ',0
ROM:00020A80 726F 636B 6574 206C 6175+                    dc.b 'rocket launch  ',0
ROM:00020A90 736F 6E69 6320 626C 6173+                    dc.b 'sonic blast    ',0
ROM:00020AA0 6D69 7373 696C 6520 2020+                    dc.b 'missile        ',0
ROM:00020AB0 6865 6176 7920 6472 6F70+                    dc.b 'heavy drop     ',0
ROM:00020AC0 6761 7320 6869 7420 2020+                    dc.b 'gas hit        ',0
ROM:00020AD0 776F 726D 2065 6174 7320+                    dc.b 'worm eats (fx) ',0
ROM:00020AE0 776F 726D 2065 6174 7320+                    dc.b 'worm eats      ',0
ROM:00020AF0 6578 706C 6F73 696F 6E20+                    dc.b 'explosion 1    ',0
ROM:00020B00 6578 706C 6F73 696F 6E20+                    dc.b 'explosion 2    ',0
ROM:00020B10 6578 706C 6F73 696F 6E20+                    dc.b 'explosion 3    ',0
ROM:00020B20 6D75 6C74 6920 6578 706C+                    dc.b 'multi explosion',0
ROM:00020B30 6475 6420 6578 706C 6F73+                    dc.b 'dud explosion  ',0
ROM:00020B40 7363 7265 616D 2020 2020+                    dc.b 'scream         ',0
ROM:00020B50 7371 7569 7368 2020 2020+                    dc.b 'squish         ',0
ROM:00020B60 7374 6174 6963 2028 6678+                    dc.b 'static (fx)    ',0
ROM:00020B70 7374 6174 6963 2020 2020+                    dc.b 'static         ',0
ROM:00020B80 636F 6E73 742E 2063 6F6D+                    dc.b 'const. complete',0
ROM:00020B90 756E 6974 2061 7070 726F+                    dc.b 'unit approach  ',0
ROM:00020BA0 756E 6465 7220 6174 7461+                    dc.b 'under attack   ',0
ROM:00020BB0 6465 6174 6820 6861 6E64+                    dc.b 'death hand     ',0
ROM:00020BC0 7965 7320 7369 7220 2020+                    dc.b 'yes sir        ',0
ROM:00020BD0 6163 6B6E 6F77 6C65 6467+                    dc.b 'acknowledged   ',0
ROM:00020BE0 6D6F 7669 6E67 206F 7574+                    dc.b 'moving out     ',0
ROM:00020BF0 7265 706F 7274 696E 6720+                    dc.b 'reporting      ',0
ROM:00020C00 706C 616E 6574 2073 6869+                    dc.b 'planet shimmer ',0 
*/

static void *g_voiceData[NUM_VOICES];            /*!< Preloaded Voices sound data */
static uint32 g_voiceDataSize[NUM_VOICES];       /*!< Preloaded Voices sound data size in byte */
static const char *s_currentMusic = NULL;        /*!< Currently loaded music file. */
static uint16 s_spokenWords[NUM_SPEECH_PARTS];   /*!< Buffer with speech to play. */
static int16 s_currentVoicePriority;            /*!< Priority of the currently playing Speech */

static void *Sound_LoadVoc(const char *filename, uint32 *retFileSize);

static void Play_Score(int16 index, uint16 volume)
{
	Driver *music = g_driverMusic;
	MSBuffer *musicBuffer = g_bufferMusic;

	if (index < 0 || index > 120 || g_gameConfig.music == 0) return;

	if (music->index == 0xFFFF) return;

	if (musicBuffer->index != 0xFFFF) {
		MPU_Stop(musicBuffer->index);
		MPU_ClearData(musicBuffer->index);
		musicBuffer->index = 0xFFFF;
	}

	musicBuffer->index = MPU_SetData(music->content, index, musicBuffer->buffer);

	MPU_Play(musicBuffer->index);
	MPU_SetVolume(musicBuffer->index, ((volume & 0xFF) * 90) / 256, 0);
}

static void Load_Score_File(const char *musicName)
{
	Driver *music = g_driverMusic;
	Driver *sound = g_driverSound;

	Stop_Score();

	if (music->index == 0xFFFF) return;

	if (music->content == sound->content) {
		music->content         = NULL;
		music->filename[0]     = '\0';
		music->contentMalloced = false;
	} else {
		Driver_UnloadFile(music);
	}

	if (sound->filename[0] != '\0' && musicName != NULL && strcasecmp(Data_File_Name(musicName, music), sound->filename) == 0) {
		g_driverMusic->content         = g_driverSound->content;
		memcpy(g_driverMusic->filename, g_driverSound->filename, sizeof(g_driverMusic->filename));
		g_driverMusic->contentMalloced = g_driverSound->contentMalloced;

		return;
	}

	Driver_LoadFile(musicName, music);
}

/**
 * Plays a music.
 * @param index The index of the music to play.
 */
void Music_Play(uint16 musicID)
{
	static uint16 currentMusicID = 0;

	if (musicID == 0xFFFF || musicID >= 38 || musicID == currentMusicID) return;

	currentMusicID = musicID;

	if (g_table_musics[musicID].string != s_currentMusic) {
		s_currentMusic = g_table_musics[musicID].string;

		Stop_Score();
		Driver_Voice_Play(NULL, 0xFF);
		Load_Score_File(NULL);
		Driver_Sound_LoadFile(NULL);
		Load_Score_File(s_currentMusic);
		Driver_Sound_LoadFile(s_currentMusic);
	}

	Play_Score(g_table_musics[musicID].index, 0xFF);
}

/**
 * Initialises the MT-32.
 * @param index The index of the music to play.
 */
void MT32_Init(void)
{
	uint16 left = 0;

	Load_Score_File("DUNEINIT");

	Play_Score(0, 0xFF);

	Text_Print(Extract_String(15), 0, 0, 15, 12); /* "Initializing the MT-32" */

	while (Score_Status()) {
		Timer_Sleep(60);

		left += 6;
		Text_Print(".", left, 10, 15, 12);
	}
}

/**
 * Play a voice. Volume is based on distance to position.
 * @param voiceID Which voice to play.
 * @param position Which position to play it on.
 */
void Voice_PlayAtTile(int16 voiceID, CellStruct position)
{
	uint16 index;
	uint16 volume;

	if (voiceID < 0 || voiceID >= 120) return;
	if (!g_gameConfig.sounds) return;

	volume = 255;
	if (position.x != 0 || position.y != 0) {
		volume = Tile_GetDistancePacked(g_minimapPosition, Tile_PackTile(position));
		if (volume > 64) volume = 64;

		volume = 255 - (volume * 255 / 80);
	}

	index = g_table_voiceMapping[voiceID];

	if (g_enableVoices != 0 && index != 0xFFFF && g_voiceData[index] != NULL && g_table_voices[index].priority >= s_currentVoicePriority) {
		s_currentVoicePriority = g_table_voices[index].priority;
		memmove(g_readBuffer, g_voiceData[index], g_voiceDataSize[index]);

		Driver_Voice_Play(g_readBuffer, s_currentVoicePriority);
	} else {
		Driver_Sound_Play(voiceID, volume);
	}
}

/**
 * Play a voice.
 * @param voiceID The voice to play.
 */
void Voice_Play(int16 voiceID)
{
	CellStruct tile;

	tile.x = 0;
	tile.y = 0;
	Voice_PlayAtTile(voiceID, tile);
}

/**
 * Load voices.
 * voiceSet 0xFFFE is for Game Intro.
 * voiceSet 0xFFFF is for Game End.
 * @param voiceSet Voice set to load : either a HouseID, or special values 0xFFFE or 0xFFFF.
 */
void Voice_LoadVoices(uint16 voiceSet)
{
	static uint16 currentVoiceSet = 0xFFFE;
	int prefixChar = ' ';
	uint16 voice;

	if (g_enableVoices == 0) return;

	for (voice = 0; voice < NUM_VOICES; voice++) {
		switch (g_table_voices[voice].string[0]) {
			case '%':
				if (g_config.Language != LANGUAGE_ENGLISH || currentVoiceSet == voiceSet) {
					if (voiceSet != 0xFFFF && voiceSet != 0xFFFE) break;
				}

				free(g_voiceData[voice]);
				g_voiceData[voice] = NULL;
				break;

			case '+':
				if (voiceSet != 0xFFFF && voiceSet != 0xFFFE) break;

				free(g_voiceData[voice]);
				g_voiceData[voice] = NULL;
				break;

			case '-':
				if (voiceSet == 0xFFFF) break;

				free(g_voiceData[voice]);
				g_voiceData[voice] = NULL;
				break;

			case '/':
				if (voiceSet != 0xFFFE) break;

				free(g_voiceData[voice]);
				g_voiceData[voice] = NULL;
				break;

			case '?':
				if (voiceSet == 0xFFFF) break;

				/* No free() as there was never a malloc(). */
				g_voiceData[voice] = NULL;
				break;

			default:
				break;
		}
	}

	if (currentVoiceSet == voiceSet) return;

	for (voice = 0; voice < NUM_VOICES; voice++) {
		char filename[16];
		const char *str = g_table_voices[voice].string;
		switch (*str) {
			case '%':
				if (g_voiceData[voice] != NULL ||
						currentVoiceSet == voiceSet || voiceSet == 0xFFFF || voiceSet == 0xFFFE) break;

				switch (g_config.Language) {
					case LANGUAGE_FRENCH: prefixChar = 'F'; break;
					case LANGUAGE_GERMAN: prefixChar = 'G'; break;
					default: prefixChar = g_table_HouseType[voiceSet].prefixChar;
				}
				snprintf(filename, sizeof(filename), str, prefixChar);

				g_voiceData[voice] = Sound_LoadVoc(filename, &g_voiceDataSize[voice]);
				break;

			case '+':
				if (voiceSet == 0xFFFF || g_voiceData[voice] != NULL) break;

				switch (g_config.Language) {
					case LANGUAGE_FRENCH:  prefixChar = 'F'; break;
					case LANGUAGE_GERMAN:  prefixChar = 'G'; break;
					default: prefixChar = 'Z'; break;
				}
				snprintf(filename, sizeof(filename), str + 1, prefixChar);

				/* XXX - In the 1.07us datafiles, a few files are named differently:
				 *
				 *  moveout.voc
				 *  overout.voc
				 *  report1.voc
				 *  report2.voc
				 *  report3.voc
				 *
				 * They come without letter in front of them. To make things a bit
				 *  easier, just check if the file exists, then remove the first
				 *  letter and see if it works then.
				 */
				if (!File_Exists(filename)) {
					memmove(filename, filename + 1, strlen(filename));
				}

				g_voiceData[voice] = Sound_LoadVoc(filename, &g_voiceDataSize[voice]);
				break;

			case '-':
				if (voiceSet != 0xFFFF || g_voiceData[voice] != NULL) break;

				g_voiceData[voice] = Sound_LoadVoc(str + 1, &g_voiceDataSize[voice]);
				break;

			case '/':
				if (voiceSet != 0xFFFE) break;

				g_voiceData[voice] = Sound_LoadVoc(str + 1, &g_voiceDataSize[voice]);
				break;

			case '?':
				break;

			default:
				if (g_voiceData[voice] != NULL) break;

				g_voiceData[voice] = Sound_LoadVoc(str, &g_voiceDataSize[voice]);
				break;
		}
	}
	currentVoiceSet = voiceSet;
}

/**
 * Unload voices.
 */
void Voice_UnloadVoices(void)
{
	uint16 voice;

	for (voice = 0; voice < NUM_VOICES; voice++) {
		free(g_voiceData[voice]);
		g_voiceData[voice] = NULL;
	}
}

/**
 * Start playing a sound sample.
 * @param index Sample to play.
 */
void Sound_StartSound(uint16 index)
{
	if (index == 0xFFFF || g_gameConfig.sounds == 0 || (int16)g_table_voices[index].priority < (int16)s_currentVoicePriority) return;

	s_currentVoicePriority = g_table_voices[index].priority;

	if (g_voiceData[index] != NULL) {
		Driver_Voice_Play(g_voiceData[index], 0xFF);
	} else {
		char filenameBuffer[16];
		const char *filename;

		filename = g_table_voices[index].string;
		if (filename[0] == '?') {
			snprintf(filenameBuffer, sizeof(filenameBuffer), filename + 1, g_playerHouseID < HOUSE_MAX ? g_table_HouseType[g_playerHouseID].prefixChar : ' ');

			Driver_Voice_LoadFile(filenameBuffer, g_readBuffer, g_readBufferSize);

			Driver_Voice_Play(g_readBuffer, 0xFF);
		}
	}
}

/**
 * Output feedback about events of the game.
 * @param index Feedback to provide (\c 0xFFFF means do nothing, \c 0xFFFE means stop, otherwise a feedback code).
 * @note If sound is disabled, the main viewport is used to display a message.
 */
void Sound_Output_Feedback(uint16 index)
{
	if (index == 0xFFFF) return;

	if (index == 0xFFFE) {
		uint8 i;

		/* Clear spoken audio. */
		for (i = 0; i < lengthof(s_spokenWords); i++) {
			s_spokenWords[i] = 0xFFFF;
		}

		Driver_Voice_Stop();

		g_viewportMessageText = NULL;
		if ((g_viewportMessageCounter & 1) != 0) {
			g_viewport_forceRedraw = true;
			g_viewportMessageCounter = 0;
		}
		s_currentVoicePriority = 0;

		return;
	}

	if (g_enableVoices == 0 || g_gameConfig.sounds == 0) {
		Driver_Sound_Play(g_feedback[index].VocType, 0xFF);

		g_viewportMessageText = Extract_String(g_feedback[index].messageId);

		if ((g_viewportMessageCounter & 1) != 0) {
			g_viewport_forceRedraw = true;
		}

		g_viewportMessageCounter = 4;

		return;
	}

	/* If nothing is being said currently, load new words. */
	if (s_spokenWords[0] == 0xFFFF) {
		uint8 i;

		for (i = 0; i < lengthof(s_spokenWords); i++) {
			s_spokenWords[i] = (g_config.Language == LANGUAGE_ENGLISH) ? g_feedback[index].voiceId[i] : g_translatedVoice[index][i];
		}
	}

	Sound_StartSpeech();
}

/**
 * Start speech.
 * Start a new speech fragment if possible.
 * @return Sound is produced.
 */
bool Sound_StartSpeech(void)
{
	if (g_gameConfig.sounds == 0) return false;

	if (Driver_Voice_IsPlaying()) return true;

	s_currentVoicePriority = 0;

	if (s_spokenWords[0] == 0xFFFF) return false;

	Sound_StartSound(s_spokenWords[0]);
	/* Move speech parts one place. */
	memmove(&s_spokenWords[0], &s_spokenWords[1], sizeof(s_spokenWords) - sizeof(s_spokenWords[0]));
	s_spokenWords[lengthof(s_spokenWords) - 1] = 0xFFFF;

	return true;
}

/**
 * Load a voice file to a malloc'd buffer.
 * @param filename The name of the file to load.
 * @return Where the file is loaded.
 */
static void *Sound_LoadVoc(const char *filename, uint32 *retFileSize)
{
	uint32 fileSize;
	void *res;

	if (filename == NULL) return NULL;
	if (!File_Exists_GetSize(filename, &fileSize)) return NULL;

	fileSize += 1;
	fileSize &= 0xFFFFFFFE;

	*retFileSize = fileSize;
	res = malloc(fileSize);
	Driver_Voice_LoadFile(filename, res, fileSize);

	return res;
}
