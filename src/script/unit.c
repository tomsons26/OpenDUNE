/** @file src/script/unit.c %Unit script routines. */

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "types.h"
#include "../os/math.h"

#include "script.h"

#include "../animation.h"
#include "../audio/sound.h"
#include "../config.h"
#include "../explosion.h"
#include "../gui/gui.h"
#include "../house.h"
#include "../map.h"
#include "../opendune.h"
#include "../pool/unit.h"
#include "../pool/pool.h"
#include "../pool/structure.h"
#include "../scenario.h"
#include "../structure.h"
#include "../table/strings.h"
#include "../tools.h"
#include "../tile.h"
#include "../string.h"
#include "../unit.h"

typedef struct PathType {
	uint16 StartCell;                                          /*!< From where we are pathfinding. */
	 int16 Score;                                           /*!< The total Score for this Path. */
	uint16 Length;                                       /*!< The size of this Path. */
	uint8 *Moves;                                          /*!< A buffer to store the Path. */
} PathType;

typedef enum FacingType {
	FACING_NONE = -1,

	FACING_FIXUP_MARK = -2,

	FACING_EDGE_LEFT = -1,

	FACING_EDGE_RIGHT = 1,
	FACING_ORDINAL_TEST = 1,

	FACING_FIRST = 0,

	FACING_NORTH = 0,		//North
	FACING_NORTH_EAST = 1,	//North East
	FACING_EAST = 2,		//East
	FACING_SOUTH_EAST = 3,	//South East
	FACING_SOUTH = 4,		//South
	FACING_SOUTH_WEST = 5,	//South West
	FACING_WEST = 6,		//West
	FACING_NORTH_WEST = 7,	//North West

	FACING_LAST = 7,

	FACING_COUNT = 8
};


static const int16 AdjacentCell[8] = {-64, -63, 1, 65, 64, 63, -1, -65}; /*!< Tile index change when moving in a direction. */

/**
 * Create a new soldier unit.
 *
 * Stack: 1 - Action for the new Unit.
 *
 * @param script The script engine to operate on.
 * @return 1 if a new Unit has been created, 0 otherwise.
 */
uint16 Script_Unit_RandomSoldier(ScriptEngine *script)
{
	Unit *u;
	Unit *nu;
	CellStruct position;

	u = g_scriptCurrentUnit;

	if (Tools_Random_256() >= g_table_unitInfo[u->o.type].o.spawnChance) return 0;

	position = Coord_Scatter(u->o.position, 20, true);

	nu = Unit_Create(UNIT_INDEX_INVALID, UNIT_SOLDIER, u->o.houseID, position, Tools_Random_256());

	if (nu == NULL) return 0;

	nu->deviated = u->deviated;

	Unit_SetAction(nu, STACK_PEEK(1));

	return 1;
}

/**
 * Gets the best target for the current unit.
 *
 * Stack: 1 - How to determine the best target.
 *
 * @param script The script engine to operate on.
 * @return The encoded index of the best target or 0 if none found.
 */
uint16 Script_Unit_FindBestTarget(ScriptEngine *script)
{
	Unit *u;

	u = g_scriptCurrentUnit;

	return Unit_FindBestTargetEncoded(u, STACK_PEEK(1));
}

/**
 * Get the priority a target has for the current unit.
 *
 * Stack: 1 - The encoded target.
 *
 * @param script The script engine to operate on.
 * @return The priority of the target.
 */
uint16 Script_Unit_GetTargetPriority(ScriptEngine *script)
{
	Unit *u;
	Unit *target;
	Structure *s;
	uint16 encoded;

	u = g_scriptCurrentUnit;
	encoded = STACK_PEEK(1);

	target = Tools_Index_GetUnit(encoded);
	if (target != NULL) return Unit_GetTargetUnitPriority(u, target);

	s = Tools_Index_GetStructure(encoded);
	if (s == NULL) return 0;

	return Unit_GetTargetStructurePriority(u, s);
}

/**
 * Delivery of transport, either to structure or to a tile.
 *
 * Stack: *none*.
 *
 * @param script The script engine to operate on.
 * @return One if delivered, zero otherwise..
 */
uint16 Script_Unit_TransportDeliver(ScriptEngine *script)
{
	Unit *u;
	Unit *u2;

	VARIABLE_NOT_USED(script);

	u = g_scriptCurrentUnit;

	if (u->o.linkedID == 0xFF) return 0;
	if (Tools_Index_GetType(u->targetMove) == IT_UNIT) return 0;

	if (Tools_Index_GetType(u->targetMove) == IT_STRUCTURE) {
		const StructureInfo *si;
		Structure *s;

		s = Tools_Index_GetStructure(u->targetMove);
		si = &g_table_structureInfo[s->o.type];

		if (s->o.type == STRUCTURE_STARPORT) {
			uint16 ret = 0;

			if (s->state == STRUCTURE_STATE_BUSY) {
				s->o.linkedID = u->o.linkedID;
				u->o.linkedID = 0xFF;
				u->o.flags.s.inTransport = false;
				u->amount = 0;

				Unit_UpdateMap(2, u);

				Voice_PlayAtTile(24, u->o.position);

				Structure_SetState(s, STRUCTURE_STATE_READY);

				ret = 1;
			}

			Object_Script_Variable4_Clear(&u->o);
			u->targetMove = 0;

			return ret;
		}

		if ((s->state == STRUCTURE_STATE_IDLE || (si->o.flags.busyStateIsIncoming && s->state == STRUCTURE_STATE_BUSY)) && s->o.linkedID == 0xFF) {
			Voice_PlayAtTile(24, u->o.position);

			Unit_EnterStructure(Unit_Get_ByIndex(u->o.linkedID), s);

			Object_Script_Variable4_Clear(&u->o);
			u->targetMove = 0;

			u->o.linkedID = 0xFF;
			u->o.flags.s.inTransport = false;
			u->amount = 0;

			Unit_UpdateMap(2, u);

			return 1;
		}

		Object_Script_Variable4_Clear(&u->o);
		u->targetMove = 0;

		return 0;
	}

	if (!Map_IsValidPosition(Tile_PackTile(Tile_Center(u->o.position)))) return 0;

	u2 = Unit_Get_ByIndex(u->o.linkedID);

	if (!Unit_SetPosition(u2, Tile_Center(u->o.position))) return 0;

	if (u2->o.houseID == g_playerHouseID) {
		Voice_PlayAtTile(24, u->o.position);
	}

	Unit_SetOrientation(u2, u->orientation[0].current, true, 0);
	Unit_SetOrientation(u2, u->orientation[0].current, true, 1);
	Unit_SetSpeed(u2, 0);

	u->o.linkedID = u2->o.linkedID;
	u2->o.linkedID = 0xFF;

	if (u->o.linkedID != 0xFF) return 1;

	u->o.flags.s.inTransport = false;

	Object_Script_Variable4_Clear(&u->o);
	u->targetMove = 0;

	return 1;
}

/**
 * Pickup a unit (either from structure or on the map). The unit that does the
 *  picking up returns the unit to his last position.
 *
 * Stack: *none*.
 *
 * @param script The script engine to operate on.
 * @return The value 0. Always.
 */
