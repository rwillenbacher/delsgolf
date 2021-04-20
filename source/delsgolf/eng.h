/*
Copyright (c) 2013-2015, Ralf Willenbacher
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in
   the documentation and/or other materials provided with the
   distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
*/



#ifdef WIN32
#include <Windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>
#else
#include <tigcclib.h>
#undef assert
#define assert() error
#endif




#define PERFORMANCE_COUNTERS 0
#define SIMPLE_PERFORMANCE_COUNTERS 0

#define REND_SCREEN_WIDTH  144
#define REND_SCREEN_HEIGHT 72
#define REND_OFFSET_LEFT   8
#define REND_OFFSET_TOP    4

#if REND_SCREEN_WIDTH & 7
#error "render buffer width not multiple of 8"
#endif

#if REND_OFFSET_LEFT & 7
#error "render buffer offset left not multiple of 8"
#endif

/*
#define REND_SCREEN_WIDTH  ((160*3)/4)
#define REND_SCREEN_HEIGHT ((100*3)/4)
*/

/*
#define REND_SCREEN_WIDTH  96
#define REND_SCREEN_HEIGHT 60
*/

#define MAX_NUM_ENTITIES 256

#define TICKS_PER_SECOND 10
#define TICK_MS          ( 1000 / TICKS_PER_SECOND )
#define TICKS( ms ) ( ( ms ) / TICK_MS )

#define YSCALE_CONST ( ( 256L * ( REND_SCREEN_HEIGHT/2 * REND_SCREEN_WIDTH )  ) / REND_SCREEN_HEIGHT )
#define INVERSE_VSCALE_SCALE ( 0xffffff / ( YSCALE_CONST ) )
#define INV_HALF_SCREEN_WIDTH ( 0x10000 / ( REND_SCREEN_WIDTH / 2 ) )


#define DRAWBUFFER_SCREEN_WIDTH 160
#define DRAWBUFFER_SCREEN_HEIGHT 100
#define DRAWBUFFER_SIZE ( ( 160 * 100 ) >> 2 )

#define SPRITE_SCALE_SHIFT 0

#ifdef WIN32
typedef char Int8;
typedef short Int16;
typedef int Int32;

typedef unsigned char UInt8;
typedef unsigned short UInt16;
typedef unsigned int UInt32;
typedef int Bool;
#else
typedef char Int8;
typedef int Int16;
typedef long Int32;

typedef unsigned char UInt8;
typedef unsigned int UInt16;
typedef unsigned long UInt32;
#endif


#include "mathlib.h"



typedef struct {
	UInt16 i_normal;
	Int16 i_length;
} plane_t;

typedef struct {
	UInt16 i_v1;
	UInt16 i_v2;
	UInt16 i_plane;
	UInt16 i_backsector;
	UInt16 ui8_flags;
	UInt8 ui8_texture_upper;
	UInt8 ui8_texture_lower;
} line_t;

typedef struct {
	UInt16 i_num_lines;
	UInt16 i_line_offset;
	UInt16 i_zvert;
	UInt8 ui8_texture_ceiling;
	UInt8 ui8_texture_floor;
	UInt16 i_connector;
	UInt16 i_visibility;
} sector_t;

typedef struct {
	UInt16 i_plane;
	UInt16 i_parent_node;
	UInt16 i_sector_offset;
	UInt16 i_backnode;
	UInt16 i_bbox_min;
	UInt16 i_bbox_max;
}node_t;

typedef struct {
	UInt16 i_plane;
	UInt16 i_backnode;
}clipnode_t;

typedef struct {
	UInt8 ui8_classname;
	UInt8 ui8_flags;
	UInt16 i_origin;
}mentity_t;

typedef struct {
	UInt16 i_origin;
	UInt16 i_connections_offset;
	UInt8 ui8_num_connections;
	UInt8 ui8_path_flags;
	UInt16 i_path_distance;
	UInt16 i_path_previous;
}connector_t;

