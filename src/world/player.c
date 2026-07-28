#include "world.h"
#include "input.h"
#include "dialog.h"
#include "script.h"
#include "audio.h"
#include "route1.h"
#include "route2.h"
#include "route22.h"
#include "viridian_forest.h"
#include "map_ids.h"
#include "flags.h"

#define STEP_PIXELS  16
#define STEP_FRAMES  16

#define PLAYER_SCREEN_X  112
#define PLAYER_SCREEN_Y   72

static const s8 DIR_DX[4] = {  0,  0, -1,  1 };
static const s8 DIR_DY[4] = {  1, -1,  0,  0 };
static bool8 player_tile_occupied_by_npc(s32 x, s32 y);
static u8 s_trainer_approach_index = 0xFF;
static Direction s_trainer_player_facing = DIR_DOWN;

static bool8 player_trainer_sight_begin(void) {
    PlayerState *p = &g_world.player;
    for (u8 i = 0; i < g_world.npc_count; i++) {
        const NpcState *trainer = &g_world.npcs[i];
        if (trainer->flags & (NPCF_HIDDEN | NPCF_TRAINER_DEFEATED)) continue;
        if (!(trainer->flags & NPCF_TRAINER)) continue;

        u8 sight = trainer->trainer_sight ? trainer->trainer_sight : 2;
        s16 dx = p->tile_x - trainer->x;
        s16 dy = p->tile_y - trainer->y;
        Direction dir = (Direction)trainer->facing;
        s16 distance;
        if (dir == DIR_DOWN && dx == 0 && dy > 0) distance = dy;
        else if (dir == DIR_UP && dx == 0 && dy < 0) distance = -dy;
        else if (dir == DIR_RIGHT && dy == 0 && dx > 0) distance = dx;
        else if (dir == DIR_LEFT && dy == 0 && dx < 0) distance = -dx;
        else continue;
        if (distance < 1 || distance > sight) continue;

        // Match the reference's straight-line sight while preventing an
        // imported trainer from walking through a wall or another object.
        for (s16 step = 1; step < distance; step++) {
            s16 x = trainer->x + (dir == DIR_RIGHT ? step : dir == DIR_LEFT ? -step : 0);
            s16 y = trainer->y + (dir == DIR_DOWN ? step : dir == DIR_UP ? -step : 0);
            if (!map_is_subtile_passable(x, y)) {
                distance = 0;
                break;
            }
        }
        if (!distance) continue;

        s_trainer_approach_index = i;
        // Pokered freezes the player when the trainer notices them. Keep the
        // direction from the player's last movement instead of turning the
        // player toward the trainer during the approach cutscene.
        s_trainer_player_facing = p->facing;
        p->move_state = MOVE_STATE_FROZEN;
        return TRUE;
    }
    return FALSE;
}

static bool8 player_trainer_sight_update(void) {
    if (s_trainer_approach_index == 0xFF) return FALSE;
    if (s_trainer_approach_index >= g_world.npc_count) {
        s_trainer_approach_index = 0xFF;
        g_world.player.facing = s_trainer_player_facing;
        g_world.player.move_state = MOVE_STATE_IDLE;
        return FALSE;
    }

    NpcState *trainer = &g_world.npcs[s_trainer_approach_index];
    PlayerState *p = &g_world.player;
    Direction dir = (Direction)trainer->facing;
    s16 dx = p->tile_x - trainer->x;
    s16 dy = p->tile_y - trainer->y;
    s16 distance = (dir == DIR_DOWN) ? dy : (dir == DIR_UP) ? -dy :
                   (dir == DIR_RIGHT) ? dx : -dx;
    if (world_npc_is_moving(s_trainer_approach_index)) return TRUE;
    if (distance <= 1) {
        s_trainer_approach_index = 0xFF;
        p->facing = s_trainer_player_facing;
        p->move_state = MOVE_STATE_IDLE;
        script_trigger_npc(trainer->script_id, (u8)(trainer - g_world.npcs));
        return TRUE;
    }

    s16 nx = trainer->x + DIR_DX[dir];
    s16 ny = trainer->y + DIR_DY[dir];
    if (!map_is_subtile_passable(nx, ny) ||
        (nx == p->tile_x && ny == p->tile_y)) {
        s_trainer_approach_index = 0xFF;
        p->facing = s_trainer_player_facing;
        p->move_state = MOVE_STATE_IDLE;
        return FALSE;
    }
    world_npc_start_step(s_trainer_approach_index, dir);
    return TRUE;
}

static bool8 player_try_ledge_jump(Direction dir) {
    PlayerState *p = &g_world.player;
    if (!g_world.map->layout->tileset ||
        g_world.map->layout->tileset->use_cell_collision)
        return FALSE;

    s32 front_x = p->tile_x + DIR_DX[dir];
    s32 front_y = p->tile_y + DIR_DY[dir];
    u16 standing = map_get_subtile_tile_id(p->tile_x, p->tile_y);
    u16 front = map_get_subtile_tile_id(front_x, front_y);
    bool8 match = FALSE;

    // Exact pokered LedgeTiles table. There is deliberately no DIR_UP entry.
    if (dir == DIR_DOWN)
        match = (standing == 0x2C && front == 0x37) ||
                (standing == 0x39 && (front == 0x36 || front == 0x37));
    else if (dir == DIR_LEFT)
        match = (front == 0x27 && (standing == 0x2C || standing == 0x39));
    else if (dir == DIR_RIGHT)
        match = (standing == 0x2C && (front == 0x0D || front == 0x1D)) ||
                (standing == 0x39 && front == 0x0D);

    if (!match)
        return FALSE;

    s32 landing_x = p->tile_x + DIR_DX[dir] * 2;
    s32 landing_y = p->tile_y + DIR_DY[dir] * 2;
    if (!map_is_subtile_passable(landing_x, landing_y) ||
        player_tile_occupied_by_npc(landing_x, landing_y))
        return FALSE;

    p->tile_x = (s16)landing_x;
    p->tile_y = (s16)landing_y;
    p->move_state = MOVE_STATE_WALKING;
    p->step_frame = STEP_FRAMES * 2;
    p->step_dx = DIR_DX[dir];
    p->step_dy = DIR_DY[dir];
    p->ledge_jumping = TRUE;
    return TRUE;
}