uint16 Script_Unit_Pickup(ScriptEngine *script)
{
	Unit *u;

	VARIABLE_NOT_USED(script);

	u = g_scriptCurrentUnit;

	if (u->o.linkedID != 0xFF) return 0;

	switch (Tools_Index_GetType(u->targetMove)) {
		case IT_STRUCTURE: {
			Structure *s;
			Unit *u2;

			s = Tools_Index_GetStructure(u->targetMove);

			/* There was nothing to pickup here */
			if (s->state != STRUCTURE_STATE_READY) {
				Object_Script_Variable4_Clear(&u->o);
				u->targetMove = 0;
				return 0;
			}

			u->o.flags.s.inTransport = true;

			Object_Script_Variable4_Clear(&u->o);
			u->targetMove = 0;

			u2 = Unit_Get_ByIndex(s->o.linkedID);

			/* Pickup the unit */
			u->o.linkedID = u2->o.index & 0xFF;
			s->o.linkedID = u2->o.linkedID;
			u2->o.linkedID = 0xFF;

			if (s->o.linkedID == 0xFF) Structure_SetState(s, STRUCTURE_STATE_IDLE);

			/* Check if the unit has a return-to position or try to find spice in case of a harvester */
			if (u2->targetLast.x != 0 || u2->targetLast.y != 0) {
				u->targetMove = Tools_Index_Encode(Tile_PackTile(u2->targetLast), IT_TILE);
			} else if (u2->o.type == UNIT_HARVESTER && Unit_GetHouseID(u2) != g_playerHouseID) {
				u->targetMove = Tools_Index_Encode(Map_SearchSpice(Tile_PackTile(u->o.position), 20), IT_TILE);
			}

			Unit_UpdateMap(2, u);

			return 1;
		}

		case IT_UNIT: {
			Unit *u2;
			Structure *s = NULL;
			PoolFindStruct find;
			int16 minDistance = 0;

			u2 = Tools_Index_GetUnit(u->targetMove);

			if (!u2->o.flags.s.allocated) return 0;

			find.houseID = Unit_GetHouseID(u);
			find.index   = 0xFFFF;
			find.type    = 0xFFFF;

			/* Find closest refinery / repair station */
			while (true) {
				Structure *s2;
				int16 distance;

				s2 = Structure_Find(&find);
				if (s2 == NULL) break;

				distance = Tile_GetDistanceRoundedUp(s2->o.position, u->o.position);

				if (u2->o.type == UNIT_HARVESTER) {
					if (s2->o.type != STRUCTURE_REFINERY || s2->state != STRUCTURE_STATE_IDLE || s2->o.script.variables[4] != 0) continue;
					if (minDistance != 0 && distance >= minDistance) break;
					minDistance = distance;
					s = s2;
					break;
				}

				if (s2->o.type != STRUCTURE_REPAIR || s2->state != STRUCTURE_STATE_IDLE || s2->o.script.variables[4] != 0) continue;

				if (minDistance != 0 && distance >= minDistance) continue;
				minDistance = distance;
				s = s2;
			}

			if (s == NULL) return 0;

			/* Deselect the unit as it is about to be picked up */
			if (u2 == g_unitSelected) Unit_Select(NULL);

			/* Pickup the unit */
			u->o.linkedID = u2->o.index & 0xFF;
			u->o.flags.s.inTransport = true;

			Unit_UpdateMap(0, u2);

			Unit_Hide(u2);

			/* Set where we are going to */
			Object_Script_Variable4_Link(Tools_Index_Encode(u->o.index, IT_UNIT), Tools_Index_Encode(s->o.index, IT_STRUCTURE));
			u->targetMove = u->o.script.variables[4];

			Unit_UpdateMap(2, u);

			if (u2->o.type != UNIT_HARVESTER) return 0;

			/* Check if we want to return to this spice field later */
			if (Map_SearchSpice(Tile_PackTile(u2->o.position), 2) == 0) {
				u2->targetPreLast.x = 0;
				u2->targetPreLast.y = 0;
				u2->targetLast.x    = 0;
				u2->targetLast.y    = 0;
			}

			return 0;
		}

		default: return 0;
	}
}

/**
 * Stop the Unit.
 *
 * Stack: *none*.
 *
 * @param script The script engine to operate on.
 * @return The value 0. Always.
 */
uint16 Script_Unit_Stop(ScriptEngine *script)
{
	Unit *u;

	VARIABLE_NOT_USED(script);

	u = g_scriptCurrentUnit;

	Unit_SetSpeed(u, 0);

	Unit_UpdateMap(2, u);

	return 0;
}

/**
 * Set the speed of a Unit.
 *
 * Stack: 1 - The new speed of the Unit.
 *
 * @param script The script engine to operate on.
 * @return The new speed; it might differ from the value given.
 */
uint16 Script_Unit_SetSpeed(ScriptEngine *script)
{
	Unit *u;
	uint16 speed;

	u = g_scriptCurrentUnit;
	speed = clamp(STACK_PEEK(1), 0, 255);

	if (!u->o.flags.s.byScenario) speed = speed * 192 / 256;

	Unit_SetSpeed(u, speed);

	return u->speed;
}

/**
 * Change the sprite (offset) of the unit.
 *
 * Stack: 1 - The new sprite offset.
 *
 * @param script The script engine to operate on.
 * @return The value 0. Always.
 */
uint16 Script_Unit_SetSprite(ScriptEngine *script)
{
	Unit *u;

	u = g_scriptCurrentUnit;

	u->spriteOffset = -(STACK_PEEK(1) & 0xFF);

	Unit_UpdateMap(2, u);

	return 0;
}

/**
 * Move the Unit to the target, and keep repeating this function till we
 *  arrived there. When closing in on the target it will slow down the Unit.
 * It is wise to only use this function on Carry-Alls.
 *
 * Stack: *none*.
 *
 * @param script The script engine to operate on.
 * @return 1 if arrived, 0 if still busy.
 */
uint16 Script_Unit_MoveToTarget(ScriptEngine *script)
{
	Unit *u;
	uint16 delay;
	CellStruct tile;
	uint16 distance;
	int8 orientation;
	int16 diff;

	u = g_scriptCurrentUnit;

	if (u->targetMove == 0) return 0;

	tile = Tools_Index_GetTile(u->targetMove);

	distance = Tile_GetDistance(u->o.position, tile);

	if ((int16)distance < 128) {
		Unit_SetSpeed(u, 0);

		u->o.position.x += clamp((int16)(tile.x - u->o.position.x), -16, 16);
		u->o.position.y += clamp((int16)(tile.y - u->o.position.y), -16, 16);

		Unit_UpdateMap(2, u);

		if ((int16)distance < 32) return 1;

		script->delay = 2;

		script->script--;
		return 0;
	}

	orientation = Tile_GetDirection(u->o.position, tile);

	Unit_SetOrientation(u, orientation, false, 0);

	diff = abs(orientation - u->orientation[0].current);
	if (diff > 128) diff = 256 - diff;

	Unit_SetSpeed(u, (max(min(distance / 8, 255), 25) * (255 - diff) + 128) / 256);

	delay = max((int16)distance / 1024, 1);

	Unit_UpdateMap(2, u);

	if (delay != 0) {
		script->delay = delay;

		script->script--;
	}

	return 0;
}

/**
 * Kill a unit. When it was a saboteur, expect a big explosion.
 *
 * Stack: *none*.
 *
 * @param script The script engine to operate on.
 * @return The value 0. Always.
 */
uint16 Script_Unit_Die(ScriptEngine *script)
{
	const UnitInfo *ui;
	Unit *u;

	VARIABLE_NOT_USED(script);

	u = g_scriptCurrentUnit;
	ui = &g_table_unitInfo[u->o.type];

	Unit_Remove(u);

	if (ui->movementType != MOVEMENT_WINGER) {
		uint16 credits;

		credits = max(ui->o.buildCredits / 100, 1);

		if (u->o.houseID == g_playerHouseID) {
			g_scenario.killedAllied++;
			g_scenario.score -= credits;
		} else {
			g_scenario.killedEnemy++;
			g_scenario.score += credits;
		}
	}

	Unit_HouseUnitCount_Remove(u);

	if (u->o.type != UNIT_SABOTEUR) return 0;

	Map_MakeExplosion(EXPLOSION_SABOTEUR_DEATH, u->o.position, 300, 0);
	return 0;
}

