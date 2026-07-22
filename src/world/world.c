#include "world.h"
#include "game.h"
#include "render.h"
#include "gba.h"
#include "script.h"
#include "text.h"
#include "map_ids.h"
#include "gfx_pokeball.h"
#include "gfx_npcs.h"

void world_init(const MapHeader *map, u8 start_x, u8 start_y) {
    PlayerState *p = &g_world.player;

    map_load(map);
    script_reset_runtime();

    p->tile_x     = (s16)start_x;
    p->tile_y     = (s16)start_y;
    p->px         = start_x * 16;
    p->py         = start_y * 16;
    p->facing     = DIR_DOWN;
    p->move_state = MOVE_STATE_IDLE;
    p->step_frame = 0;
    p->walk_cycle = 0;

    g_world.npc_count = map->npc_count;
    if (g_world.npc_count > ARRAY_COUNT(g_world.npcs))
        g_world.npc_count = ARRAY_COUNT(g_world.npcs);
    for (u8 i = 0; i < g_world.npc_count; i++) {
        const NpcDef *src = &map->npcs[i];
        NpcState *dst = &g_world.npcs[i];
        dst->x = src->x;
        dst->y = src->y;
        dst->sprite_tile = src->sprite_tile;
        dst->facing = src->facing;
        dst->flags = src->flags;
        dst->script_id = src->script_id;
        dst->px = (s16)src->x * 16;
        dst->py = (s16)src->y * 16;
        dst->step_dx = 0;
        dst->step_dy = 0;
        dst->step_frame = 0;
        dst->walking = FALSE;
        dst->walk_cycle = 0;
    }

    tilemap_init();
    tilemap_rebuild();
    tilemap_load_player_sprite();
    camera_update();
    tilemap_update_scroll();

    // Clear the UI layer (BG0) when loading a new map
    text_clear();
}

void world_npc_start_step(u8 index, Direction dir) {
    if (index >= g_world.npc_count) return;
    NpcState *npc = &g_world.npcs[index];
    if (npc->walking) return;

    static const s8 dx[] = { 0, 0, -1, 1 };
    static const s8 dy[] = { 1, -1, 0, 0 };
    s32 nx = npc->x + dx[dir];
    s32 ny = npc->y + dy[dir];
    // Scripted NPC paths are authored against the reference map and may
    // cross grass/door transition tiles that the player collision sampler
    // intentionally treats conservatively. Player movement still uses the
    // normal collision path in player.c.
    npc->facing = (u8)dir;
    npc->x = (u8)nx;
    npc->y = (u8)ny;
    npc->step_dx = dx[dir];
    npc->step_dy = dy[dir];
    npc->step_frame = 16;
    npc->walking = TRUE;
}

bool8 world_npc_is_moving(u8 index) {
    return index < g_world.npc_count && g_world.npcs[index].walking;
}

static void world_npcs_update(void) {
    for (u8 i = 0; i < g_world.npc_count; i++) {
        NpcState *npc = &g_world.npcs[i];
        if (!npc->walking) continue;
        if (npc->step_frame > 0) {
            npc->step_frame--;
            npc->px += npc->step_dx;
            npc->py += npc->step_dy;
        }
        if (npc->step_frame == 0) {
            npc->walking = FALSE;
            npc->walk_cycle ^= 1;
        }
    }
}

void world_do_warp(const WarpEvent *w) {
    const MapHeader *dest;

    if (w->dest_map == WARP_LAST_MAP) {
        dest = g_world.last_map;
    } else {
        dest = map_get_by_id(w->dest_map);
    }

    if (!dest) return; // destination not yet implemented

    // Find the destination spawn point from dest's warp table
    u8 spawn_x = 4, spawn_y = 4; // fallback
    if (w->dest_warp < dest->warp_count) {
        const WarpEvent *dw = &dest->warps[w->dest_warp];
        spawn_x = dw->x;
        spawn_y = dw->y;
    }

    g_world.last_map = g_world.map;
    world_init(dest, spawn_x, spawn_y);
}

void world_update(void) {
    // Per-frame map script (may block input, show dialog, etc.)
    if (g_world.map->script)
        g_world.map->script();

    world_npcs_update();
    player_update();
    camera_update();
    tilemap_update_scroll();
}

