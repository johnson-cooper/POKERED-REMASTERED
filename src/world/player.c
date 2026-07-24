#include "world.h"
#include "input.h"
#include "dialog.h"
#include "script.h"
#include "audio.h"
#include "route1.h"
#include "map_ids.h"
#include "flags.h"

#define STEP_PIXELS  16
#define STEP_FRAMES  16

#define PLAYER_SCREEN_X  112
#define PLAYER_SCREEN_Y   72

static const s8 DIR_DX[4] = {  0,  0, -1,  1 };
static const s8 DIR_DY[4] = {  1, -1,  0,  0 };

// NPC coordinates are the same logical subtile coordinates used by player
// movement. Treat every visible NPC as a solid tile so the player cannot walk
// through people or scripted objects. The interaction code still allows the
// player to stand next to an NPC and press A.
static bool8 player_tile_occupied_by_npc(s32 x, s32 y) {
    for (u8 i = 0; i < g_world.npc_count; i++) {
        const NpcState *npc = &g_world.npcs[i];
        if (npc->flags & NPCF_HIDDEN) continue;
        if ((s32)npc->x == x && (s32)npc->y == y) return TRUE;
    }
    return FALSE;
}

// Check if the player just stepped onto a warp tile.
static void player_check_warps(void) {
    const MapHeader *map = g_world.map;
    s16 tx = g_world.player.tile_x;
    s16 ty = g_world.player.tile_y;

    for (u8 i = 0; i < map->warp_count; i++) {
        const WarpEvent *w = &map->warps[i];
        if (tx == (s16)w->x && ty == (s16)w->y) {
            world_do_warp(w);
            return;
        }
    }
}

static bool8 player_tile_has_warp(s32 x, s32 y) {
    const MapHeader *map = g_world.map;
    for (u8 i = 0; i < map->warp_count; i++) {
        const WarpEvent *w = &map->warps[i];
        if ((s16)w->x == (s16)x && (s16)w->y == (s16)y)
            return TRUE;
    }
    return FALSE;
}

// Check NPC interaction on A press. Return TRUE only when an interaction
// actually starts, so a harmless A press in the overworld stays silent.
static bool8 player_check_npc_interact(void) {
    const MapHeader *map = g_world.map;
    if (!map->npcs || g_world.npc_count == 0) return FALSE;

    PlayerState *p = &g_world.player;
    s16 fx = p->tile_x + DIR_DX[p->facing];
    s16 fy = p->tile_y + DIR_DY[p->facing];

    // Prefer the tile the player is facing, matching the original behavior,
    // but also accept any immediately adjacent NPC. Overworld sprites are
    // anchored one tile above their logical coordinate, so requiring only
    // the exact facing tile makes visually adjacent NPCs—especially Daisy in
    // Rival's House—appear non-interactive.
    s8 fallback = -1;

    for (u8 i = 0; i < g_world.npc_count; i++) {
        const NpcState *npc = &g_world.npcs[i];
        if (npc->flags & NPCF_HIDDEN) continue;

        // Pokéballs sit on the rear table surface. They can only be
        // examined from in front of or behind the table, never from the
        // sides, even though the generic NPC test supports all directions.
        bool8 is_pokeball = (npc->script_id >= 10 && npc->script_id <= 12);
        if (is_pokeball) {
            if (p->facing != DIR_UP)
                continue;

            // The balls are drawn on the rear edge of the table, while the
            // player stands on the floor in front of it. Match the visual
            // alignment rather than requiring the generic NPC tile to be
            // exactly one logical tile away.
            s16 dx = (s16)npc->x - p->tile_x;
            s16 dy = (s16)npc->y - p->tile_y;
            if (dx != 0 || dy != -1)
                continue;
            script_trigger_npc(npc->script_id, i);
            return TRUE;
        }

        if ((s16)npc->x == fx && (s16)npc->y == fy) {
            script_trigger_npc(npc->script_id, i);
            return TRUE;
        }

        s16 dx = (s16)npc->x - p->tile_x;
        s16 dy = (s16)npc->y - p->tile_y;
        if ((dx == 0 && (dy == 1 || dy == -1)) ||
            (dy == 0 && (dx == 1 || dx == -1)))
            fallback = (s8)i;
    }

    if (fallback >= 0) {
        const NpcState *npc = &g_world.npcs[(u8)fallback];
        s16 dx = (s16)npc->x - p->tile_x;
        s16 dy = (s16)npc->y - p->tile_y;
        if (dx > 0) p->facing = DIR_RIGHT;
        else if (dx < 0) p->facing = DIR_LEFT;
        else if (dy > 0) p->facing = DIR_DOWN;
        else p->facing = DIR_UP;
        script_trigger_npc(npc->script_id, (u8)fallback);
        return TRUE;
    }

    return FALSE;
}