/**
 * Make an explosion at the coordinates of the unit.
 *  It does damage to the surrounding units based on the unit.
 *
 * Stack: 1 - Explosion type
 *
 * @param script The script engine to operate on.
 * @return The value 0. Always.
 */
uint16 Script_Unit_ExplosionSingle(ScriptEngine *script)
{
	Unit *u;

	u = g_scriptCurrentUnit;

	Map_MakeExplosion(STACK_PEEK(1), u->o.position, g_table_unitInfo[u->o.type].o.hitpoints, Tools_Index_Encode(u->o.index, IT_UNIT));
	return 0;
}

/**
 * Make 8 explosions: 1 at the unit, and 7 around him.
 * It does damage to the surrounding units with predefined damage, but
 *  anonymous.
 *
 * Stack: 1 - The radius of the 7 explosions.
 *
 * @param script The script engine to operate on.
 * @return The value 0. Always.
 */
uint16 Script_Unit_ExplosionMultiple(ScriptEngine *script)
{
	Unit *u;
	uint8 i;

	u = g_scriptCurrentUnit;

	Map_MakeExplosion(EXPLOSION_DEATH_HAND, u->o.position, Tools_RandomLCG_Range(25, 50), 0);

	for (i = 0; i < 7; i++) {
		Map_MakeExplosion(EXPLOSION_DEATH_HAND, Coord_Scatter(u->o.position, STACK_PEEK(1), false), Tools_RandomLCG_Range(75, 150), 0);
	}

	return 0;
}

/**
 * Makes the current unit fire a bullet (or eat its target).
 *
 * Stack: *none*.
 *
 * @param script The script engine to operate on.
 * @return The value 1 if the current unit fired/eat, 0 otherwise.
 */
uint16 Script_Unit_Fire(ScriptEngine *script)
{
	const UnitInfo *ui;
	Unit *u;
	uint16 target;
	UnitType typeID;
	uint16 distance;
	bool fireTwice;
	uint16 damage;

	u = g_scriptCurrentUnit;

	target = u->targetAttack;
	if (target == 0 || !Tools_Index_IsValid(target)) return 0;

	if (u->o.type != UNIT_SANDWORM && target == Tools_Index_Encode(Tile_PackTile(u->o.position), IT_TILE)) u->targetAttack = 0;

	if (u->targetAttack != target) {
		Unit_SetTarget(u, target);
		return 0;
	}

	ui = &g_table_unitInfo[u->o.type];

	if (u->o.type != UNIT_SANDWORM && u->orientation[ui->o.flags.hasTurret ? 1 : 0].speed != 0) return 0;

	if (Tools_Index_GetType(target) == IT_TILE && Object_GetByPackedTile(Tools_Index_GetPackedTile(target)) != NULL) Unit_SetTarget(u, target);

	if (u->fireDelay != 0) return 0;

	distance = Object_GetDistanceToEncoded(&u->o, target);

	if ((int16)(ui->fireDistance << 8) < (int16)distance) return 0;

	if (u->o.type != UNIT_SANDWORM && (Tools_Index_GetType(target) != IT_UNIT || g_table_unitInfo[Tools_Index_GetUnit(target)->o.type].movementType != MOVEMENT_WINGER)) {
		int16 diff = 0;
		int8 orientation;

		orientation = Tile_GetDirection(u->o.position, Tools_Index_GetTile(target));

		diff = abs(u->orientation[ui->o.flags.hasTurret ? 1 : 0].current - orientation);
		if (ui->movementType == MOVEMENT_WINGER) diff /= 8;

		if (diff >= 8) return 0;
	}

	damage = ui->damage;
	typeID = ui->bulletType;

	fireTwice = ui->flags.firesTwice && u->o.hitpoints > ui->o.hitpoints / 2;

	if ((u->o.type == UNIT_TROOPERS || u->o.type == UNIT_TROOPER) && (int16)distance > 512) typeID = UNIT_MISSILE_TROOPER;

	switch (typeID) {
		case UNIT_SANDWORM: {
			Unit *u2;

			Unit_UpdateMap(0, u);

			u2 = Tools_Index_GetUnit(target);

			if (u2 != NULL) {
				u2->o.script.variables[1] = 0xFFFF;
				Unit_RemovePlayer(u2);
				Unit_HouseUnitCount_Remove(u2);
				Unit_Remove(u2);
			}

			Map_MakeExplosion(ui->explosionType, u->o.position, 0, 0);

			Voice_PlayAtTile(63, u->o.position);

			Unit_UpdateMap(1, u);

			u->amount--;

			script->delay = 12;

			if ((int8)u->amount < 1) Unit_SetAction(u, ACTION_DIE);
		} break;

		case UNIT_MISSILE_TROOPER:
			damage -= damage / 4;
			/* FALL-THROUGH */

		case UNIT_MISSILE_ROCKET:
		case UNIT_MISSILE_TURRET:
		case UNIT_MISSILE_DEVIATOR:
		case UNIT_BULLET:
		case UNIT_SONIC_BLAST: {
			Unit *bullet;

			bullet = Unit_CreateBullet(u->o.position, typeID, Unit_GetHouseID(u), damage, target);

			if (bullet == NULL) return 0;

			bullet->originEncoded = Tools_Index_Encode(u->o.index, IT_UNIT);

			Voice_PlayAtTile(ui->bulletSound, u->o.position);

			Unit_Deviation_Decrease(u, 20);
		} break;

		default: break;
	}

	u->fireDelay = Tools_AdjustToGameSpeed(ui->fireDelay * 2, 1, 0xFFFF, true);

	if (fireTwice) {
		u->o.flags.s.fireTwiceFlip = !u->o.flags.s.fireTwiceFlip;
		if (u->o.flags.s.fireTwiceFlip) u->fireDelay = Tools_AdjustToGameSpeed(5, 1, 10, true);
	} else {
		u->o.flags.s.fireTwiceFlip = false;
	}

	u->fireDelay += Tools_Random_256() & 1;

	Unit_UpdateMap(2, u);

	return 1;
}

/**
 * Set the orientation of a unit.
 *
 * Stack: 1 - New orientation for unit.
 *
 * @param script The script engine to operate on.
 * @return The current orientation of the unit (it will move to the requested over time).
 */
uint16 Script_Unit_SetOrientation(ScriptEngine *script)
{
	Unit *u;

	u = g_scriptCurrentUnit;

	Unit_SetOrientation(u, (int8)STACK_PEEK(1), false, 0);

	return u->orientation[0].current;
}

/**
 * Rotate the unit to aim at the enemy.
 *
 * Stack: *none*.
 *
 * @param script The script engine to operate on.
 * @return 0 if the enemy is no longer there or if we are looking at him, 1 otherwise.
 */
uint16 Script_Unit_Rotate(ScriptEngine *script)
{
	const UnitInfo *ui;
	Unit *u;
	uint16 index;
	int8 current;
	CellStruct tile;
	int8 orientation;

	VARIABLE_NOT_USED(script);

	u = g_scriptCurrentUnit;
	ui = &g_table_unitInfo[u->o.type];

	if (ui->movementType != MOVEMENT_WINGER && (u->currentDestination.x != 0 || u->currentDestination.y != 0)) return 1;

	index = ui->o.flags.hasTurret ? 1 : 0;

	/* Check if we are already rotating */
	if (u->orientation[index].speed != 0) return 1;
	current = u->orientation[index].current;

	if (!Tools_Index_IsValid(u->targetAttack)) return 0;

	/* Check where we should rotate to */
	tile = Tools_Index_GetTile(u->targetAttack);
	orientation = Tile_GetDirection(u->o.position, tile);

	/* If we aren't already looking at it, rotate */
	if (orientation == current) return 0;
	Unit_SetOrientation(u, orientation, false, index);

	return 1;
}