void world_render(void) {
    const PlayerState *p   = &g_world.player;
    const Camera      *cam = &g_world.camera;

    render_clear_sprites();

    // Draw NPCs as OBJ sprites
    if (g_world.npc_count) {
        for (u8 i = 0; i < g_world.npc_count; i++) {
            const NpcState *npc = &g_world.npcs[i];
            if (npc->flags & NPCF_HIDDEN) continue;

            s16 nx = (s16)(npc->px - cam->x - 8);
            s16 ny = (s16)(npc->py - cam->y - 16);

            // The original Pokéball art is slightly left-biased relative
            // to the table's visual center in this tileset.
            if (g_world.map->map_id == MAP_OAKS_LAB &&
                npc->sprite_tile == GFX_POKEBALL_TILE_BASE)
                nx += 8;

            // Only draw if on screen
            if (nx < -16 || nx > 240 || ny < -16 || ny > 160) continue;

            u16 sprite_id = npc->sprite_tile;
            u8 sprite_param = 0;
            if (npc->sprite_tile == GFX_BLUE_TILE_BASE ||
                npc->sprite_tile == GFX_OAK_TILE_BASE) {
                /* While stepping, use the step vector rather than a facing
                 * value that another cutscene command may update mid-step.
                 * This keeps a downward walk on Oak's down frames. */
                Direction pose_dir = (Direction)npc->facing;
                if (npc->step_dy > 0) pose_dir = DIR_DOWN;
                else if (npc->step_dy < 0) pose_dir = DIR_UP;
                else if (npc->step_dx < 0) pose_dir = DIR_LEFT;
                else if (npc->step_dx > 0) pose_dir = DIR_RIGHT;
                u8 frame = (pose_dir == DIR_DOWN) ? 0 :
                           (pose_dir == DIR_UP)   ? 1 : 2;
                /* Select walking art from the actual movement vector. This
                 * keeps a vertical cutscene step on its matching vertical
                 * walking frame even if the scripted facing value changes. */
                if (npc->walking) {
                    if (npc->step_dy > 0) {
                        /* The converted Oak walking slot is side-facing in
                         * this asset build. Keep the valid front-facing pose
                         * during downward movement until that asset is
                         * re-converted from the reference sheet. */
                        frame = 0;
                    } else if (npc->step_dy < 0 && (npc->step_frame & 8)) {
                        frame = 4;
                    } else if (npc->step_dx != 0 && (npc->step_frame & 8)) {
                        frame = 5;
                    }
                }
                if (pose_dir == DIR_RIGHT)
                    sprite_param ^= 1;
                if (sprite_id == npc->sprite_tile)
                    sprite_id = (u16)(npc->sprite_tile + frame * 4);
            } else if (npc->walking && (npc->step_frame & 8)) {
                sprite_id = (u16)(npc->sprite_tile + 4);
            }

            RenderCmd cmd = {
                .type  = RCMD_DRAW_SPRITE,
                .id    = sprite_id,
                .x     = nx,
                .y     = ny,
                .param = sprite_param,
            };
            render_submit(cmd);
        }
    }

    // Draw player (always on top of NPCs)
    s16 sx = (s16)(p->px - cam->x);
    // pokered aligns sprite Y positions four pixels above the map grid
    // position (InitializeSpriteScreenPosition).
    s16 sy = (s16)(p->py - cam->y - 4);

    // The sheet stores down, up, and one side-facing pose. Left and right
    // share the side pose; the renderer mirrors it for the opposite side.
    u8 frame = (p->facing == DIR_DOWN) ? 0 :
               (p->facing == DIR_UP)   ? 1 : 2;
    bool8 walking_pose = (p->move_state == MOVE_STATE_WALKING &&
                          (p->step_frame & 8));
    if (walking_pose)
        frame = (u8)(frame + 3);

    RenderCmd cmd = {
        .type  = RCMD_DRAW_SPRITE,
        .id    = (u16)(frame * 4),
        .x     = sx,
        .y     = sy,
        .param = (p->facing == DIR_RIGHT) ? 1 : 0,
    };
    render_submit(cmd);
}
