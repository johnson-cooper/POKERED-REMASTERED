#pragma once
// Pokered overworld block IDs — index into s_metatiles[] in tileset_overworld.c.
// Values match original pokered block IDs so data ports directly.

// Grass / ground
#define MT_GRASS_STEP  0x00  // grass with entrance step tiles
#define MT_GRASS       0x01  // plain grass
#define MT_TREE_CORNER 0x08  // tree bush at building corner
#define MT_DIRT        0x10  // open dirt / ground path

// Paths and roads
#define MT_PATH_WALK   0x0A  // narrow walkway/path transition
#define MT_WALKWAY_L   0x1D  // path segment left half
#define MT_WALKWAY_R   0x1E  // path segment right half
#define MT_PATH_CORNER 0x31  // path corner piece
#define MT_ROAD_H      0x74  // horizontal town road
#define MT_ROAD_CORNER 0x77  // road corner with grass

// Buildings
#define MT_BLDG_TOP_L  0x38  // building top-left (roof tiles)
#define MT_BLDG_TOP_R  0x39  // building top-right
#define MT_BLDG_DOOR   0x3A  // building door / entrance step
#define MT_BLDG_WALL_L 0x3C  // building lower-left wall
#define MT_BLDG_WALL_R 0x3D  // building lower-right wall
#define MT_BLDG_LEFT   0x0C  // narrow building left column
#define MT_BLDG_MID    0x0D  // narrow building middle column
#define MT_BLDG_RIGHT  0x0E  // narrow building right / above-door
#define MT_BLDG_FRONT_L 0x72 // Oak's Lab front-left beige facade
#define MT_BLDG_FRONT_R 0x73 // Oak's Lab front-right beige facade

// Signs / objects
#define MT_SIGN        0x56  // sign post

// Route south boundary
#define MT_ROUTE_B     0x61  // route boundary straight
#define MT_ROUTE_BL    0x64  // route boundary bottom-left
#define MT_ROUTE_BR    0x65  // route boundary bottom-right

// Map borders (tree/bush walls)
#define MT_BORDER_TL   0x0B  // top-left corner border
#define MT_BORDER_L    0x4E  // left edge border
#define MT_BORDER_R    0x4D  // right edge border
#define MT_BORDER_TR   0x4F  // top-right corner border
#define MT_BORDER_BL   0x50  // bottom-left corner border
#define MT_TREE_TOP    0x52  // tree row along north edge
