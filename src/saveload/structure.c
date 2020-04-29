/** @file src/saveload/structure.c Load/save routines for Structure. */

#include <stdio.h>
#include <string.h>
#include "types.h"

#include "saveload.h"
#include "../house.h"
#include "../structure.h"
#include "../pool/structure.h"
#include "../pool/pool.h"

static const SaveLoadDesc s_saveStructure[] = {
	SLD_SLD   (Building,              o, g_saveObject),
	SLD_ENTRY (Building, SLDT_UINT16, creatorHouseID),
	SLD_ENTRY (Building, SLDT_UINT16, rotationSpriteDiff),
	SLD_EMPTY (           SLDT_UINT8),
	SLD_ENTRY (Building, SLDT_UINT16, objectType),
	SLD_ENTRY (Building, SLDT_UINT8,  upgradeLevel),
	SLD_ENTRY (Building, SLDT_UINT8,  upgradeTimeLeft),
	SLD_ENTRY (Building, SLDT_UINT16, countDown),
	SLD_ENTRY (Building, SLDT_UINT16, buildCostRemainder),
	SLD_ENTRY (Building,  SLDT_INT16, State),
	SLD_ENTRY (Building, SLDT_UINT16, hitpointsMax),
	SLD_END
};

/**
 * Load all Structures from a file.
 * @param fp The file to load from.
 * @param length The length of the data chunk.
 * @return True if and only if all bytes were read successful.
 */
bool Structure_Load(FILE *fp, uint32 length)
{
	while (length > 0) {
		Building *s;
		Building sl;

		memset(&sl, 0, sizeof(sl));

		/* Read the next Structure from disk */
		if (!SaveLoad_Load(s_saveStructure, fp, &sl)) return false;

		length -= SaveLoad_GetLength(s_saveStructure);

		sl.o.script.scriptInfo = g_scriptStructure;
		sl.o.script.script = g_scriptStructure->start + (size_t)sl.o.script.script;
		if (sl.upgradeTimeLeft == 0) sl.upgradeTimeLeft = Structure_IsUpgradable(&sl) ? 100 : 0;

		/* Get the Structure from the pool */
		s = Structure_Get_ByIndex(sl.o.index);
		if (s == NULL) return false;

		/* Copy over the data */
		*s = sl;
	}
	if (length != 0) return false;

	Structure_Recount();

	return true;
}

/**
 * Save all Structures to a file. It converts pointers to indices where needed.
 * @param fp The file to save to.
 * @return True if and only if all bytes were written successful.
 */
bool Structure_Save(FILE *fp)
{
	PoolFindStruct find;

	find.houseID = HOUSE_INVALID;
	find.type    = 0xFFFF;
	find.index   = 0xFFFF;

	while (true) {
		Building *s;
		Building ss;

		s = Structure_Find(&find);
		if (s == NULL) break;
		ss = *s;

		if (!SaveLoad_Save(s_saveStructure, fp, &ss)) return false;
	}

	return true;
}