/**
 * Get the direction to a tile or our current direction.
 *
 * Stack: 1 - An encoded tile to get the direction to.
 *
 * @param script The script engine to operate on.
 * @return The direction to the encoded tile if valid, otherwise our current orientation.
 */
uint16 Script_Unit_GetOrientation(ScriptEngine *script)
{
	Unit *u;
	uint16 encoded;

	u = g_scriptCurrentUnit;
	encoded = STACK_PEEK(1);

	if (Tools_Index_IsValid(encoded)) {
		CellStruct tile;

		tile = Tools_Index_GetTile(encoded);

		return Tile_GetDirection(u->o.position, tile);
	}

	return u->orientation[0].current;
}

/**
 * Set the new destination of the unit.
 *
 * Stack: 1 - An encoded index where to move to.
 *
 * @param script The script engine to operate on.
 * @return The value 0. Always.
 */
uint16 Script_Unit_SetDestination(ScriptEngine *script)
{
	Unit *u;
	uint16 encoded;

	u = g_scriptCurrentUnit;
	encoded = STACK_PEEK(1);

	if (encoded == 0 || !Tools_Index_IsValid(encoded)) {
		u->targetMove = 0;
		return 0;
	}

	if (u->o.type == UNIT_HARVESTER) {
		Structure *s;

		s = Tools_Index_GetStructure(encoded);
		if (s == NULL) {
			u->targetMove = encoded;
			u->Path[0] = 0xFF;
			return 0;
		}

		if (s->o.script.variables[4] != 0) return 0;
	}

	Unit_SetDestination(u, encoded);
	return 0;
}

/**
 * Set a new target, and rotate towards him if needed.
 *
 * Stack: 1 - An encoded tile of the unit/tile to target.
 *
 * @param script The script engine to operate on.
 * @return The new target.
 */
uint16 Script_Unit_SetTarget(ScriptEngine *script)
{
	Unit *u;
	uint16 target;
	CellStruct tile;
	int8 orientation;

	u = g_scriptCurrentUnit;

	target = STACK_PEEK(1);

	if (target == 0 || !Tools_Index_IsValid(target)) {
		u->targetAttack = 0;
		return 0;
	}

	tile = Tools_Index_GetTile(target);

	orientation = Tile_GetDirection(u->o.position, tile);

	u->targetAttack = target;
	if (!g_table_unitInfo[u->o.type].o.flags.hasTurret) {
		u->targetMove = target;
		Unit_SetOrientation(u, orientation, false, 0);
	}
	Unit_SetOrientation(u, orientation, false, 1);

	return u->targetAttack;
}

/**
 * Sets the action for the current unit.
 *
 * Stack: 1 - The action.
 *
 * @param script The script engine to operate on.
 * @return The value 0. Always.
 */
uint16 Script_Unit_SetAction(ScriptEngine *script)
{
	Unit *u;
	ActionType action;

	u = g_scriptCurrentUnit;

	action = STACK_PEEK(1);

	if (u->o.houseID == g_playerHouseID && action == ACTION_HARVEST && u->nextActionID != ACTION_INVALID) return 0;

	Unit_SetAction(u, action);

	return 0;
}

/**
 * Sets the action for the current unit to default.
 *
 * Stack: *none*.
 *
 * @param script The script engine to operate on.
 * @return The value 0. Always.
 */
uint16 Script_Unit_SetActionDefault(ScriptEngine *script)
{
	Unit *u;

	VARIABLE_NOT_USED(script);

	u = g_scriptCurrentUnit;

	Unit_SetAction(u, g_table_unitInfo[u->o.type].o.actionsPlayer[3]);

	return 0;
}

/**
 * Set the current destination of a Unit, bypassing any pathfinding.
 * It is wise to only use this function on Carry-Alls.
 *
 * Stack: 1 - An encoded tile, the destination.
 *
 * @param script The script engine to operate on.
 * @return The value 0. Always.
 */
uint16 Script_Unit_SetDestinationDirect(ScriptEngine *script)
{
	Unit *u;
	uint16 encoded;

	encoded = STACK_PEEK(1);

	if (!Tools_Index_IsValid(encoded)) return 0;

	u = g_scriptCurrentUnit;

	if ((u->currentDestination.x == 0 && u->currentDestination.y == 0) || g_table_unitInfo[u->o.type].flags.isNormalUnit) {
		u->currentDestination = Tools_Index_GetTile(encoded);
	}

	Unit_SetOrientation(u, Tile_GetDirection(u->o.position, u->currentDestination), false, 0);

	return 0;
}

/**
 * Get information about the unit, like hitpoints, current target, etc.
 *
 * Stack: 1 - Which information you would like.
 *
 * @param script The script engine to operate on.
 * @return The information you requested.
 */
uint16 Script_Unit_GetInfo(ScriptEngine *script)
{
	const UnitInfo *ui;
	Unit *u;

	u = g_scriptCurrentUnit;
	ui = &g_table_unitInfo[u->o.type];

	switch (STACK_PEEK(1)) {
		case 0x00: return u->o.hitpoints * 256 / ui->o.hitpoints;
		case 0x01: return Tools_Index_IsValid(u->targetMove) ? u->targetMove : 0;
		case 0x02: return ui->fireDistance << 8;
		case 0x03: return u->o.index;
		case 0x04: return u->orientation[0].current;
		case 0x05: return u->targetAttack;
		case 0x06:
			if (u->originEncoded == 0 || u->o.type == UNIT_HARVESTER) Unit_FindClosestRefinery(u);
			return u->originEncoded;
		case 0x07: return u->o.type;
		case 0x08: return Tools_Index_Encode(u->o.index, IT_UNIT);
		case 0x09: return u->movingSpeed;
		case 0x0A: return abs(u->orientation[0].target - u->orientation[0].current);
		case 0x0B: return (u->currentDestination.x == 0 && u->currentDestination.y == 0) ? 0 : 1;
		case 0x0C: return u->fireDelay == 0 ? 1 : 0;
		case 0x0D: return ui->flags.explodeOnDeath;
		case 0x0E: return Unit_GetHouseID(u);
		case 0x0F: return u->o.flags.s.byScenario ? 1 : 0;
		case 0x10: return u->orientation[ui->o.flags.hasTurret ? 1 : 0].current;
		case 0x11: return abs(u->orientation[ui->o.flags.hasTurret ? 1 : 0].target - u->orientation[ui->o.flags.hasTurret ? 1 : 0].current);
		case 0x12: return (ui->movementType & 0x40) == 0 ? 0 : 1;
		case 0x13: return (u->o.seenByHouses & (1 << g_playerHouseID)) == 0 ? 0 : 1;
		default:   return 0;
	}
}