typedef struct {
	Int16 i_num_vertices;
	Int16 i_num_planes;
	Int16 i_num_lines;
	Int16 i_num_sectors;
	Int16 i_num_nodes;
	Int16 i_num_entities;
	Int16 i_num_textures;
	Int16 i_num_visibility;
	Int16 i_num_connectors;
	Int16 i_num_connector_connections;
	Int16 i_num_clip_nodes;
	Int16 i_num_entity_group_sectors;
	Int16 i_entity_clip_tree_root;
	Int16 i_padding;

	void *p_loaded_map;

	vec2_t *p_vertices;
	plane_t *p_planes;
	line_t *p_lines;
	sector_t *p_sectors;
	node_t *p_nodes;
	mentity_t *p_entities;
	UInt8 ( *rgpui8_textures )[ 16 ];
	UInt8 *pui8_visibility;
	connector_t *p_connectors;
	UInt16 *pi_connections;
	UInt8 *pui8_connection_distance;
	clipnode_t *p_clipnodes;
}map_t;

typedef struct {
	UInt16 ui16_next_entity;
	UInt8 ui8_entity;
} entities_in_sector_frags_t;

typedef struct sectors_of_entitiy_frags_s {
	UInt16 ui16_next_sector;
	UInt16 ui16_sector;
} sectors_of_entity_frags_t;


#define MAP_LINE_FLAGS_TMAP_Y  1
#define MAP_LINE_FLAGS_PATH    2
#define MAP_LINE_FLAGS_UPPER   4
#define MAP_LINE_FLAGS_LOWER   8
#define MAP_LINE_FLAGS_MIDDLE  16

typedef struct {
	Int16 i_x1;
	Int16 i_y1;
	Int16 i_x2;
	Int16 i_y2;
	Int16 i_z1;
	Int16 i_z2;
	Int16 i_z3;
	Int16 i_z4;
	UInt8 ui8_flags;
} eng_map_line_segment_t;

typedef struct {
	UInt16 x1;
	UInt16 x2;

	Int32 y01;
	Int32 y11;
	Int32 y21;
	Int32 y31;
	Int32 y02;
	Int32 y12;
	Int32 y22;
	Int32 y32;


	UInt8 ui8_flags;
} eng_line_segment_t;

typedef Int16 engvec_t[ 3 ];

typedef struct {
	UInt8 rgui8_name[ 8 ];
	UInt16 i_file_offset;
	UInt16 i_texture_length;
} repository_texture_t;

typedef struct {
	UInt8 ui8_width;
	UInt8 ui8_height;
	UInt16 i_file_offset;
	UInt16 i_sprite_length;
} repository_sprite_t;

typedef struct {
	UInt16 i_file_offset;
	UInt16 i_map_length;
} repository_map_t;


typedef struct {
	UInt16 i_num_textures;
	UInt16 i_num_sprites;
	UInt16 i_num_maps;
	UInt16 i_num_texture_data;
} repository_header_t;

typedef struct {
	Int16 b_file_up;
#ifdef WIN32
	FILE *f_file;
#else
	FILES s_file;
#endif
	repository_header_t s_header;
	void *p_allocated;

	repository_texture_t *rgs_textures;
	repository_sprite_t *rgs_sprites;
	repository_map_t *rgs_maps;
} repository_t;

typedef struct {
	Int16 i_idx;
	Int16 i_lru;
	Int16 i_offset;
} cached_texture_t;

#define TEXTURE_CACHE_SIZE 16
typedef struct {
	Int16 i_lru_counter;
	Int16 i_num_cached_textures;
	cached_texture_t rgs_textures[ TEXTURE_CACHE_SIZE ];
	UInt8 rgui8_texture_data[ 32 * ( ( ( TEXTURE_CACHE_SIZE + 7 ) / 8 ) * 256 ) ];
} texture_cache_t;

typedef struct cached_sprite_s {
	struct cached_sprite_s *p_next;
	Int16 i_idx;
	repository_sprite_t *p_sprite;
	UInt16 ui_lru;
	UInt16 ui_offset;
	Int16 i_size;
} cached_sprite_t;

