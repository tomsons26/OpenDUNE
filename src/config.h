/** @file src/config.h Configuration and options load and save definitions. */

#ifndef CONFIG_H
#define CONFIG_H

MSVC_PACKED_BEGIN
/**
 * This is the layout of decoded dune.cfg.
 */
typedef struct ConfigType {
	/* 0000(1)   */ PACK uint8  VideoMode;                 /*!< Graphic mode to use. */
	/* 0001(1)   */ PACK uint8  MusicCard;                   /*!< Index into music drivers array. */
	/* 0002(1)   */ PACK uint8  SoundCard;                   /*!< Index into sound drivers array. */
	/* 0003(1)   */ PACK uint8  DigitCard;                   /*!< Index into digitized sound drivers array. */
	/* 0004(1)   */ PACK bool   UseMouse;                   /*!< Use Mouse. */
	/* 0005(1)   */ PACK bool   UseXMS;                     /*!< Use Extended Memory. */
	/* 0006(1)   */ PACK uint8  UseHMA;              /*!< ?? */
	/* 0007(1)   */ PACK uint8  UseEMS;              /*!< ?? */
	/* 0008(1)   */ PACK uint8  Language;                   /*!< @see Language. */
	/* 0009(1)   */ PACK uint8  CheckSum;                   /*!< Used to check validity on config data. See Config_Read(). */
} GCC_PACKED ConfigType;
MSVC_PACKED_END
assert_compile(sizeof(ConfigType) == 0xA);

/**
 * This is the layout of options.cfg.
 */
typedef struct GameCfg {
	uint16 music;                      /*!< 0:Off, 1:On. */
	uint16 sounds;                     /*!< 0:Off, 1:On. */
	uint16 GameSpeed;                  /*!< 0:Slowest, 1:Slow, 2:Normal, 3:Fast, 4:Fastest. */
	uint16 hints;                      /*!< 0:Off, 1:On. */
	uint16 AutoScroll;                 /*!< 0:Off, 1:On. */
} GameCfg;

extern GameCfg g_gameConfig;
extern ConfigType g_config;

extern bool g_enableSoundMusic;
extern bool g_enableVoices;

extern bool Read_Config_Struct(const char *filename, ConfigType *config);
extern bool Write_Config_Struct(const char *filename, ConfigType *config);
extern bool Config_Default(ConfigType *config);
extern bool GameOptions_Load(void);
extern void GameOptions_Save(void);

#endif /* CONFIG_H */