//Path finding in D2 to RA is built on Crash and Turn path finding algorithm.
//In C&C it recieved various updates.
/*
Copy from info on it on http://web.archive.org/web/19991006064545/http://perplexed.com/GPMega/advanced/crashnturn.htm


This is probably the easiest one to implement and was the first one I implemented in my Prisoner of War project. This is an algorithm that evaluates the way one step at a time. Trying to move in a straight line to the endpoint direction and adjusting the rules as it goes. As most of you already know, this is not a good way of solving it. The unit will behave odd, choosing silly path's. Sometimes it will get locked up in a loop. Anyway, here is how I implemented it:

Try to move in a straight line to the endpoint. Always remember the previous tile coordinates. (One brain-cell).
If it collides with an obstacle (landscape or other unit) try the RIGHT_HAND rule. Rotating right until a free passage is found and then move to that tile.
Repeat 1-2 until the goal is reached or a collision with the previous tile occurs.
If we collided with our previous position (trying to go back to the last tile we came from) try changing to a LEFT_HAND rule instead. Then follow the same procedure above (using a LEFT_HAND rule instead of RIGHT_HAND).
The last point can be changed to say it should not be able to go back into it's previous tracks. This way it will continue using the RIGHT_HAND rule all the way.

The algorithm is not very good and gets easily locked up. Especially in the U-formed obstacle example.

The crash'n turn algorithm can be further enhanced to make the unit behave a bit more intelligent. I.e. look at the below example:


  ########
###########  A
###########
###########
###########
 #######

B

Our unit wants to go from A to B. We can easily see the best way will be to use a LEFT_HAND rule to reach the goal. Our initial algorithm will always use a RIGHT_HAND rule first. This will look silly, as it will take the other way around the obstacle.

By modifying the algorithm a bit we can decide which rule we should start using. Imagine yourself wanting to go past the obstacle. You would probably think something like: Lets follow the edge until we reach the corner and then move towards the endpoint B. Okay, you naturally chose a LEFT_HAND rule.

This is what we do: Alternate between RIGHT_HAND and LEFT_HAND scanning on the first obstacle you meet. The first one that gives a free path tells you which rule to use. This way the routine will probably choose a smarter rule to start with based on it's angle to the destination point. It will of course fail on difficult terrain and the same loop problems are quite evident in this implementation also.

Here is another example, it will choose the LEFT_HAND rule because it saw that the left edge was free. It will choose the same path from B to A also:


   ***********
A**###########**B
   ###########
   ###########
   ###########

On the example below it will fail to behave intelligently because the scanning found the right to be the first free path:


    ######
A***######   B
   *######   *
  **######   *
 *###########*
 *###########*
 **#######**
   *******  
*/

/**
 * Get the Score to enter this tile from a direction.
 *
 * @param packed The packed tile.
 * @param direction The direction we move on this tile.
 * @return 256 if tile is not accessable, or a Score for entering otherwise.
 */
static int16 Passable_Cell(uint16 packed, uint8 orient8)
{
	int16 res;
	Unit *u;

	if (g_scriptCurrentUnit == NULL) return 0;

	u = g_scriptCurrentUnit;

	if (g_dune2_enhanced) {
		res = Unit_GetTileEnterScore(u, packed, orient8);
	} else {
		res = Unit_GetTileEnterScore(u, packed, orient8 << 5);
	}

	if (res == -1) res = 256;

	return res;
}

/**
 * Smoothen the Path found by the pathfinder.
 * @param data The found Path to smoothen.
 */
static void Optimize_Moves(PathType *path)
{
	static const _trans[8] = {
		FACING_NORTH,
		FACING_NORTH,
		FACING_NORTH_EAST,
		FACING_EAST,
		FACING_SOUTH_EAST,
		FACING_FIXUP_MARK,
		FACING_NONE,
		FACING_NORTH
	};


	uint16 packed;
	uint8 *bufferFrom;
	uint8 *bufferTo;

	path->Moves[path->Length] = FACING_NONE;
	packed = path->StartCell;

	if (path->Length > 1) {
		bufferTo = path->Moves + 1;

		while (*bufferTo != FACING_NONE) {
			int8 direction;
			uint8 dir;

			bufferFrom = bufferTo - 1;

			while (*bufferFrom == FACING_FIXUP_MARK && bufferFrom != path->Moves) bufferFrom--;

			if (*bufferFrom == FACING_FIXUP_MARK) {
				bufferTo++;
				continue;
			}

			direction = (*bufferTo - *bufferFrom) & 0x7;
			direction = _trans[direction];

			/* The directions are opposite of each other, so they can both be removed */
			if (direction == 3) {
				*bufferFrom = FACING_FIXUP_MARK;
				*bufferTo   = FACING_FIXUP_MARK;

				bufferTo++;
				continue;
			}

			/* The directions are close to each other, so follow */
			if (direction == 0) {
				packed += AdjacentCell[*bufferFrom];
				bufferTo++;
				continue;
			}

			if ((*bufferFrom & 0x1) != 0) {
				dir = (*bufferFrom + (direction < 0 ? -1 : 1)) & 0x7;

				/* If we go 45 degrees with 90 degree difference, we can also go straight */
				if (abs(direction) == 1) {
					if (Passable_Cell(packed + AdjacentCell[dir], dir) <= 255) {
						*bufferTo = dir;
						*bufferFrom = dir;
					}
					packed += AdjacentCell[*bufferFrom];
					bufferTo++;
					continue;
				}
			} else {
				dir = (*bufferFrom + direction) & 0x7;
			}

			/* In these cases we can do with 1 direction change less, so remove one */
			*bufferTo = dir;
			*bufferFrom = FACING_FIXUP_MARK;

			/* Walk back one tile */
			while (*bufferFrom == FACING_FIXUP_MARK && path->Moves != bufferFrom) bufferFrom--;
			if (*bufferFrom != FACING_FIXUP_MARK) {
				packed += AdjacentCell[(*bufferFrom + 4) & 0x7];
			} else {
				packed = path->StartCell;
			}
		}
	}

	bufferFrom = path->Moves;
	bufferTo   = path->Moves;
	packed     = path->StartCell;
	path->Score = 0;
	path->Length = 0;

	/* Build the new improved Path, without gaps */
	for (; *bufferTo != FACING_NONE; bufferTo++) {
		if (*bufferTo == FACING_FIXUP_MARK) continue;

		packed += AdjacentCell[*bufferTo];
		path->Score += Passable_Cell(packed, *bufferTo);
		path->Length++;
		*bufferFrom++ = *bufferTo;
	}

	path->Length++;
	*bufferFrom = FACING_NONE;
}

/**
 * Try to connect two tiles (packedDst and data->packed) via a simplistic algorithm.
 * @param packedDst The tile to try to get to.
 * @param data Information about the found Path, and the start point.
 * @param searchDirection The search direction (1 for clockwise, -1 for counterclockwise).
 * @param directionStart The direction to start looking at.
 * @return True if a Path was found.
 */
static bool Follow_Edge(uint16 packedDst, PathType *data, int8 searchDirection, uint8 directionStart)
{
	uint16 packedNext;
	uint16 packedCur;
	uint8 *buffer;
	uint16 bufferSize;

	packedCur  = data->StartCell;
	buffer     = data->Moves;
	bufferSize = 0;

	while (bufferSize < 100) {
		uint8 direction = directionStart;

		while (true) {
			/* Look around us, first in the start direction, for a valid tile */
			direction = (direction + searchDirection) & 0x7;

			/* In case we are directly looking at our destination tile, we are pretty much done */
			if ((direction & 0x1) != 0 && (packedCur + AdjacentCell[(direction + searchDirection) & 0x7]) == packedDst) {
				direction = (direction + searchDirection) & 0x7;
				packedNext = packedCur + AdjacentCell[direction];
				break;
			} else {
				/* If we are back to our start direction, we didn't find a Path */
				if (direction == directionStart) return false;

				/* See if the tile next to us is a valid position */
				packedNext = packedCur + AdjacentCell[direction];
				if (Passable_Cell(packedNext, direction) <= 255) break;
			}
		}

		*buffer++ = direction;
		bufferSize++;

		/* If we found the destination, smooth the Path and we are done */
		if (packedNext == packedDst) {
			*buffer = 0xFF;
			data->Length = bufferSize;
			Optimize_Moves(data);
			data->Length--;
			return true;
		}

		/* If we return to our start tile, we didn't find a Path */
		if (data->StartCell == packedNext) return false;

		/* Now look at the next tile, starting 3 directions back */
		directionStart = (direction - searchDirection * 3) & 0x7;
		packedCur = packedNext;
	}

	/* We ran out of search space and didn't find a Path */
	return false;
}