#define SPRITE_CACHE_SIZE ( 128 * 128 )
#define SPRITE_CACHE_ENTRIES 48
typedef struct {
	UInt16 ui_lru_counter;
	UInt16 ui_sprite_cache_free_data;
	cached_sprite_t rgs_cached_sprites[ SPRITE_CACHE_ENTRIES ];
	cached_sprite_t *p_free;
	cached_sprite_t *p_used;
	/* if this is specified after a 4 byte pointer it should be 4 byte aligned, leading to faster memcopies on compaction */
	UInt8 rgui8_sprite_data[ SPRITE_CACHE_SIZE ];
	UInt32 ui_sprite_data_padding;
} sprite_cache_t;


typedef struct entity_s {
	UInt8 ui8_entity_id;
	Int16 i_entityclass;
	UInt16 ui16_sectors_of_entity;

#define ENTITY_FLAGS_ACTIVE 1
#define ENTITY_FLAGS_DRAW   2
#define ENTITY_FLAGS_DRAWN  4
#define ENTITY_FLAGS_CLIP   8
#define ENTITY_FLAGS_ENTITY_CHECKED 16
	UInt16 ui_flags;
	UInt16 i_radius;
	UInt16 i_height;
	vec3_t v_origin;
	UInt16 ui_last_draw;

	UInt16 i_last_think;
	Int16 i_health;

	UInt8  ui8_think_state;
	UInt8  ui8_think_state_duration;
	UInt8  ui8_anim_state;
	UInt8  ui8_anim_state_duration;
	UInt8 i_sprite_index;
	deg8_t d8_yaw;

} entity_t;

struct engine_s;
typedef void ( *entity_spawn_f ) ( struct engine_s *ps_eng, entity_t *p_entity, mentity_t *p_mentity );
typedef void ( *entity_think_f ) ( struct engine_s *ps_eng, entity_t *p_entity );
typedef void ( *entity_gothit_f ) ( struct engine_s *ps_eng, entity_t *p_entity, Int16 i_damage );

typedef struct {
	UInt8 rgui8_name[ 16 ];
	UInt8 ui8_radius;
	UInt8 ui8_height;
	entity_spawn_f f_spawn;
	entity_think_f f_think;
	entity_gothit_f f_gothit;
} entityclass_t;


typedef struct {
	UInt8 i_next;
	UInt8 i_x1;
	UInt8 i_x2;
}cliplist_t;

typedef struct {
	UInt32 i_u, i_v;
	Int32 i_scaleu, i_scalev;
	UInt16 i_clip_idx;
	Int16 i_sprite_idx;
	Int16 i_dist;
	UInt8 i_x1, i_x2;
	UInt8 i_y1, i_y2;
} drawsprite_t;

typedef struct {
	UInt8 ui8_width;
	UInt8 rgui8_data[ 6 ];
} char_t;

typedef Int16 ( *sector_list_cb_f ) ( sector_t *p_sector, vec3_t v1, vec3_t v2 );

typedef struct {
	frac16_t v_normal[ 2 ];
	Int16 i_length;
} clipplane_t;

typedef struct {
	UInt8  ui8_wpn_flags;
	UInt8  ui8_weapon;
	UInt16 ui16_ammo_pistol;
	UInt16 ui16_ammo_rifle;
} player_t;

typedef struct {
	Int16 i_plane_y, i_plane_z;
	Int32 i_u_xstep, i_v_xstep, i_tu, i_tv;
} floor_ceiling_cache_entry_t;

#define FLOOR_CEIL_CACHE_SIZE 1
typedef struct {
	floor_ceiling_cache_entry_t rgs_uv_cache[ REND_SCREEN_HEIGHT ][ FLOOR_CEIL_CACHE_SIZE ];
	UInt16 rgi_uv_cache_pos[ REND_SCREEN_HEIGHT ];
} floor_ceiling_cache_t;




typedef struct {
#define EKEY_LEFT  1
#define EKEY_RIGHT 2
#define EKEY_UP    4
#define EKEY_DOWN  8
#define EKEY_1     16
#define EKEY_2     32
#define EKEY_3     64
#define EKEY_4     128
#define EKEY_5     0x100
#define EKEY_6     0x200
#define EKEY_7     0x400
#define EKEY_8     0x800
#define EKEY_9     0x1000
#define EKEY_0     0x2000
	UInt16 ui16_keys1;
#define EKEY_ESC   1
	UInt16 ui16_keys2;
} input_t;

