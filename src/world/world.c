#include "world.h"
#include "game.h"
#include "render.h"
#include "gba.h"
#include "script.h"
#include "dialog.h"
#include "text.h"
#include "map_ids.h"
#include "gfx_pokeball.h"
#include "gfx_npcs.h"
#include "irq.h"
#include "flags.h"
#include "audio.h"
#include "audio.h"

typedef enum {
    WORLD_TRANSITION_NONE = 0,
    WORLD_TRANSITION_OUT,
    WORLD_TRANSITION_IN,
} WorldTransitionPhase;

static WorldTransitionPhase s_transition_phase = WORLD_TRANSITION_NONE;
static u8 s_transition_level;
static WarpEvent s_pending_warp;
static bool8 s_pending_connection;
static u8 s_connection_dest_map;
static Direction s_connection_direction;
static s16 s_connection_coordinate;
static s8 s_connection_offset;

// GBA hardware fade registers. Darken mode applies a black overlay to all
// visible layers; BLDY ranges from 0 (normal) to 16 (fully black).
#define WORLD_REG_BLDCNT (*(vu16 *)0x04000050)
#define WORLD_REG_BLDY   (*(vu16 *)0x04000054)
static void world_transition_apply(void) {
    if (s_transition_phase == WORLD_TRANSITION_NONE) {
        WORLD_REG_BLDCNT = 0;
        WORLD_REG_BLDY = 0;
        // Restore the room clipping window after the fade. Keeping it off
        // during the fade prevents the small interior's edge from appearing
        // as a line while the destination map is being rebuilt.
        tilemap_update_scroll();
        return;
    }

    WORLD_REG_BLDCNT = 0x00BF; // darken BG0-BG3, OBJ, and backdrop
    WORLD_REG_BLDY = s_transition_level;
}

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
    p->ledge_jumping = FALSE;

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
        dst->step_dir = dst->facing;
        dst->step_frame = 0;
        dst->walking = FALSE;
        dst->walk_cycle = 0;
        dst->movement = src->movement;
        dst->move_timer = (u16)(i * 23);
        dst->item_id = src->item_id;
        dst->item_flag = src->item_flag;
        dst->text = src->text;
        dst->trainer_id = src->trainer_id;
        dst->trainer_party = src->trainer_party;
        dst->trainer_sight = src->trainer_sight;
        dst->trainer_flag = src->trainer_flag;
        dst->trainer_text = src->trainer_text;
        if ((src->flags & NPCF_TRAINER) && src->trainer_flag != 0 &&
            src->trainer_flag < FLAG_COUNT &&
            flags_get((GameFlag)src->trainer_flag))
            dst->flags |= NPCF_TRAINER_DEFEATED;
        if ((src->flags & NPCF_ITEM) && src->item_flag < FLAG_COUNT &&
            flags_get((GameFlag)src->item_flag))
            dst->flags |= NPCF_HIDDEN;
    }

    // Map loads rebuild NPC state from the static map definition. Reapply
    // Oak's Lab's persistent object state so taken balls and the departed
    // Rival do not reappear after leaving and re-entering the room.
    if (map->map_id == MAP_OAKS_LAB) {
        if (flags_get(FLAG_RIVAL_LEFT_OAKS_LAB))
            g_world.npcs[0].flags |= NPCF_HIDDEN;
        if (flags_get(FLAG_OAKSLAB_CHARMANDER_TAKEN))
            g_world.npcs[2].flags |= NPCF_HIDDEN;
        if (flags_get(FLAG_OAKSLAB_BULBASAUR_TAKEN))
            g_world.npcs[3].flags |= NPCF_HIDDEN;
        if (flags_get(FLAG_OAKSLAB_SQUIRTLE_TAKEN))
            g_world.npcs[4].flags |= NPCF_HIDDEN;
    }
    if (map->map_id == MAP_RIVALS_HOUSE && flags_get(FLAG_GOT_TOWN_MAP)) {
        if (g_world.npc_count > 1)
            g_world.npcs[1].flags |= NPCF_HIDDEN;
    }

    tilemap_init();
    camera_update();
    tilemap_rebuild();
    tilemap_load_player_sprite();
    tilemap_update_scroll();

    // Clear the UI layer (BG0) when loading a new map
    text_clear();

    if (map->music_id)
        audio_music_play((AudioMusicId)map->music_id);
    else
        audio_music_stop();
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
    npc->step_dir = (u8)dir;
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
        if (!npc->walking) {
            if (npc->flags & NPCF_HIDDEN || npc->movement == NPC_MOVE_STAY ||
                script_blocks_input() || dialog_is_open())
                continue;

            // Pokered's WALK/ANY_DIR and WALK/UP_DOWN objects pause between
            // steps. Pick a deterministic direction from the VBlank counter,
            // then reject fence, building, player, and NPC collisions.
            if (++npc->move_timer < (u16)(45 + i * 18)) continue;
            npc->move_timer = 0;
            u8 start = (u8)((g_vblank_count + i * 3) & 3);
            for (u8 attempt = 0; attempt < 4; attempt++) {
                Direction dir = (Direction)((start + attempt) & 3);
                if (npc->movement == NPC_MOVE_UP_DOWN &&
                    (dir == DIR_LEFT || dir == DIR_RIGHT))
                    continue;
                if (npc->movement == NPC_MOVE_LEFT_RIGHT &&
                    (dir == DIR_UP || dir == DIR_DOWN))
                    continue;
                s16 nx = (s16)npc->x +
                    (dir == DIR_RIGHT ? 1 : dir == DIR_LEFT ? -1 : 0);
                s16 ny = (s16)npc->y +
                    (dir == DIR_DOWN ? 1 : dir == DIR_UP ? -1 : 0);
                if (!map_is_subtile_passable(nx, ny)) continue;
                if (g_world.player.tile_x == nx && g_world.player.tile_y == ny)
                    continue;
                bool8 occupied = FALSE;
                for (u8 j = 0; j < g_world.npc_count; j++) {
                    if (j != i && !(g_world.npcs[j].flags & NPCF_HIDDEN) &&
                        g_world.npcs[j].x == nx && g_world.npcs[j].y == ny) {
                        occupied = TRUE;
                        break;
                    }
                }
                if (!occupied) {
                    world_npc_start_step(i, dir);
                    break;
                }
            }
            continue;
        }
        if (npc->step_frame > 0) {
            npc->step_frame--;
            npc->px += npc->step_dx;
            npc->py += npc->step_dy;
        }
        if (npc->step_frame == 0) {
            npc->walking = FALSE;
            npc->walk_cycle ^= 1;
            npc->move_timer = 0;
        }
    }
}