/**
 * Try to find a path between two points.
 *
 * @param packedSrc The start point.
 * @param packedDst The end point.
 * @param buffer The buffer to store the Path in.
 * @param bufferSize The size of the buffer.
 * @return A struct with information about the found Path.
 */
static PathType Find_Path(uint16 packedSrc, uint16 packedDst, void *buffer, int16 bufferSize)
{
	uint16 packedCur;
	PathType res;

	res.StartCell = packedSrc;
	res.Score     = 0;
	res.Length = 0;
	res.Moves = buffer;

	res.Moves[0] = 0xFF;

	bufferSize--;

	packedCur = packedSrc;
	while (res.Length < bufferSize) {
		uint8  direction;
		uint16 packedNext;
		int16  Score;

		if (packedCur == packedDst) break;

		/* Try going directly to the destination tile */
		direction = Tile_GetDirectionPacked(packedCur, packedDst) / 32;
		packedNext = packedCur + AdjacentCell[direction];

		/* Check for valid movement towards the tile */
		Score = Passable_Cell(packedNext, direction);
		if (Score <= 255) {
			res.Moves[res.Length++] = direction;
			res.Score += Score;
		} else {
			uint8 dir;
			bool foundCounterclockwise = false;
			bool foundClockwise = false;
			int16 Length;
			PathType Paths[2];
			uint8 PathsBuffer[2][102];
			PathType *bestPath;

			while (true) {
				if (packedNext == packedDst) break;

				/* Find the first valid tile on the (direct) Path. */
				dir = Tile_GetDirectionPacked(packedNext, packedDst) / 32;
				packedNext += AdjacentCell[dir];
				if (Passable_Cell(packedNext, dir) > 255) continue;

				/* Try to find a connection between our last valid tile and the new valid tile */
				Paths[1].StartCell = packedCur;
				Paths[1].Score     = 0;
				Paths[1].Length = 0;
				Paths[1].Moves = PathsBuffer[0];
				foundCounterclockwise = Follow_Edge(packedNext, &Paths[1], -1, direction);

				Paths[0].StartCell = packedCur;
				Paths[0].Score     = 0;
				Paths[0].Length = 0;
				Paths[0].Moves = PathsBuffer[1];
				foundClockwise = Follow_Edge(packedNext, &Paths[0], 1, direction);

				if (foundCounterclockwise || foundClockwise) break;

				do {
					if (packedNext == packedDst) break;

					dir = Tile_GetDirectionPacked(packedNext, packedDst) / 32;
					packedNext += AdjacentCell[dir];
				} while (Passable_Cell(packedNext, dir) <= 255);
			}

			if (foundCounterclockwise || foundClockwise) {
				/* Find the best (partial) Path */
				if (!foundClockwise) {
					bestPath = &Paths[1];
				} else if (!foundCounterclockwise) {
					bestPath = &Paths[0];
				} else {
					bestPath = &Paths[Paths[1].Score < Paths[0].Score ? 1 : 0];
				}

				/* Calculate how much more we can copy into our own buffer */
				Length = min(bufferSize - res.Length, bestPath->Length);
				if (Length <= 0) break;

				/* Copy the rest into our own buffer */
				memcpy(&res.Moves[res.Length], bestPath->Moves, Length);
				res.Length += Length;
				res.Score     += bestPath->Score;
			} else {
				/* Means we didn't find a Path. packedNext is now equal to packedDst */
				break;
			}
		}

		packedCur = packedNext;
	}

	if (res.Length < bufferSize) res.Moves[res.Length++] = 0xFF;

	Optimize_Moves(&res);

	return res;
}

/**
 * Calculate the Path to a tile.
 *
 * Stack: 1 - An encoded tile to calculate the Path to.
 *
 * @param script The script engine to operate on.
 * @return 0 if we arrived on location, 1 otherwise.
 */
uint16 Script_Unit_CalculatePath(ScriptEngine *script)
{
	Unit *u;
	uint16 encoded;
	uint16 packedSrc;
	uint16 packedDst;

	u = g_scriptCurrentUnit;
	encoded = STACK_PEEK(1);

	if (u->currentDestination.x != 0 || u->currentDestination.y != 0 || !Tools_Index_IsValid(encoded)) return 1;

	packedSrc = Tile_PackTile(u->o.position);
	packedDst = Tools_Index_GetPackedTile(encoded);

	if (packedDst == packedSrc) {
		u->Path[0] = 0xFF;
		u->targetMove = 0;
		return 0;
	}

	if (u->Path[0] == 0xFF) {
		PathType res;
		uint8 Moves[42];

		res = Find_Path(packedSrc, packedDst, Moves, 40);

		memcpy(u->Path, res.Moves, min(res.Length, 14));

		if (u->Path[0] == 0xFF) {
			u->targetMove = 0;
			if (u->o.type == UNIT_SANDWORM) {
				script->delay = 720;
			}
		}
	} else {
		uint16 distance;

		distance = Tile_GetDistancePacked(packedDst, packedSrc);
		if (distance < 14) u->Path[distance] = 0xFF;
	}

	if (u->Path[0] == 0xFF) return 1;

	if (u->orientation[0].current != (int8)(u->Path[0] * 32)) {
		Unit_SetOrientation(u, (int8)(u->Path[0] * 32), false, 0);
		return 1;
	}

	if (!Unit_StartMovement(u)) {
		u->Path[0] = 0xFF;
		return 0;
	}

	memmove(&u->Path[0], &u->Path[1], 13);
	u->Path[13] = 0xFF;
	return 1;
}

/**
 * Move the unit to the first available structure it can find of the required
 *  type.
 *
 * Stack: 1 - Type of structure.
 *
 * @param script The script engine to operate on.
 * @return An encoded structure index.
 */
uint16 Script_Unit_MoveToStructure(ScriptEngine *script)
{
	Unit *u;
	PoolFindStruct find;

	u = g_scriptCurrentUnit;

	if (u->o.linkedID != 0xFF) {
		Structure *s;

		s = Tools_Index_GetStructure(Unit_Get_ByIndex(u->o.linkedID)->originEncoded);

		if (s != NULL && s->state == STRUCTURE_STATE_IDLE && s->o.script.variables[4] == 0) {
			uint16 encoded;

			encoded = Tools_Index_Encode(s->o.index, IT_STRUCTURE);

			Object_Script_Variable4_Link(Tools_Index_Encode(u->o.index, IT_UNIT), encoded);

			u->targetMove = u->o.script.variables[4];

			return encoded;
		}
	}

	find.houseID = Unit_GetHouseID(u);
	find.index   = 0xFFFF;
	find.type    = STACK_PEEK(1);

	while (true) {
		Structure *s;
		uint16 encoded;

		s = Structure_Find(&find);
		if (s == NULL) break;

		if (s->state != STRUCTURE_STATE_IDLE) continue;
		if (s->o.script.variables[4] != 0) continue;

		encoded = Tools_Index_Encode(s->o.index, IT_STRUCTURE);

		Object_Script_Variable4_Link(Tools_Index_Encode(u->o.index, IT_UNIT), encoded);

		u->targetMove = encoded;

		return encoded;
	}

	return 0;
}

