/** @file src/structure.h %Structure handling definitions. */

#ifndef STRUCTURE_H
#define STRUCTURE_H

#include "object.h"

/**
 * Types of Structures available in the game.
 */
typedef enum StructType {
	STRUCTURE_SLAB_1x1          = 0,
	STRUCTURE_SLAB_2x2          = 1,
	STRUCTURE_PALACE            = 2,
	STRUCTURE_LIGHT_VEHICLE     = 3,
	STRUCTURE_HEAVY_VEHICLE     = 4,
	STRUCTURE_HIGH_TECH         = 5,
	STRUCTURE_HOUSE_OF_IX       = 6,
	STRUCTURE_WOR_TROOPER       = 7,
	STRUCTURE_CONSTRUCTION_YARD = 8,
	STRUCTURE_WINDTRAP          = 9,
	STRUCTURE_BARRACKS          = 10,
	STRUCTURE_STARPORT          = 11,
	STRUCTURE_REFINERY          = 12,
	STRUCTURE_REPAIR            = 13,
	STRUCTURE_WALL              = 14,
	STRUCTURE_TURRET            = 15,
	STRUCTURE_ROCKET_TURRET     = 16,
	STRUCTURE_SILO              = 17,
	STRUCTURE_OUTPOST           = 18,

	STRUCTURE_MAX               = 19,
	STRUCTURE_INVALID           = 0xFF
} StructType;

/**
 * Flags used to indicate structures in a bitmask.
 */
typedef enum StructureFlag {
	FLAG_STRUCTURE_SLAB_1x1          = 1 << STRUCTURE_SLAB_1x1,          /* 0x____01 */
	FLAG_STRUCTURE_SLAB_2x2          = 1 << STRUCTURE_SLAB_2x2,          /* 0x____02 */
	FLAG_STRUCTURE_PALACE            = 1 << STRUCTURE_PALACE,            /* 0x____04 */
	FLAG_STRUCTURE_LIGHT_VEHICLE     = 1 << STRUCTURE_LIGHT_VEHICLE,     /* 0x____08 */
	FLAG_STRUCTURE_HEAVY_VEHICLE     = 1 << STRUCTURE_HEAVY_VEHICLE,     /* 0x____10 */
	FLAG_STRUCTURE_HIGH_TECH         = 1 << STRUCTURE_HIGH_TECH,         /* 0x____20 */
	FLAG_STRUCTURE_HOUSE_OF_IX       = 1 << STRUCTURE_HOUSE_OF_IX,       /* 0x____40 */
	FLAG_STRUCTURE_WOR_TROOPER       = 1 << STRUCTURE_WOR_TROOPER,       /* 0x____80 */
	FLAG_STRUCTURE_CONSTRUCTION_YARD = 1 << STRUCTURE_CONSTRUCTION_YARD, /* 0x__01__ */
	FLAG_STRUCTURE_WINDTRAP          = 1 << STRUCTURE_WINDTRAP,          /* 0x__02__ */
	FLAG_STRUCTURE_BARRACKS          = 1 << STRUCTURE_BARRACKS,          /* 0x__04__ */
	FLAG_STRUCTURE_STARPORT          = 1 << STRUCTURE_STARPORT,          /* 0x__08__ */
	FLAG_STRUCTURE_REFINERY          = 1 << STRUCTURE_REFINERY,          /* 0x__10__ */
	FLAG_STRUCTURE_REPAIR            = 1 << STRUCTURE_REPAIR,            /* 0x__20__ */
	FLAG_STRUCTURE_WALL              = 1 << STRUCTURE_WALL,              /* 0x__40__ */
	FLAG_STRUCTURE_TURRET            = 1 << STRUCTURE_TURRET,            /* 0x__80__ */
	FLAG_STRUCTURE_ROCKET_TURRET     = 1 << STRUCTURE_ROCKET_TURRET,     /* 0x01____ */
	FLAG_STRUCTURE_SILO              = 1 << STRUCTURE_SILO,              /* 0x02____ */
	FLAG_STRUCTURE_OUTPOST           = 1 << STRUCTURE_OUTPOST,           /* 0x04____ */

	FLAG_STRUCTURE_NONE              = 0,
	FLAG_STRUCTURE_NEVER             = -1                                /*!< Special flag to mark that certain buildings can never be built on a Construction Yard. */
} StructureFlag;

/** Available structure layouts. */
typedef enum BSizeType {
	BSIZE_1x1 = 0,
	BSIZE_2x1 = 1,
	BSIZE_1x2 = 2,
	BSIZE_2x2 = 3,
	BSIZE_2x3 = 4,
	BSIZE_3x2 = 5,
	BSIZE_3x3 = 6,

	BSIZE_COUNT = 7
} BSizeType;