static void world_finish_warp(void) {
    const MapHeader *dest;

    u8 spawn_x = 4, spawn_y = 4;
    if (s_pending_connection) {
        dest = map_get_by_id(s_connection_dest_map);
    } else if (s_pending_warp.dest_map == WARP_LAST_MAP) {
        dest = g_world.last_map;
    } else {
        dest = map_get_by_id(s_pending_warp.dest_map);
    }

    if (!dest) {
        s_transition_phase = WORLD_TRANSITION_NONE;
        world_transition_apply();
        g_world.player.move_state = MOVE_STATE_IDLE;
        return; // destination not yet implemented
    }

    if (s_pending_connection) {
        s32 coordinate = (s32)s_connection_coordinate + s_connection_offset;
        s32 max_x = dest->layout->width * 2 - 1;
        s32 max_y = dest->layout->height * 2 - 1;
        if (s_connection_direction == DIR_UP || s_connection_direction == DIR_DOWN) {
            if (coordinate < 0) coordinate = 0;
            if (coordinate > max_x) coordinate = max_x;
            spawn_x = (u8)coordinate;
            spawn_y = s_connection_direction == DIR_UP ? (u8)max_y : 0;
        } else {
            if (coordinate < 0) coordinate = 0;
            if (coordinate > max_y) coordinate = max_y;
            spawn_y = (u8)coordinate;
            spawn_x = s_connection_direction == DIR_LEFT ? (u8)max_x : 0;
        }
    } else if (s_pending_warp.dest_warp < dest->warp_count) {
        // Find the destination spawn point from the destination warp table.
        const WarpEvent *dw = &dest->warps[s_pending_warp.dest_warp];
        spawn_x = dw->x;
        spawn_y = dw->y;
    }

    g_world.last_map = g_world.map;
    world_init(dest, spawn_x, spawn_y);
    audio_sfx_play(AUDIO_SFX_WARP_IN);
}

void world_do_warp(const WarpEvent *w) {
    if (!w || s_transition_phase != WORLD_TRANSITION_NONE) return;

    s_pending_warp = *w;
    s_pending_connection = FALSE;
    audio_sfx_play(AUDIO_SFX_WARP_OUT);

    s_transition_level = 0;
    s_transition_phase = WORLD_TRANSITION_OUT;
    g_world.player.move_state = MOVE_STATE_FROZEN;
    world_transition_apply();
}

void world_do_connection(u8 dest_map, Direction direction, s16 source_coordinate,
                         s8 offset) {
    if (s_transition_phase != WORLD_TRANSITION_NONE) return;
    s_connection_dest_map = dest_map;
    s_connection_direction = direction;
    s_connection_coordinate = source_coordinate;
    s_connection_offset = offset;
    s_pending_connection = TRUE;
    audio_sfx_play(AUDIO_SFX_WARP_OUT);
    s_transition_level = 0;
    s_transition_phase = WORLD_TRANSITION_OUT;
    g_world.player.move_state = MOVE_STATE_FROZEN;
    world_transition_apply();
}