/**
 * Gets the amount of the unit linked to current unit, or current unit if not linked.
 *
 * Stack: *none*.
 *
 * @param script The script engine to operate on.
 * @return The amount.
 */
uint16 Script_Unit_GetAmount(ScriptEngine *script)
{
	Unit *u;

	VARIABLE_NOT_USED(script);

	u = g_scriptCurrentUnit;

	if (u->o.linkedID == 0xFF) return u->amount;

	return Unit_Get_ByIndex(u->o.linkedID)->amount;
}

/**
 * Checks if the current unit is in transport.
 *
 * Stack: *none*.
 *
 * @param script The script engine to operate on.
 * @return True if the current unit is in transport.
 */
uint16 Script_Unit_IsInTransport(ScriptEngine *script)
{
	Unit *u;

	VARIABLE_NOT_USED(script);

	u = g_scriptCurrentUnit;

	return u->o.flags.s.inTransport ? 1 : 0;
}

/**
 * Start the animation on the current tile.
 *
 * Stack: *none*.
 *
 * @param script The script engine to operate on.
 * @return The value 1. Always.
 */
uint16 Script_Unit_StartAnimation(ScriptEngine *script)
{
	Unit *u;
	uint16 animationUnitID;
	uint16 position;

	VARIABLE_NOT_USED(script);

	u = g_scriptCurrentUnit;

	position = Tile_PackTile(Tile_Center(u->o.position));
	Animation_Stop_ByTile(position);

	animationUnitID = g_table_landscapeInfo[Map_GetLandType(Tile_PackTile(u->o.position))].isSand ? 0 : 1;
	if (u->o.script.variables[1] == 1) animationUnitID += 2;

	g_map[position].houseID = Unit_GetHouseID(u);

	assert(animationUnitID < 4);
	if (g_table_unitInfo[u->o.type].displayMode == DISPLAYMODE_INFANTRY_3_FRAMES) {
		Animation_Start(g_table_animation_unitScript1[animationUnitID], u->o.position, 0, Unit_GetHouseID(u), 4);
	} else {
		Animation_Start(g_table_animation_unitScript2[animationUnitID], u->o.position, 0, Unit_GetHouseID(u), 4);
	}

	return 1;
}

/**
 * Call a UnitType and make it go to the current unit. In general, type should
 *  be a Carry-All for this to make any sense.
 *
 * Stack: 1 - An unit type.
 *
 * @param script The script engine to operate on.
 * @return An encoded unit index.
 */
uint16 Script_Unit_CallUnitByType(ScriptEngine *script)
{
	Unit *u;
	Unit *u2;
	uint16 encoded;
	uint16 encoded2;

	u = g_scriptCurrentUnit;

	if (u->o.script.variables[4] != 0) return u->o.script.variables[4];
	if (!g_table_unitInfo[u->o.type].o.flags.canBePickedUp || u->deviated != 0) return 0;

	encoded = Tools_Index_Encode(u->o.index, IT_UNIT);

	u2 = Unit_CallUnitByType(STACK_PEEK(1), Unit_GetHouseID(u), encoded, false);
	if (u2 == NULL) return 0;

	encoded2 = Tools_Index_Encode(u2->o.index, IT_UNIT);

	Object_Script_Variable4_Link(encoded, encoded2);
	u2->targetMove = encoded;

	return encoded2;
}

/**
 * Unknown function 2552.
 *
 * Stack: *none*.
 *
 * @param script The script engine to operate on.
 * @return The value 0. Always.
 */
uint16 Script_Unit_Unknown2552(ScriptEngine *script)
{
	Unit *u;
	Unit *u2;

	VARIABLE_NOT_USED(script);

	u = g_scriptCurrentUnit;
	if (u->o.script.variables[4] == 0) return 0;

	u2 = Tools_Index_GetUnit(u->o.script.variables[4]);
	if (u2 == NULL || u2->o.type != UNIT_CARRYALL) return 0;

	Object_Script_Variable4_Clear(&u->o);
	u2->targetMove = 0;

	return 0;
}

/**
 * Finds a structure.
 *
 * Stack: 1 - A structure type.
 *
 * @param script The script engine to operate on.
 * @return An encoded structure index, or 0 if none found.
 */
uint16 Script_Unit_FindStructure(ScriptEngine *script)
{
	Unit *u;
	PoolFindStruct find;

	u = g_scriptCurrentUnit;

	find.houseID = Unit_GetHouseID(u);
	find.index   = 0xFFFF;
	find.type    = STACK_PEEK(1);

	while (true) {
		Structure *s;

		s = Structure_Find(&find);
		if (s == NULL) break;
		if (s->state != STRUCTURE_STATE_IDLE) continue;
		if (s->o.linkedID != 0xFF) continue;
		if (s->o.script.variables[4] != 0) continue;

		return Tools_Index_Encode(s->o.index, IT_STRUCTURE);
	}

	return 0;
}

/**
 * Displays the "XXX XXX destroyed." message for the current unit.
 *
 * Stack: *none*.
 *
 * @param script The script engine to operate on.
 * @return The value 0. Always.
 */
uint16 Script_Unit_DisplayDestroyedText(ScriptEngine *script)
{
	const UnitInfo *ui;
	Unit *u;

	VARIABLE_NOT_USED(script);

	u = g_scriptCurrentUnit;
	ui = &g_table_unitInfo[u->o.type];

	if (g_config.Language == LANGUAGE_FRENCH) {
		GUI_DisplayText(String_Get_ByIndex(STR_S_S_DESTROYED), 0, String_Get_ByIndex(ui->o.stringID_abbrev), g_table_houseInfo[Unit_GetHouseID(u)].name);
	} else {
		GUI_DisplayText(String_Get_ByIndex(STR_S_S_DESTROYED), 0, g_table_houseInfo[Unit_GetHouseID(u)].name, String_Get_ByIndex(ui->o.stringID_abbrev));
	}

	return 0;
}

/**
 * Removes fog around the current unit.
 *
 * Stack: *none*.
 *
 * @param script The script engine to operate on.
 * @return The value 0. Always.
 */
uint16 Script_Unit_RemoveFog(ScriptEngine *script)
{
	Unit *u;

	VARIABLE_NOT_USED(script);

	u = g_scriptCurrentUnit;
	Unit_RemoveFog(u);
	return 0;
}

/**
 * Make the current unit harvest spice.
 *
 * Stack: *none*.
 *
 * @param script The script engine to operate on.
 * @return ??.
 */
uint16 Script_Unit_Harvest(ScriptEngine *script)
{
	Unit *u;
	uint16 packed;
	uint16 type;

	VARIABLE_NOT_USED(script);

	u = g_scriptCurrentUnit;

	if (u->o.type != UNIT_HARVESTER) return 0;
	if (u->amount >= 100) return 0;

	packed = Tile_PackTile(u->o.position);

	type = Map_GetLandType(packed);
	if (type != LST_SPICE && type != LST_THICK_SPICE) return 0;

	u->amount += Tools_Random_256() & 1;
	u->o.flags.s.inTransport = true;

	Unit_UpdateMap(2, u);

	if (u->amount > 100) u->amount = 100;

	if ((Tools_Random_256() & 0x1F) != 0) return 1;

	Map_ChangeSpiceAmount(packed, -1);

	return 0;
}

/**
 * Check if the given tile is a valid destination. In case of for example
 *  a carry-all it checks if the unit carrying can be placed on destination.
 * In case of structures, it checks if you can walk into it.
 *
 * Stack: 1 - An encoded tile, indicating the destination.
 *
 * @param script The script engine to operate on.
 * @return ??.
 */
