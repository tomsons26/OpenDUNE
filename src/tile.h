/** @file src/tile.h %Tile definitions. */

#ifndef TILE_H
#define TILE_H

extern bool Tile_IsValid(CellStruct tile);
extern uint16 Tile_GetX(CellStruct tile);
extern uint16 Tile_GetY(CellStruct tile);
extern uint32 Tile_GetXY(CellStruct tile);
extern uint8 Tile_GetPosX(CellStruct tile);
extern uint8 Tile_GetPosY(CellStruct tile);
extern CellStruct Tile_MakeXY(uint16 x, uint16 y);
extern uint16 Tile_PackTile(CellStruct tile);
extern uint16 Tile_PackXY(uint16 x, uint16 y);
extern CellStruct Tile_UnpackTile(uint16 packed);
extern uint8 Tile_GetPackedX(uint16 packed);
extern uint8 Tile_GetPackedY(uint16 packed);
extern bool Tile_IsOutOfMap(uint16 packed);
extern uint16 Tile_GetDistance(CellStruct from, CellStruct to);
extern uint16 Tile_GetDistancePacked(uint16 packed_from, uint16 packed_to);
extern uint16 Tile_GetDistanceRoundedUp(CellStruct from, CellStruct to);
extern CellStruct Tile_AddTileDiff(CellStruct from, CellStruct diff);
extern CellStruct Tile_Center(CellStruct tile);
extern void Sight_From(CellStruct tile, uint16 radius);
extern uint16 Tile_GetTileInDirectionOf(uint16 packed_from, uint16 packed_to);
extern uint8 Tile_GetDirectionPacked(uint16 packed_from, uint16 packed_to);
extern CellStruct Coord_Move(CellStruct tile, int16 orientation, uint16 distance);
extern CellStruct Coord_Scatter(CellStruct tile, uint16 distance, bool center);
extern int8 Tile_GetDirection(CellStruct from, CellStruct to);
extern CellStruct Tile_MoveByOrientation(CellStruct position, uint8 orientation);

extern uint8 Direction_To_Facing(uint8 orientation);
extern uint8 Orientation_Orientation256ToOrientation16(uint8 orientation);

#endif /* TILE_H */