static bool8 player_try_collision_warp(s32 nx, s32 ny) {
    // Fires when the player walks into an impassable tile that is a warp
    // (pokered: CheckWarpsCollision — used for building doors).
    const MapHeader *map = g_world.map;

    // Pallet Town's north connection spans the full visible top boundary in
    // the reference connection system. Once Oak's opening sequence is done,
    // allow the player to leave from any top-edge subtile instead of relying
    // on one exact warp coordinate.
    if (map->map_id == MAP_PALLET_TOWN && ny < 0 &&
        flags_get(FLAG_FOLLOWED_OAK_INTO_LAB)) {
        for (u8 i = 0; i < map->warp_count; i++) {
            if (map->warps[i].dest_map == MAP_ROUTE_1) {
                world_do_warp(&map->warps[i]);
                return TRUE;
            }
        }
    }

    // Map-edge warps are triggered while stepping off the boundary tile in
    // the reference engine. The destination warp is authored on the last
    // visible tile, so also match the player's current tile when nx/ny is
    // outside the map.
    const MapLayout *layout = map->layout;
    s32 map_width = layout->width * 2;
    s32 map_height = layout->height * 2;
    if (nx < 0 || ny < 0 || nx >= map_width || ny >= map_height) {
        for (u8 i = 0; i < map->warp_count; i++) {
            const WarpEvent *w = &map->warps[i];
            if ((s16)w->x == g_world.player.tile_x &&
                (s16)w->y == g_world.player.tile_y) {
                world_do_warp(w);
                return TRUE;
            }
        }
    }
    return FALSE;
}

bool8 player_script_start_step(Direction dir) {
    PlayerState *p = &g_world.player;
    s32 nx = p->tile_x + DIR_DX[dir];
    s32 ny = p->tile_y + DIR_DY[dir];

    p->facing = dir;

    if (script_oaks_lab_blocks_exit((u8)dir)) {
        p->move_state = MOVE_STATE_TURNING;
        p->step_frame = 4;
        p->step_dx = 0;
        p->step_dy = 0;
        return FALSE;
    }

    // Boundary warps must resolve before movement because their destination
    // is outside the current map. Exact warp tiles, however, are walkable:
    // the player should finish stepping onto the tile before the warp fires.
    if (player_try_collision_warp(nx, ny))
        return TRUE;

    if (!player_tile_has_warp(nx, ny) && !map_is_subtile_passable(nx, ny)) {
        p->move_state = MOVE_STATE_TURNING;
        p->step_frame = 4;
        p->step_dx = 0;
        p->step_dy = 0;
        return FALSE;
    }

    if (player_tile_occupied_by_npc(nx, ny)) {
        p->move_state = MOVE_STATE_TURNING;
        p->step_frame = 4;
        p->step_dx = 0;
        p->step_dy = 0;
        return FALSE;
    }

    p->tile_x     = (s16)nx;
    p->tile_y     = (s16)ny;
    p->move_state = MOVE_STATE_WALKING;
    p->step_frame = STEP_FRAMES;
    p->step_dx    = DIR_DX[dir];
    p->step_dy    = DIR_DY[dir];
    return TRUE;
}