void world_update(void) {
    if (s_transition_phase == WORLD_TRANSITION_OUT) {
        if (s_transition_level < 16)
            s_transition_level = (u8)(s_transition_level + 4);
        if (s_transition_level >= 16) {
            world_finish_warp();
            s_transition_phase = WORLD_TRANSITION_IN;
        }
        world_transition_apply();
        return;
    }

    if (s_transition_phase == WORLD_TRANSITION_IN) {
        if (s_transition_level >= 4)
            s_transition_level = (u8)(s_transition_level - 4);
        else {
            s_transition_level = 0;
            s_transition_phase = WORLD_TRANSITION_NONE;
            g_world.player.move_state = MOVE_STATE_IDLE;
        }
        world_transition_apply();
        return;
    }

    // Generic NPC dialogs are map-independent. This must run before the map
    // script so maps without a script (for example Rival's House) still
    // advance and close their dialogs correctly.
    script_update();

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
            if (npc->flags & (NPCF_HIDDEN | NPCF_NO_SPRITE)) continue;

            // Align the sprite with the logical tile used for interaction.
            // The former anchor placed NPC graphics left and nearly a tile
            // above the position where their dialogue triggered.
            s16 nx = (s16)(npc->px - cam->x);
            s16 ny = (s16)(npc->py - cam->y - 4);

            // Pokéballs sit on the rear edge of Oak's Lab's table, one tile
            // above their logical interaction coordinates.
            // The original Pokéball art is slightly left-biased relative
            // to the table's visual center in this tileset.
            // Only draw if on screen
            if (nx < -16 || nx > 240 || ny < -16 || ny > 160) continue;

            u16 sprite_id = npc->sprite_tile;
            u8 sprite_param = 0;
            if (npc->sprite_tile == GFX_BLUE_TILE_BASE ||
                npc->sprite_tile == GFX_YOUNGSTER_TILE_BASE ||
                npc->sprite_tile == GFX_GAMBLER_TILE_BASE ||
                npc->sprite_tile == GFX_OAK_TILE_BASE ||
                npc->sprite_tile == GFX_GIRL_TILE_BASE ||
                npc->sprite_tile == GFX_SCIENTIST_TILE_BASE ||
                npc->sprite_tile == GFX_FISHER_TILE_BASE ||
                npc->sprite_tile == GFX_DAISY_TILE_BASE ||
                npc->sprite_tile == GFX_MOM_TILE_BASE) {
                // Use the direction captured when the step began. Deriving
                // the pose from pixel deltas allowed a scripted Oak step to
                // reuse the previous left-facing animation during DOWN.
                Direction pose_dir = npc->walking
                    ? (Direction)npc->step_dir
                    : (Direction)npc->facing;
                if (npc->sprite_tile == GFX_OAK_TILE_BASE && npc->walking &&
                    npc->step_dir == DIR_DOWN)
                    pose_dir = DIR_DOWN;
                u8 frame = (pose_dir == DIR_DOWN) ? 0 :
                           (pose_dir == DIR_UP)   ? 1 : 2;
                // Keep Oak visibly facing down throughout DOWN steps. The
                // source walking frame reads as a left/down pose in this
                // converted sheet, so use the clean down-facing frame.
                if (npc->walking && (npc->step_frame & 8) &&
                    !(npc->sprite_tile == GFX_OAK_TILE_BASE &&
                      npc->step_dir == DIR_DOWN))
                    frame += 3;
                if (pose_dir == DIR_RIGHT) sprite_param ^= 1;
                sprite_id = (u16)(npc->sprite_tile + frame * 4);
            }

            RenderCmd cmd = {
                .type  = RCMD_DRAW_SPRITE,
                .id    = sprite_id,
                .x     = nx,
                .y     = ny,
                .param = (u8)(sprite_param | 0x10), // NPC palette bank 1
            };
            render_submit(cmd);
        }
    }

    // Draw player (always on top of NPCs)
    s16 sx = (s16)(p->px - cam->x);
    // pokered aligns sprite Y positions four pixels above the map grid
    // position (InitializeSpriteScreenPosition).
    s16 sy = (s16)(p->py - cam->y - 4);

    if (p->ledge_jumping) {
        u8 progress = (u8)(32 - p->step_frame);
        u8 height = progress <= 16
                  ? (u8)(progress / 2)
                  : (u8)((32 - progress) / 2);
        sy -= height;
    }

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