typedef struct {
/*	Int32 i_saved_y_stepv;
	Int16 i_saved_pu; */
	Int16 i_py;
	Int16 i_pheight;
} render_vline_struct_t;

typedef struct {
	Int32 i_z1_sl16;
	Int16 i_v_offset;
	Int16 i16_x_start;
	Int16 i16_x_end;
	UInt8 *pui8_tex;
	UInt8 *pui8_db;
	UInt16 *pui16_ylut;
	Int16 *pi16_ritab;
} render_vline_gen_struct_t;

typedef struct {
	vec2_t v_norm;
	vec2_t t_norm;
	fixed4_t v_dist;
	Int32 i_zbase, i_zx, i_idist;
	Int16 i_ubase, i_ux;
	fixed4_t i_ut;
} render_texture_params_t;

typedef struct {
		vec2_t cv1, cv2, v_solid_vec, v_new_origin;
		Int16 b_verbose;
		Int16 i_sradius;
		Int16 i_entity_height;
		Int16 i_entity_z;
		Int16 i_catch_plane;

		sector_t *p_blocker_sec;
		plane_t *p_catched_plane;
} entity_move_r_args_t;

typedef struct engine_s
{
#define MEMORY_ALLOC_SIZE 60000
	void *p_memory;
	UInt16 ui16_size_for_map;
	void *p_memory_for_map;

	void *p_drawbuffer;

	/* renderer */
	vec3_t origin;
	deg8_t yaw;
	Int16 rgi16_cosine_table[ 256 ];
	char_t rgs_characters[ 0x80 ];
	UInt8 rgui8_big_characters[ 39 * 8 ];
	transform_t world_transform;

	clipplane_t s_clipplanes[ 3 ]; /* left, right, behind */
	Int16 rgi_clipplanes_minmax_idx[ 3 ][ 2 ];
	Int16 i_sector_clipflags;

	UInt8 rgui8_yclip[ REND_SCREEN_WIDTH ][ 2 ];
	UInt16 rgui16_ylut[ DRAWBUFFER_SCREEN_HEIGHT ];
	Int16 rgi16_atan[ REND_SCREEN_HEIGHT ];
	Int16 rgi16_ritab[ REND_SCREEN_HEIGHT ];
	Int16 i_num_cliplist;
	Int16 i_cliplist;
	cliplist_t rgs_cliplist[ REND_SCREEN_WIDTH ];

	render_vline_struct_t rgs_vlines[ REND_SCREEN_WIDTH ];

	/* plane renderer ( floor/ceiling) */
	Int16 i_mark_floor, i_mark_ceiling;
	UInt8 rgui8_ceiling[ REND_SCREEN_WIDTH + 1 ][ 2 ];
	UInt8 rgui8_floor[ REND_SCREEN_WIDTH + 1 ][ 2 ];
	Int16 i_floor_minx, i_floor_maxx;
	Int16 i_ceil_minx, i_ceil_maxx;

	Int16 i_plane_x, i_plane_y, i_plane_z;
	Int16 i_ceiling_z, i_floor_z;
	Int16 i_ceiling_texture, i_floor_texture;
	Int16 i_plane_span_length;
	Int32 i_plane_u_offset;
	Int32 i_plane_v_offset;

	/* sprite renderer */
	UInt8 rgui8_bitmap_translation_tab[ 256 ];
	UInt16 i_num_sprite_clip;
#define MAX_NUM_SPRITECLIP (REND_SCREEN_WIDTH * 8)
	UInt8 ( *pui8_sprite_clip )[ 2 ];
	Int16 i_num_drawsprites;
#define MAX_NUM_DRAWSPRITES 64
	drawsprite_t drawsprites[ MAX_NUM_DRAWSPRITES ];
	UInt8 ui8_viewmodel;


	/* engine */
	UInt8 rgui8_texture_mapping_table[ 256 ];
	UInt8 *pui8_tex;
	UInt8 *pui8_visibility;
	Int16 i_num_node_visibility;
	UInt8 *pui8_node_visibility;

	eng_map_line_segment_t s_map_line;
	eng_line_segment_t s_line_segment;
	floor_ceiling_cache_t s_floor_ceiling_cache;
	sector_t *ps_gsector;
	line_t *ps_gline;

	entity_move_r_args_t s_entity_move_r_args;


	map_t s_map;
	repository_t s_repository;
	texture_cache_t *ps_texture_cache;
	sprite_cache_t *ps_sprite_cache;
#define SCRATCH_SPR_SIZE 2048
	UInt8 *pui8_scratch_spr;

	/* entity to sector to entity mapping */
#define ENTITY_MAPPING_SENTINEL			0xffff
#define MAX_NUM_ENTITIES_IN_SECTOR ( 128 * 4 )
	entities_in_sector_frags_t rgs_entities_in_sector_frags[ MAX_NUM_ENTITIES_IN_SECTOR ];
	UInt16 ui16_free_entities_in_sector_frags;
	UInt16 *pui16_sector_entities;

#define MAX_NUM_SECTORS_OF_ENTITY ( 128 * 4 )
	sectors_of_entity_frags_t rgs_sectors_of_entitiy_frags[ MAX_NUM_SECTORS_OF_ENTITY ];
	UInt16 ui16_free_sectors_of_entity_frags;

	UInt8 ui8_num_entity_cast_entities;
	UInt16 ui16_entity_cast_additional_radius;
#define MAX_ENTITY_CAST_ENTITIES 64 /* overkill */
	UInt8 rgui8_entity_cast_entities[ MAX_ENTITY_CAST_ENTITIES ];

	/* input */
#define MENU_STATE_INGAME      0
#define MENU_STATE_NEW_GAME    1
#define MENU_STATE_QUIT        2
#define MENU_STATE_DIFFICULTY0 3
#define MENU_STATE_DIFFICULTY1 4
#define MENU_STATE_DIFFICULTY2 5
#define MENU_STATE_DIFFICULTY3 6
#define MENU_STATE_CHANGELEVEL 7
	UInt8 ui8_menu_state;
	UInt8 ui8_current_map;
	input_t s_input;

	/* game */
	UInt16 ui16_difficulty;
	UInt16 ui16_time;
	UInt16 ui16_last_time;
	UInt32 ui16_random;

	player_t s_player;
	entity_t *ps_player;

	Int16 i_num_entities;
	entity_t *rgs_entities;
	/*entity_t rgs_entities[ MAX_NUM_ENTITIES ];*/

	UInt8 b_verbose;
	UInt8 ui8_666;
	UInt8 b_godmode;

	UInt8 b_clean_status_bar;

	render_texture_params_t s_tex_params;

} engine_t;

typedef struct {
	UInt16 rgui16_channel_remainder[ 2 ];
	UInt8 rgi8_channel_remain[ 2 ];
	Int8 rgi8_channel_tone[ 2 ];
	Int8 rgi8_channel_sounds[ 2 ];
	Int8 rgi8_channel_sounds_pos[ 2 ];
} tones_t;


#include "spritedef.h"
#include "textures.h"
#include "vspan.h"
#include "spritespan.h"
#include "fpmath.h"
#include "sprites.h"
#include "repository.h"
#include "map.h"
#include "entities.h"
#include "draw.h"
#include "render.h"
#include "soundfx.h"

#if PERFORMANCE_COUNTERS
extern UInt16 i_timer_map;
extern UInt16 i_timer_line_segment;
extern UInt16 i_timer_scan_vspan;
extern UInt16 i_timer_render_vspan;
extern UInt16 i_timer_render_vspan_inner;
extern UInt16 i_timer_scan_floor;
extern UInt16 i_timer_render_floor;
extern UInt16 i_timer_render_floor_inner;
extern UInt16 i_timer_flush_floor;
extern UInt16 i_timer_input;
extern UInt16 i_timer_sprites;
extern UInt16 i_timer_buffer;
#endif