// NPC coordinates are the same logical subtile coordinates used by player
// movement. Treat every visible NPC as a solid tile so the player cannot walk
// through people or scripted objects. The interaction code still allows the
// player to stand next to an NPC and press A.
static bool8 player_tile_occupied_by_npc(s32 x, s32 y) {
    for (u8 i = 0; i < g_world.npc_count; i++) {
        const NpcState *npc = &g_world.npcs[i];
        if (npc->flags & (NPCF_HIDDEN | NPCF_NO_SPRITE)) continue;
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
    if (!map) return FALSE;

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

    // Pokered signs and other wall-mounted text are background events rather
    // than NPCs. Prefer the tile directly in front of the player.
    if (map->bg_events) {
        for (u8 i = 0; i < map->bg_event_count; i++) {
            const BackgroundEvent *event = &map->bg_events[i];
            if ((s16)event->x == fx && (s16)event->y == fy) {
                script_trigger_background_event(event);
                return TRUE;
            }
        }
    }

    // In pokered the nurse object is behind the Pokécenter counter at
    // (3,1), while the player talks to the counter from the short approach
    // lane below it. The nurse's tall sprite makes this look like a normal
    // adjacent NPC interaction, but the logical coordinates are several
    // tiles apart. Run this only after ordinary NPC targeting has had a
    // chance, and keep it to the single reference counter approach tile so
    // it does not consume interactions across the south end.
    if (map->map_id == MAP_VIRIDIAN_POKECENTER &&
        p->tile_x == 3 && p->tile_y == 3) {
        for (u8 i = 0; i < g_world.npc_count; i++) {
            const NpcState *npc = &g_world.npcs[i];
            if (!(npc->flags & NPCF_HIDDEN) && npc->script_id == 25) {
                script_trigger_npc(npc->script_id, i);
                return TRUE;
            }
        }
    }

    // Viridian Mart clerk is behind the left counter at (0,5). The player
    // stands at x=2 facing LEFT (counter-side interaction, matching pokered's
    // counter NPC mechanic where the player talks across an impassable tile).
    if (map->map_id == MAP_VIRIDIAN_MART &&
        p->tile_x == 2 && p->facing == DIR_LEFT) {
        for (u8 i = 0; i < g_world.npc_count; i++) {
            const NpcState *npc = &g_world.npcs[i];
            if (!(npc->flags & NPCF_HIDDEN) && npc->script_id == 22 &&
                (s16)npc->y == (s16)p->tile_y) {
                script_trigger_npc(npc->script_id, i);
                return TRUE;
            }
        }
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
        Direction exit_direction = nx < 0 ? DIR_LEFT :
                                   nx >= map_width ? DIR_RIGHT :
                                   ny < 0 ? DIR_UP : DIR_DOWN;
        for (u8 i = 0; i < map->connection_count; i++) {
            const MapConnection *connection = &map->connections[i];
            if (connection->direction != (u8)exit_direction) continue;
            s16 coordinate = (exit_direction == DIR_UP || exit_direction == DIR_DOWN)
                           ? g_world.player.tile_x : g_world.player.tile_y;
            world_do_connection(connection->dest_map, exit_direction,
                                coordinate, connection->offset);
            return TRUE;
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

    if (player_try_ledge_jump(dir))
        return TRUE;

    // Boundary warps must resolve before movement because their destination
    // is outside the current map. Exact warp tiles, however, are walkable:
    // the player should finish stepping onto the tile before the warp fires.
    if (player_try_collision_warp(nx, ny))
        return TRUE;

    if (!player_tile_has_warp(nx, ny) &&
        !map_is_subtile_passable_from(nx, ny, dir)) {
        p->move_state = MOVE_STATE_TURNING;
        p->step_frame = 4;
        p->step_dx = 0;
        p->step_dy = 0;
        return FALSE;
    }

    // Pokered's Viridian Old Man is a map-script gate, not merely a solid
    // NPC. Trying to step onto his tile displays the private-property text
    // and leaves the player on the approach tile until the Pokédex flag is
    // earned.
    if (script_viridian_old_man_blocks(nx, ny)) {
        script_trigger_npc(19, 4);
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

    if (s_trainer_approach_index != 0xFF) {
        player_trainer_sight_update();
        return;
    }

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
                p->ledge_jumping = FALSE;
                player_check_warps();
                // If warp fired, world_init() reset player to IDLE — bail out
                if (p->move_state == MOVE_STATE_IDLE) return;
                route1_try_wild_encounter();
                route2_try_wild_encounter();
                route22_try_wild_encounter();
                viridian_forest_try_wild_encounter();
            }
            p->move_state = MOVE_STATE_IDLE;
        }
        return;
    }

    if (player_trainer_sight_begin()) {
        player_trainer_sight_update();
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