/** States a structure can be in */
typedef enum BStateType {
	BSTATE_DETECT    = -2,                        /*!< Used when setting state, meaning to detect which state it has by looking at other properties. */
	BSTATE_JUSTBUILT = -1,                        /*!< This shows you the building animation etc. */
	BSTATE_IDLE      = 0,                         /*!< Structure is doing nothing. */
	BSTATE_BUSY      = 1,                         /*!< Structure is busy (harvester in refinery, unit in repair, .. */
	BSTATE_READY     = 2                          /*!< Structure is ready and unit will be deployed soon. */
} BStateType;

/**
 * A Structure as stored in the memory.
 */
typedef struct Building {
	Object o;                                               /*!< Common to Unit and Structures. */
	uint16 creatorHouseID;                                  /*!< The Index of the House who created this Structure. Required in case of take-overs. */
	uint16 rotationSpriteDiff;                              /*!< Which sprite to show for the current rotation of Turrets etc. */
	uint16 objectType;                                      /*!< Type of Unit/Structure we are building. */
	uint8  upgradeLevel;                                    /*!< The current level of upgrade of the Structure. */
	uint8  upgradeTimeLeft;                                 /*!< Time left before upgrade is complete, or 0 if no upgrade available. */
	uint16 countDown;                                       /*!< General countdown for various of functions. */
	uint16 buildCostRemainder;                              /*!< The remainder of the buildCost for next tick. */
	 int16 state;                                           /*!< The state of the structure. @see StructureState. */
	uint16 hitpointsMax;                                    /*!< Max amount of hitpoints. */
}  Building;

/**
 * Static information per Structure type.
 */
typedef struct BuildingType {
	ObjectType ot;                                           /*!< Common to UnitInfo and BuildingType. */
	uint32 enterFilter;                                     /*!< Bitfield determining which unit is allowed to enter the structure. If bit n is set, then units of type n may enter */
	uint16 creditsStorage;                                  /*!< How many credits this Structure can store. */
	 int16 powerUsage;                                      /*!< How much power this Structure uses (positive value) or produces (negative value). */
	uint16 layout;                                          /*!< Layout type of Structure. */
	uint16 iconGroup;                                       /*!< In which IconGroup the sprites of the Structure belongs. */
	uint8  animationIndex[3];                               /*!< The index inside g_table_animation_structure for the Animation of the Structure. */
	uint8  buildableUnits[8];                               /*!< Which units this structure can produce. */
	uint16 upgradeCampaign[3];                              /*!< Minimum campaign for upgrades. */
} BuildingType;

/** X/Y pair defining a 2D size. */
typedef struct XYSize {
	uint16 width;  /*!< Horizontal length. */
	uint16 height; /*!< Vertical length. */
} XYSize;

struct House;
struct Widget;

extern BuildingType g_table_BuildingType[STRUCTURE_MAX];
extern const uint16  g_table_structure_layoutTiles[BSIZE_COUNT][9];
extern const uint16  g_table_structure_layoutEdgeTiles[BSIZE_COUNT][8];
extern const uint16  g_table_structure_layoutTileCount[BSIZE_COUNT];
extern const tile32  g_table_structure_layoutTileDiff[BSIZE_COUNT];
extern const XYSize  g_table_structure_layoutSize[BSIZE_COUNT];
extern const int16   g_table_structure_layoutTilesAround[BSIZE_COUNT][16];

extern Building *g_structureActive;
extern uint16 g_structureActivePosition;
extern uint16 g_structureActiveType;

extern uint16 g_structureIndex;

extern void GameLoop_Structure(void);
extern uint8 Structure_StringToType(const char *name);
extern Building *Structure_Create(uint16 index, uint8 typeID, uint8 houseID, uint16 position);
extern bool Structure_Place(Building *s, uint16 position);
extern void Structure_CalculateHitpointsMax(struct House *h);
extern void Structure_SetState(Building *s, int16 animation);
extern Building *Structure_Get_ByPackedTile(uint16 packed);
extern uint32 Structure_GetStructuresBuilt(struct House *h);
extern int16 Structure_IsValidBuildLocation(uint16 position, StructType type);
extern bool Structure_Save(FILE *fp);
extern bool Structure_Load(FILE *fp, uint32 length);
extern void Structure_ActivateSpecial(Building *s);
extern void Structure_RemoveFog(Building *s);
extern bool Structure_Damage(Building *s, uint16 Damage, uint16 range);
extern bool Structure_IsUpgradable(Building *s);
extern bool Structure_ConnectWall(uint16 position, bool recurse);
extern struct Unit *Structure_GetLinkedUnit(Building *s);
extern void Structure_UntargetMe(Building *s);
extern uint16 Structure_FindFreePosition(Building *s, bool checkForSpice);
extern void Structure_Remove(Building *s);
extern bool Structure_BuildObject(Building *s, uint16 objectType);
extern bool Structure_SetUpgradingState(Building *s, int8 value, struct Widget *w);
extern bool Structure_SetRepairingState(Building *s, int8 value, struct Widget *w);
extern void Structure_UpdateMap(Building *s);
extern uint32 Structure_GetBuildable(Building *s);
extern void Structure_HouseUnderAttack(uint8 houseID);
extern uint16 Structure_AI_PickNextToBuild(Building *s);

#endif /* STRUCTURE_H */