bool8 player_script_is_moving(void) {
    return g_world.player.move_state == MOVE_STATE_WALKING;
}

bool8 player_script_start_step_forced(Direction dir) {
    PlayerState *p = &g_world.player;
    s32 nx = p->tile_x + DIR_DX[dir];
    s32 ny = p->tile_y + DIR_DY[dir];
    if (nx < 0 || ny < 0) return FALSE;

    p->facing = dir;
    p->tile_x = (s16)nx;
    p->tile_y = (s16)ny;
    p->move_state = MOVE_STATE_WALKING;
    p->step_frame = STEP_FRAMES;
    p->step_dx = DIR_DX[dir];
    p->step_dy = DIR_DY[dir];
    return TRUE;
}

void player_update(void) {
    PlayerState *p = &g_world.player;

    // Scripts block player input, but an already-started scripted walk must
    // continue advancing frame-by-frame.
    if (script_blocks_input() && g_world.player.move_state != MOVE_STATE_WALKING)
        return;
    if (dialog_is_open()) return;

    if (p->move_state == MOVE_STATE_FROZEN) return;

    if (p->move_state != MOVE_STATE_IDLE) {
        if (p->step_frame > 0) {
            p->step_frame--;
            if (p->move_state == MOVE_STATE_WALKING) {
                p->px += p->step_dx;
                p->py += p->step_dy;
            }
        }
        if (p->step_frame == 0) {
            if (p->move_state == MOVE_STATE_WALKING) {
                p->walk_cycle ^= 1;
                player_check_warps();
                // If warp fired, world_init() reset player to IDLE — bail out
                if (p->move_state == MOVE_STATE_IDLE) return;
                route1_try_wild_encounter();
            }
            p->move_state = MOVE_STATE_IDLE;
        }
        return;
    }

    // A button: NPC interaction
    if (input_pressed(KEY_A)) {
        if (player_check_npc_interact())
            audio_sfx_play(AUDIO_SFX_CONFIRM);
        return;
    }

    Direction dir = DIR_DOWN;
    bool8 moving  = TRUE;

    if      (input_held(KEY_DOWN))  dir = DIR_DOWN;
    else if (input_held(KEY_UP))    dir = DIR_UP;
    else if (input_held(KEY_LEFT))  dir = DIR_LEFT;
    else if (input_held(KEY_RIGHT)) dir = DIR_RIGHT;
    else    moving = FALSE;

    if (moving)
        player_script_start_step(dir);
}

void camera_update(void) {
    PlayerState *p   = &g_world.player;
    Camera      *cam = &g_world.camera;

    s32 target_x = (s32)p->tile_x * STEP_PIXELS - PLAYER_SCREEN_X;
    s32 target_y = (s32)p->tile_y * STEP_PIXELS - PLAYER_SCREEN_Y;

    cam->x = target_x - (p->step_frame * p->step_dx);
    cam->y = target_y - (p->step_frame * p->step_dy);

    const MapLayout *layout = g_world.map->layout;
    s32 map_width  = layout->width  * 32;
    s32 map_height = layout->height * 32;
    s32 max_x = map_width - 240;
    s32 max_y = map_height - 160;

    // Small interiors cannot fill the viewport. Keep the camera centered on
    // the player instead of clamping to (0,0), which pins the room to the
    // upper-left and leaves the rest of the screen blank. Larger maps keep
    // the usual edge clamping so the camera never reveals outside the map.
    if (max_x >= 0) {
        if (cam->x < 0) cam->x = 0;
        if (cam->x > max_x) cam->x = max_x;
    }
    if (max_y >= 0) {
        if (cam->y < 0) cam->y = 0;
        if (cam->y > max_y) cam->y = max_y;
    }
}