uint16 Script_Unit_IsValidDestination(ScriptEngine *script)
{
	Unit *u;
	Unit *u2;
	uint16 encoded;
	uint16 index;

	u = g_scriptCurrentUnit;
	encoded = STACK_PEEK(1);
	index = Tools_Index_Decode(encoded);

	switch (Tools_Index_GetType(encoded)) {
		case IT_TILE:
			if (!Map_IsValidPosition(index)) return 1;
			if (u->o.linkedID == 0xFF) return 1;
			u2 = Unit_Get_ByIndex(u->o.linkedID);
			u2->o.position = Tools_Index_GetTile(encoded);
			if (!Unit_IsTileOccupied(u2)) return 0;
			u2->o.position.x = 0xFFFF;
			u2->o.position.y = 0xFFFF;
			return 1;

		case IT_STRUCTURE: {
			Structure *s;

			s = Structure_Get_ByIndex(index);
			if (s->o.houseID == Unit_GetHouseID(u)) return 0;
			if (u->o.linkedID == 0xFF) return 1;
			u2 = Unit_Get_ByIndex(u->o.linkedID);
			return Unit_IsValidMovementIntoStructure(u2, s) != 0 ? 1 : 0;
		}

		default: return 1;
	}
}

/**
 * Get a random tile around the Unit.
 *
 * Stack: 1 - An encoded index of a tile, completely ignored, as long as it is a tile.
 *
 * @param script The script engine to operate on.
 * @return An encoded tile, or 0.
 */
uint16 Script_Unit_GetRandomTile(ScriptEngine *script)
{
	Unit *u;
	CellStruct tile;

	u = g_scriptCurrentUnit;

	if (Tools_Index_GetType(STACK_PEEK(1)) != IT_TILE) return 0;

	tile = Coord_Scatter(u->o.position, 80, true);

	return Tools_Index_Encode(Tile_PackTile(tile), IT_TILE);
}

/**
 * Perform a random action when we are sitting idle, like rotating around.
 *
 * Stack: *none*.
 *
 * @param script The script engine to operate on.
 * @return The value 0. Always.
 */
uint16 Script_Unit_IdleAction(ScriptEngine *script)
{
	Unit *u;
	uint16 random;
	uint16 movementType;
	uint16 i;

	VARIABLE_NOT_USED(script);

	u = g_scriptCurrentUnit;

	random = Tools_RandomLCG_Range(0, 10);
	movementType = g_table_unitInfo[u->o.type].movementType;

	if (movementType != MOVEMENT_FOOT && movementType != MOVEMENT_TRACKED && movementType != MOVEMENT_WHEELED) return 0;

	if (movementType == MOVEMENT_FOOT && random > 8) {
		u->spriteOffset = Tools_Random_256() & 0x3F;
		Unit_UpdateMap(2, u);
	}

	if (random > 2) return 0;

	/* Ensure the order of Tools_Random_256() calls. */
	i = (Tools_Random_256() & 1) == 0 ? 1 : 0;
	Unit_SetOrientation(u, Tools_Random_256(), false, i);

	return 0;
}

/**
 * Makes the current unit to go to the closest structure of the given type.
 *
 * Stack: 1 - The type of the structure.
 *
 * @param script The script engine to operate on.
 * @return The value 1 if and only if a structure has been found.
 */
uint16 Script_Unit_GoToClosestStructure(ScriptEngine *script)
{
	Unit *u;
	Structure *s = NULL;
	PoolFindStruct find;
	uint16 distanceMin =0;

	u = g_scriptCurrentUnit;

	find.houseID = Unit_GetHouseID(u);
	find.index   = 0xFFFF;
	find.type    = STACK_PEEK(1);

	while (true) {
		Structure *s2;
		uint16 distance;

		s2 = Structure_Find(&find);

		if (s2 == NULL) break;
		if (s2->state != STRUCTURE_STATE_IDLE) continue;
		if (s2->o.linkedID != 0xFF) continue;
		if (s2->o.script.variables[4] != 0) continue;

		distance = Tile_GetDistanceRoundedUp(s2->o.position, u->o.position);

		if (distance >= distanceMin && distanceMin != 0) continue;

		distanceMin = distance;
		s = s2;
	}

	if (s == NULL) return 0;

	Unit_SetAction(u, ACTION_MOVE);
	Unit_SetDestination(u, Tools_Index_Encode(s->o.index, IT_STRUCTURE));

	return 1;
}

/**
 * Transform an MCV into Construction Yard.
 *
 * Stack: *none*.
 *
 * @param script The script engine to operate on.
 * @return 1 if and only if the transformation succeeded.
 */
uint16 Script_Unit_MCVDeploy(ScriptEngine *script)
{
	Unit *u;
	Structure *s = NULL;
	uint16 i;

	VARIABLE_NOT_USED(script);

	u = g_scriptCurrentUnit;

	Unit_UpdateMap(0, u);

	for (i = 0; i < 4; i++) {
		static const int8 offsets[4] = { 0, -1, -64, -65 };

		s = Structure_Create(STRUCTURE_INDEX_INVALID, STRUCTURE_CONSTRUCTION_YARD, Unit_GetHouseID(u), Tile_PackTile(u->o.position) + offsets[i]);

		if (s != NULL) {
			Unit_Remove(u);
			return 1;
		}
	}

	if (Unit_GetHouseID(u) == g_playerHouseID) {
		GUI_DisplayText(String_Get_ByIndex(STR_UNIT_IS_UNABLE_TO_DEPLOY_HERE), 0);
	}

	Unit_UpdateMap(1, u);

	return 0;
}

/**
 * Get the best target around you. Only considers units on sand.
 *
 * Stack: *none*.
 *
 * @param script The script engine to operate on.
 * @return An encoded unit index, or 0.
 */
uint16 Script_Unit_Sandworm_GetBestTarget(ScriptEngine *script)
{
	Unit *u;
	Unit *u2;

	VARIABLE_NOT_USED(script);

	u = g_scriptCurrentUnit;

	u2 = Unit_Sandworm_FindBestTarget(u);
	if (u2 == NULL) return 0;

	return Tools_Index_Encode(u2->o.index, IT_UNIT);
}

/**
 * Unknown function 2BD5.
 *
 * Stack: *none*.
 *
 * @param script The script engine to operate on.
 * @return ??.
 */
uint16 Script_Unit_Unknown2BD5(ScriptEngine *script)
{
	Unit *u;

	VARIABLE_NOT_USED(script);

	u = g_scriptCurrentUnit;

	switch (Tools_Index_GetType(u->o.script.variables[4])) {
		case IT_UNIT: {
			Unit *u2;

			u2 = Tools_Index_GetUnit(u->o.script.variables[4]);

			if (Tools_Index_Encode(u->o.index, IT_UNIT) == u2->o.script.variables[4] && u2->o.houseID == u->o.houseID) return 1;

			u2->targetMove = 0;
		} break;

		case IT_STRUCTURE: {
			Structure *s;

			s = Tools_Index_GetStructure(u->o.script.variables[4]);
			if (Tools_Index_Encode(u->o.index, IT_UNIT) == s->o.script.variables[4] && s->o.houseID == u->o.houseID) return 1;
		} break;

		default: break;
	}

	Object_Script_Variable4_Clear(&u->o);
	return 0;
}

/**
 * Blink the unit for 32 ticks.
 *
 * Stack: *none*.
 *
 * @param script The script engine to operate on.
 * @return The value 0. Always.
 */
uint16 Script_Unit_Blink(ScriptEngine *script)
{
	Unit *u;

	VARIABLE_NOT_USED(script);

	u = g_scriptCurrentUnit;
	u->blinkCounter = 32;
	return 0;
}
