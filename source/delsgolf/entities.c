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


#include "eng.h"

#define WPN_PISTOL 1
#define WPN_RIFLE  2

typedef struct {
	UInt8 ui8_sprite_idx;
	UInt8 ui8_ticks;
	UInt8 ui8_next;
} sprite_anim_t;

const sprite_anim_t rgs_sprite_anims[] =
{
#define SPRITE_ANIM_PISTOL_IDLE		0
	/*   0 */ { SPRITE_PISTOL_IDLE, 127, 0 },
#define SPRITE_ANIM_PISTOL_FIRE		1
	/*   1 */ { SPRITE_PISTOL_FIRE0, TICKS( 250 ), 2 },
	/*   2 */ { SPRITE_PISTOL_FIRE1, TICKS( 300 ), 0 },
#define SPRITE_ANIM_RIFLE_IDLE		3
	/*   3 */ { SPRITE_RIFLE_IDLE, 127, 3 },
#define SPRITE_ANIM_RIFLE_FIRE		4
	/*   4 */ { SPRITE_RIFLE_FIRE0, TICKS( 100 ), 5 },
	/*   5 */ { SPRITE_RIFLE_FIRE1, TICKS( 100 ), 3 },
/* start from 2nd sprite */
#define SPRITE_ANIM_SG_WALK			7
	/*   6 */ { SPRITE_SG_WALK0_A0, TICKS( 300 ) | 0x80, 7 },
	/*   7 */ { SPRITE_SG_WALK1_A0, TICKS( 300 ) | 0x80, 8 },
	/*   8 */ { SPRITE_SG_WALK2_A0, TICKS( 300 ) | 0x80, 9 },
	/*   9 */ { SPRITE_SG_WALK3_A0, TICKS( 300 ) | 0x80, 6 },
#define SPRITE_ANIM_SG_HIT		   10
	/*  10 */ { SPRITE_SG_HIT_A0, TICKS( 300 ) | 0x80, 6 },
#define SPRITE_ANIM_SG_FIRE		   11
	/*  11 */ { SPRITE_SG_FIRE0, TICKS( 300 ), 12 },
	/*  12 */ { SPRITE_SG_FIRE1, TICKS( 300 ), 6 },
#define SPRITE_ANIM_SG_DEATH	   13
	/*  13 */ { SPRITE_SG_DEATH0, TICKS( 200 ), 14 },
	/*  14 */ { SPRITE_SG_DEATH1, TICKS( 200 ), 15 },
	/*  15 */ { SPRITE_SG_DEATH2, TICKS( 200 ), 16 },
	/*  16 */ { SPRITE_SG_DEATH3, TICKS( 200 ), 17 },
	/*  17 */ { SPRITE_SG_DEATH4, 127, 17 },
};


Int16 line_against_entity_check( engine_t *ps_eng, vec2_t v_origin, vec2_t v_end, entity_t *ps_other_entity, Int16 i_additional_radius );
void eng_clip_interp( Int16 i_cnt, vec3_t v1, Int16 i_dist1, vec3_t v2, Int16 i_dist2 );

static void entity_start_sprite_animation( entity_t *ps_entity, Int16 i16_anim )
{
	ps_entity->ui8_anim_state = ( UInt8 )i16_anim;
	ps_entity->ui8_anim_state_duration = 0;
}

static void entity_run_sprite_animation( entity_t *ps_entity )
{
	const sprite_anim_t *ps_anim;

	ps_entity->ui8_anim_state_duration += 1;

	ps_anim = &rgs_sprite_anims[ ps_entity->ui8_anim_state ];

	if( ps_entity->ui8_anim_state_duration >= ( ps_anim->ui8_ticks & 0x7f ) )
	{
		ps_entity->ui8_anim_state = ps_anim->ui8_next;
		ps_entity->ui8_anim_state_duration = 0;
	}
}




static void player_spawn_spawn( engine_t *ps_eng, entity_t *p_entity, mentity_t *p_mentity )
{
	p_entity->ui_flags = ENTITY_FLAGS_ACTIVE | ENTITY_FLAGS_CLIP;
	ps_eng->s_player.ui8_weapon = WPN_PISTOL;
	ps_eng->s_player.ui8_wpn_flags = WPN_PISTOL;
	ps_eng->s_player.ui16_ammo_rifle = 0;
	p_entity->i_health = 100;
	ps_eng->ps_player = p_entity;
	ps_eng->b_clean_status_bar = FALSE;

	entity_start_sprite_animation( p_entity, SPRITE_ANIM_PISTOL_IDLE );

	p_mentity = NULL;
}

void entities_fixup_player( engine_t *ps_eng, player_t *ps_player )
{
	if( ps_player )
	{
		ps_eng->s_player = *ps_player;
	}
	if( ps_eng->ps_player )
	{
		if( ps_eng->s_player.ui8_weapon == WPN_RIFLE )
		{
			entity_start_sprite_animation( ps_eng->ps_player, SPRITE_ANIM_RIFLE_IDLE );
		}
	}
}

Int16 entities_resolve_sprite_index( engine_t *ps_eng, entity_t *ps_entity )
{
	UInt16 ui16_angle;
	Int16 i_flip = 0;
	Int16 i_sprite_idx;
	vec2_t v_delt;
	Int16 i_anim;

	i_anim = ps_entity->ui8_anim_state;
	i_sprite_idx = rgs_sprite_anims[ i_anim ].ui8_sprite_idx;

	if( rgs_sprite_anims[ i_anim ].ui8_ticks & 0x80 )
	{
		ui16_angle = ps_entity->d8_yaw;
		v_delt[ 0 ] = ps_entity->v_origin[ 0 ] - ps_eng->origin[ 0 ];
		v_delt[ 1 ] = ps_entity->v_origin[ 1 ] - ps_eng->origin[ 1 ];

		ui16_angle = angle_vector( ps_eng, v_delt ) - ui16_angle;
		ui16_angle = ( ui16_angle + ( 1 << 4 ) ) >> 5;
		//printf("ui8_angle: %d\n", ui8_angle );
		ui16_angle &= 7;

		if( ui16_angle < 4 )
		{
			if( ui16_angle > 0 )
			{
				i_flip = 1;
			}
		}
		else
		{
			ui16_angle = 8 - ui16_angle;
		}
		i_sprite_idx += ui16_angle;
	}

	return i_flip ? -i_sprite_idx : i_sprite_idx;
}



Int16 get_entity_through_cast_r( engine_t *ps_eng, Int16 i_node, vec3_t v_start, vec3_t v_end )
{
	Int16 i_dist1, i_dist2;
	Int16 i_result = -2;

	while( 1 )
	{
		if( ps_eng->s_map.p_nodes[ i_node ].i_plane >= 0xfffe ) /* leaf */
		{
			UInt16 i_num_sectors = ps_eng->s_map.p_nodes[ i_node ].i_plane - 0xfffe;
			if( i_num_sectors < 1 )
			{
				return -1;
			}
			else
			{
				UInt16 ui16_sec, ui16_secnum;

				/* check sector entities */
				for( ui16_sec = 0; ui16_sec < i_num_sectors; ui16_sec++ )
				{
					UInt16 ui16_next_entity;
					ui16_secnum = ps_eng->s_map.p_nodes[ i_node ].i_sector_offset + ui16_sec;
					ui16_next_entity = ps_eng->pui16_sector_entities[ ui16_secnum ];
					while( ui16_next_entity != ENTITY_MAPPING_SENTINEL )
					{
						entity_t *ps_entity;

						ps_entity = &ps_eng->rgs_entities[ ps_eng->rgs_entities_in_sector_frags[ ui16_next_entity ].ui8_entity ];
						ui16_next_entity = ps_eng->rgs_entities_in_sector_frags[ ui16_next_entity ].ui16_next_entity;

						if( ps_entity->ui_flags & ENTITY_FLAGS_CLIP &&
							!(ps_entity->ui_flags & ENTITY_FLAGS_ENTITY_CHECKED ) &&
							ps_entity->ui_flags & ENTITY_FLAGS_ACTIVE &&
							line_against_entity_check( ps_eng, v_start, v_end, ps_entity, ps_eng->ui16_entity_cast_additional_radius ) )
						{
							ps_entity->ui_flags |= ENTITY_FLAGS_ENTITY_CHECKED;
							if( ps_eng->ui8_num_entity_cast_entities < MAX_ENTITY_CAST_ENTITIES )
							{
								ps_eng->rgui8_entity_cast_entities[ ps_eng->ui8_num_entity_cast_entities++ ] = ps_entity->ui8_entity_id;
							}
						}
					}
				}
				return 0;
			}
		}
		else
		{
			plane_t *p_plane;
			vec3_t v_split;
			p_plane = &ps_eng->s_map.p_planes[ ps_eng->s_map.p_nodes[ i_node ].i_plane ];
			i_dist1 = map_point_plane_dist( ps_eng, p_plane, v_start );
			i_dist2 = map_point_plane_dist( ps_eng, p_plane, v_end );

			if( i_dist1 > -4 && i_dist2 >= -4 )
			{
				i_node = i_node + 1;
				continue;
			}
			else if( i_dist1 < 4 && i_dist2 < 4 )
			{
				i_node = ps_eng->s_map.p_nodes[ i_node ].i_backnode;
				continue;
			}
			else
			{
				load_vector2v( v_split, v_start );
				eng_clip_interp( 3, v_split, i_dist1, v_end, i_dist2 );
				if( i_dist1 >= 0 )
				{
					i_result = get_entity_through_cast_r( ps_eng, i_node + 1, v_start, v_split );
					if( i_result >= 0 )
					{
						return get_entity_through_cast_r( ps_eng, ps_eng->s_map.p_nodes[ i_node ].i_backnode, v_split, v_end );
					}
					return -1;
				}
				else /* if( i_dist1 < 0 ) */
				{
					i_result = get_entity_through_cast_r( ps_eng, ps_eng->s_map.p_nodes[ i_node ].i_backnode, v_start, v_split );
					if( i_result >= 0 )
					{
						return get_entity_through_cast_r( ps_eng, i_node + 1, v_split, v_end );
					}
					return -1;
				}
			}
			return -1; /* unused */
		}
	}
}

entity_t *get_entity_through_cast( engine_t *ps_eng, entity_t *ps_entity_origin, vec3_t v_end, Int16 i_additional_radius )
{
	Int16 i16_num_cast_entities, i_idx, i_cdist;
	entity_t *ps_rentity;

	i_cdist = 0x7fff;

	ps_eng->ui8_num_entity_cast_entities = 0;
	ps_eng->ui16_entity_cast_additional_radius = i_additional_radius;
	for( i_idx = 0; i_idx < ps_eng->i_num_entities; i_idx++ )
	{
		ps_eng->rgs_entities[ i_idx ].ui_flags &= ~ENTITY_FLAGS_ENTITY_CHECKED;
	}
	get_entity_through_cast_r( ps_eng, 0, ps_entity_origin->v_origin, v_end );

	i16_num_cast_entities = ps_eng->ui8_num_entity_cast_entities;
	ps_rentity = 0;
	for( i_idx = 0; i_idx < i16_num_cast_entities; i_idx++ )
	{
		UInt32 ui32_dist_sqr;
		UInt16 ui16_dist;
		vec2_t v_eedelt;
		entity_t *ps_entity;
		ps_entity = &ps_eng->rgs_entities[ ps_eng->rgui8_entity_cast_entities[ i_idx ] ];
		if( ps_entity == ps_entity_origin )
		{
			continue;
		}

		v_eedelt[ 0 ] = ps_entity->v_origin[ 0 ] - ps_entity_origin->v_origin[ 0 ];
		v_eedelt[ 1 ] = ps_entity->v_origin[ 1 ] - ps_entity_origin->v_origin[ 1 ];
		ui32_dist_sqr = dot_16( v_eedelt, v_eedelt );
		ui16_dist = isqrt( ui32_dist_sqr );
		if( ( UInt16)i_cdist > ui16_dist )
		{
			ps_rentity = ps_entity;
			i_cdist = ui16_dist;
		}
	}

	return ps_rentity;
}


static void player_damage_cast( engine_t *ps_eng, Int16 i16_damage );


static void player_think( engine_t *ps_eng, entity_t *p_entity )
{
	entity_run_sprite_animation( p_entity );
	p_entity->ui8_think_state_duration += 1;

	if( p_entity->ui8_think_state < ENTITIES_THINK_STATE_DYING )
	{
		if( !( ps_eng->ui8_menu_state == MENU_STATE_INGAME ) )
		{
			return; /* player is thinking in menu */
		}

		if( ps_eng->s_input.ui16_keys1 & EKEY_UP )
		{
			vec3_t v_new;
			load_vector( v_new, p_entity->v_origin[ 0 ] + ( ( cos_d8( ps_eng, ps_eng->yaw ) ) >> 9 ), p_entity->v_origin[ 1 ] + ( ( sin_d8( ps_eng, ps_eng->yaw ) ) >> 9 ), p_entity->v_origin[ 2 ] );
			entity_move( ps_eng, p_entity, v_new, 1 );
		}
		else if( ps_eng->s_input.ui16_keys1 & EKEY_DOWN )
		{
			vec3_t v_new;
			load_vector( v_new, p_entity->v_origin[ 0 ] - ( ( cos_d8( ps_eng, ps_eng->yaw ) ) >> 9 ), p_entity->v_origin[ 1 ] - ( ( sin_d8( ps_eng, ps_eng->yaw ) ) >> 9 ), p_entity->v_origin[ 2 ] );
			entity_move( ps_eng, p_entity, v_new, 1 );
		}

		if( ps_eng->s_input.ui16_keys1 & EKEY_LEFT )
		{
			p_entity->d8_yaw = ( p_entity->d8_yaw - 2 );
		}
		if( ps_eng->s_input.ui16_keys1 & EKEY_RIGHT )
		{
			p_entity->d8_yaw = ( p_entity->d8_yaw + 2 );
		}

		if( ps_eng->s_input.ui16_keys1 & ( EKEY_1 | EKEY_2 ) )
		{
			if( ps_eng->s_input.ui16_keys1 & EKEY_1 )
			{
				ps_eng->s_player.ui8_weapon = WPN_PISTOL;
				entity_start_sprite_animation( p_entity, SPRITE_ANIM_PISTOL_IDLE );
			}
			if( ps_eng->s_input.ui16_keys1 & EKEY_2 && ps_eng->s_player.ui16_ammo_rifle >= 2 && ps_eng->s_player.ui8_wpn_flags & WPN_RIFLE )
			{
				ps_eng->s_player.ui8_weapon = WPN_RIFLE;
				entity_start_sprite_animation( p_entity, SPRITE_ANIM_RIFLE_IDLE );
			}
			ps_eng->b_clean_status_bar = FALSE;
		}

		if( ps_eng->s_input.ui16_keys1 & EKEY_0 &&
			( p_entity->ui8_anim_state == SPRITE_ANIM_PISTOL_IDLE ||
			  p_entity->ui8_anim_state == SPRITE_ANIM_RIFLE_IDLE ) )
		{
			p_entity->ui8_anim_state = 1;
			p_entity->ui8_anim_state_duration = 0;
			/* fire */
			if( ps_eng->s_player.ui8_weapon == WPN_RIFLE )
			{
				if( ps_eng->s_player.ui16_ammo_rifle < 2 )
				{
					ps_eng->s_player.ui8_weapon = WPN_PISTOL;
				}
				else
				{
					ps_eng->s_player.ui16_ammo_rifle -= 2;
				}
				ps_eng->b_clean_status_bar = FALSE;
			}
			if( ps_eng->s_player.ui8_weapon == WPN_RIFLE )
			{
				player_damage_cast( ps_eng, WPN_RIFLE );
				entity_start_sprite_animation( p_entity, SPRITE_ANIM_RIFLE_FIRE );
				play_sound( 2, 0 );
			}
			else
			{
				player_damage_cast( ps_eng, WPN_PISTOL );
				entity_start_sprite_animation( p_entity, SPRITE_ANIM_PISTOL_FIRE );
				play_sound( 1, 0 );
			}
		}
		else if( ps_eng->s_input.ui16_keys1 & EKEY_3 )
		{
			player_damage_cast( ps_eng, 0 );
		}

		ps_eng->ui8_viewmodel = rgs_sprite_anims[ p_entity->ui8_anim_state ].ui8_sprite_idx;
	}
	else
	{
		p_entity->i_height -= 3;
		if( p_entity->i_height < 8 )
		{
			p_entity->i_height = 8;
		}
	}
}


static void player_gothit( engine_t *ps_eng, entity_t *ps_player, Int16 i16_damage )
{
	if( !ps_eng->b_godmode )
	{
		ps_player->i_health -= i16_damage;
	}
	if( ps_player->i_health < 0 && ps_player->ui8_think_state < ENTITIES_THINK_STATE_DYING )
	{
		ps_player->ui_flags &= ~( ENTITY_FLAGS_CLIP | ENTITY_FLAGS_ACTIVE );
		ps_player->i_health = 0;
		ps_player->ui8_think_state = ENTITIES_THINK_STATE_DYING;
		play_sound( 5, 1 );
	}
	ps_eng->b_clean_status_bar = FALSE;
}



static void imp_spawn( engine_t *ps_eng, entity_t *p_entity, mentity_t *p_mentity )
{
	p_entity->ui_flags = ENTITY_FLAGS_ACTIVE | ENTITY_FLAGS_DRAW | ENTITY_FLAGS_CLIP;
	p_entity->i_sprite_index = SPRITE_INDEX_SPECIAL_SEARGENT;

	p_entity->ui8_think_state = 0;
	p_entity->ui8_think_state_duration = rand16( ps_eng ) & 7;
	p_entity->i_health = 100;

	entity_start_sprite_animation( p_entity, SPRITE_ANIM_SG_WALK );

	p_mentity = NULL;
}


static void imp_think( engine_t *ps_eng, entity_t *p_entity )
{
	entity_run_sprite_animation( p_entity );

	p_entity->ui8_think_state_duration += 1;

	if( p_entity->ui8_think_state == ENTITIES_THINK_STATE_IDLE )
	{
		if( p_entity->ui8_think_state_duration >= TICKS( 1500 ) )
		{
			p_entity->ui8_think_state_duration = 0;
			if( entity_line_of_sight( ps_eng, p_entity, ps_eng->ps_player ) ) /* as things currently work the imp will not aggro if there is another clipping entity between player and imp */
			{
				p_entity->ui8_think_state = ENTITIES_THINK_STATE_WALK;
				goto imp_think_determine_walk;
			}
		}
	}
	else if( p_entity->ui8_think_state == ENTITIES_THINK_STATE_FIRE )
	{
		if( p_entity->ui8_think_state_duration >= TICKS( 500 ) ) /* should match fire animation */
		{
			p_entity->ui8_think_state_duration = 0;
			p_entity->ui8_think_state = ENTITIES_THINK_STATE_WALK;
			goto imp_think_determine_walk;
		}
	}
	else if( p_entity->ui8_think_state == ENTITIES_THINK_STATE_HIT )
	{
		if( p_entity->ui8_think_state_duration >= TICKS( 200 ) )
		{
			p_entity->ui8_think_state_duration = TICKS( 1600 ); /* almost ready to retaliate */
			p_entity->ui8_think_state = ENTITIES_THINK_STATE_WALK;
			goto imp_think_determine_walk;
		}
	}
	else if( p_entity->ui8_think_state == ENTITIES_THINK_STATE_WALK )
	{
		if( p_entity->ui8_think_state_duration >= TICKS( 2000 ) )
		{
			p_entity->ui8_think_state_duration = 0;
			if( entity_line_of_sight( ps_eng, p_entity, ps_eng->ps_player ) )
			{
				p_entity->ui8_think_state = ENTITIES_THINK_STATE_FIRE;
				entity_start_sprite_animation( p_entity, SPRITE_ANIM_SG_FIRE );
				/* fire */
				if( ps_eng->ui16_difficulty > ( rand16( ps_eng ) & 0xff ) )
				{
					player_gothit( ps_eng, ps_eng->ps_player, 11 );
				}
				play_sound( 3, 0 );
			}
			else
			{
				p_entity->ui8_think_state = ENTITIES_THINK_STATE_IDLE;
			}
		}
		else
		{
			vec3_t v_new;
			load_vector( v_new, p_entity->v_origin[ 0 ] + ( ( cos_d8( ps_eng, p_entity->d8_yaw ) ) >> 12 ), p_entity->v_origin[ 1 ] + ( ( sin_d8( ps_eng, p_entity->d8_yaw ) ) >> 12 ), p_entity->v_origin[ 2 ] );
			entity_move( ps_eng, p_entity, v_new, 0 );
		}
	}
	else if( p_entity->ui8_think_state == ENTITIES_THINK_STATE_DYING )
	{
		if( p_entity->ui8_think_state_duration >= TICKS( 1000 ) )
		{
			p_entity->ui8_think_state = ENTITIES_THINK_STATE_DEAD;
		}
	}

	return;

imp_think_determine_walk:
	{
		UInt8 ui8_angle;
		vec2_t v_delt;

		v_delt[ 0 ] = p_entity->v_origin[ 0 ] - ps_eng->ps_player->v_origin[ 0 ];
		v_delt[ 1 ] = p_entity->v_origin[ 1 ] - ps_eng->ps_player->v_origin[ 1 ];
		ui8_angle = angle_vector( ps_eng, v_delt );
		ui8_angle += ( rand16( ps_eng ) & 0x7f ) - 0x40;
		p_entity->d8_yaw = ui8_angle;
	}
}

void imp_gothit( engine_t *ps_eng, entity_t *ps_entity, Int16 i_damage )
{
	ps_entity->i_health -= i_damage;
	if( ps_entity->i_health < 1 )
	{
		/* dead */
		ps_entity->i_health = 0;
		ps_entity->ui8_think_state = ENTITIES_THINK_STATE_DYING;
		ps_entity->ui_flags &= ~ENTITY_FLAGS_CLIP;
		entity_start_sprite_animation( ps_entity, SPRITE_ANIM_SG_DEATH );
		play_sound( 5, 1 );
	}
	else if( ps_entity->ui8_think_state != ENTITIES_THINK_STATE_FIRE ) /* do not start hit anim if firing */
	{
		ps_entity->ui8_think_state = ENTITIES_THINK_STATE_HIT;
		ps_entity->ui8_think_state_duration = 0;
		entity_start_sprite_animation( ps_entity, SPRITE_ANIM_SG_HIT );
	}
	ps_eng = NULL;
}


static Int16 pickup_in_player_range( engine_t *ps_eng, entity_t *p_entity )
{
	vec2_t v_delt;

	v_delt[ 0 ] = p_entity->v_origin[ 0 ] - ps_eng->ps_player->v_origin[ 0 ];
	v_delt[ 1 ] = p_entity->v_origin[ 1 ] - ps_eng->ps_player->v_origin[ 1 ];

	if( v_delt[ 0 ] < ( -24 << 4 ) || v_delt[ 1 ] < ( -24 << 4 ) ||
		 v_delt[ 0 ] > ( 24 << 4 ) || v_delt[ 1 ] > ( 24 << 4 ) )
	{
		return 0;
	}
	return 1;
}


static void rifle_spawn( engine_t *ps_eng, entity_t *p_entity, mentity_t *p_mentity )
{
	p_entity->ui_flags = ENTITY_FLAGS_ACTIVE | ENTITY_FLAGS_DRAW;
	p_entity->i_sprite_index = SPRITE_RIFLE;
	p_mentity = NULL;
	ps_eng = NULL;
}

static void rifle_think( engine_t *ps_eng, entity_t *p_entity )
{
	if( p_entity->ui_flags & ENTITY_FLAGS_ACTIVE )
	{
		if( pickup_in_player_range( ps_eng, p_entity ) )
		{
			p_entity->ui_flags &= ~( ENTITY_FLAGS_ACTIVE | ENTITY_FLAGS_DRAW );
			if( ( ps_eng->s_player.ui8_wpn_flags & WPN_RIFLE ) == 0 )
			{
				ps_eng->s_player.ui8_weapon = WPN_RIFLE;
				ps_eng->s_player.ui8_wpn_flags |= WPN_RIFLE;
				entity_start_sprite_animation( ps_eng->ps_player, SPRITE_ANIM_RIFLE_IDLE );
			}
			ps_eng->s_player.ui16_ammo_rifle += 15;
			if( ps_eng->s_player.ui16_ammo_rifle > 250 )
			{
				ps_eng->s_player.ui16_ammo_rifle = 250;
			}
			ps_eng->b_clean_status_bar = FALSE;
			play_sound( 4, 1 );
		}
	}
}


static void rflammo_spawn( engine_t *ps_eng, entity_t *p_entity, mentity_t *p_mentity )
{
	p_entity->ui_flags = ENTITY_FLAGS_ACTIVE | ENTITY_FLAGS_DRAW;
	p_entity->i_sprite_index = SPRITE_RFLAMMO;
	p_mentity = NULL;
	ps_eng = NULL;
}

static void rflammo_think( engine_t *ps_eng, entity_t *p_entity )
{
	if( p_entity->ui_flags & ENTITY_FLAGS_ACTIVE )
	{
		if( pickup_in_player_range( ps_eng, p_entity ) )
		{
			if( ps_eng->s_player.ui16_ammo_rifle < 250 )
			{
				p_entity->ui_flags &= ~( ENTITY_FLAGS_ACTIVE | ENTITY_FLAGS_DRAW );
				ps_eng->s_player.ui16_ammo_rifle += 15;
				if( ps_eng->s_player.ui16_ammo_rifle > 250 )
				{
					ps_eng->s_player.ui16_ammo_rifle = 250;
				}
				ps_eng->b_clean_status_bar = FALSE;
				play_sound( 4, 1 );
			}
		}
	}
}

static void mdpck_spawn( engine_t *ps_eng, entity_t *p_entity, mentity_t *p_mentity )
{
	p_entity->ui_flags = ENTITY_FLAGS_ACTIVE | ENTITY_FLAGS_DRAW;
	p_entity->i_sprite_index = SPRITE_MDPCK;
	p_mentity = NULL;
	ps_eng = NULL;
}

static void mdpck_think( engine_t *ps_eng, entity_t *p_entity )
{
	if( p_entity->ui_flags & ENTITY_FLAGS_ACTIVE )
	{
		if( pickup_in_player_range( ps_eng, p_entity ) )
		{
			if( ps_eng->ps_player->i_health < 100 )
			{
				p_entity->ui_flags &= ~( ENTITY_FLAGS_ACTIVE | ENTITY_FLAGS_DRAW );
				ps_eng->ps_player->i_health += 40;
				if( ps_eng->ps_player->i_health > 100 )
				{
					ps_eng->ps_player->i_health = 100;
				}
				ps_eng->b_clean_status_bar = FALSE;
				play_sound( 4, 1 );
			}
		}
	}
}


static void lvltrig_spawn( engine_t *ps_eng, entity_t *p_entity, mentity_t *p_mentity )
{
	p_entity->ui_flags = ENTITY_FLAGS_ACTIVE | ENTITY_FLAGS_DRAW | ENTITY_FLAGS_CLIP;
	p_entity->i_sprite_index = SPRITE_LVLTRIG;
	p_mentity = NULL;
	ps_eng = NULL;
}

static void lvltrig_gothit( engine_t *ps_eng, entity_t *ps_entity, Int16 i_damage )
{
	if( i_damage == 0 )
	{
		ps_eng->ui8_menu_state = MENU_STATE_CHANGELEVEL;
	}
	ps_entity = NULL;
}

static void ship_spawn( engine_t *ps_eng, entity_t *p_entity, mentity_t *p_mentity )
{
	p_entity->ui_flags = ENTITY_FLAGS_ACTIVE | ENTITY_FLAGS_DRAW | ENTITY_FLAGS_CLIP;
	p_entity->i_sprite_index = SPRITE_SHIP;
	p_mentity = NULL;
	ps_eng = NULL;
}

static void ooo_spawn( engine_t *ps_eng, entity_t *p_entity, mentity_t *p_mentity )
{
	p_entity->ui_flags = ENTITY_FLAGS_ACTIVE | ENTITY_FLAGS_DRAW | ENTITY_FLAGS_CLIP;
	p_entity->i_sprite_index = SPRITE_OOO;
	p_mentity = NULL;
	ps_eng = NULL;
}

entityclass_t rgs_entity_classes[] = {
	{ "player_start", 10, 28, player_spawn_spawn, player_think, player_gothit },
	{ "imp", 11, 32, imp_spawn, imp_think, imp_gothit },
	{ "rifle", 14, 12, rifle_spawn, rifle_think, 0 },
	{ "rflammo", 14, 12, rflammo_spawn, rflammo_think, 0 },
	{ "mdpck", 14, 12, mdpck_spawn, mdpck_think, 0 },
	{ "lvltrig", 5, 20, lvltrig_spawn, 0, lvltrig_gothit },
	{ "ship", 80, 20, ship_spawn, 0, 0 },
	{ "ooo", 16, 20, ooo_spawn, 0, 0 },
	{ "end", 0, 0, 0, 0, 0 }
};



static void player_damage_cast( engine_t *ps_eng, Int16 i_damage_type )
{
	entity_t *ps_entity, *ps_entity_hit;
	vec3_t v_end;
	Int16 i_dmg, i_range_shift;

	ps_entity = ps_eng->ps_player;
	if( i_damage_type == WPN_PISTOL )
	{
		i_dmg = 36;
		i_range_shift = 1;
	}
	else if( i_damage_type == WPN_RIFLE )
	{
		i_dmg = 2 * 20;
		i_range_shift = 1;
	}
	else
	{
		i_dmg = 0;
		i_range_shift = 7;
	}

	load_vector( v_end, ps_entity->v_origin[ 0 ] + ( ( cos_d8( ps_eng, ps_eng->yaw ) ) >> i_range_shift ), ps_entity->v_origin[ 1 ] + ( ( sin_d8( ps_eng, ps_eng->yaw ) ) >> i_range_shift ), ps_entity->v_origin[ 2 ] );
	if( v_end[ 0 ] >= 16384 )
	{
		eng_clip_interp( 3, v_end, v_end[ 0 ] - 16384, ps_entity->v_origin, ps_entity->v_origin[ 0 ] - 16384 );
	}
	else if( v_end[ 0 ] <= -16384 )
	{
		eng_clip_interp( 3, v_end, v_end[ 0 ] + 16384, ps_entity->v_origin, ps_entity->v_origin[ 0 ] + 16384 );
	}
	if( v_end[ 1 ] >= 16384 )
	{
		eng_clip_interp( 3, v_end, v_end[ 1 ] - 16384, ps_entity->v_origin, ps_entity->v_origin[ 1 ] - 16384 );
	}
	else if( v_end[ 1 ] <= -16384 )
	{
		eng_clip_interp( 3, v_end, v_end[ 1 ] + 16384, ps_entity->v_origin, ps_entity->v_origin[ 1 ] + 16384 );
	}

	ps_entity_hit = get_entity_through_cast( ps_eng, ps_entity, v_end, 1 );
	if( ps_entity_hit && rgs_entity_classes[ ps_entity_hit->i_entityclass ].f_gothit )
	{
		rgs_entity_classes[ ps_entity_hit->i_entityclass ].f_gothit( ps_eng, ps_entity_hit, i_dmg );
	}
}



void entities_think( engine_t *ps_eng )
{
	Int16 i_idx;

	for( i_idx = 0; i_idx < ps_eng->i_num_entities; i_idx++ )
	{
		entity_t *ps_entity;

		ps_entity = &ps_eng->rgs_entities[ i_idx ];
		if( rgs_entity_classes[  ps_entity->i_entityclass ].f_think )
		{
			rgs_entity_classes[  ps_entity->i_entityclass ].f_think( ps_eng, ps_entity );
		}
	}
}


static void add_entity_to_sector( engine_t *ps_eng, entity_t *ps_entity, UInt16 ui16_sector )
{
	UInt16 ui16_new_entity_in_sector;
	UInt16 ui16_new_sector_for_entity;

	ui16_new_entity_in_sector = ps_eng->ui16_free_entities_in_sector_frags;
	ui16_new_sector_for_entity = ps_eng->ui16_free_sectors_of_entity_frags;

	if( ui16_new_entity_in_sector == 0xffff || ui16_new_sector_for_entity == 0xffff )
	{
#ifdef WIN32
		assert( FALSE );
#endif
	}
	ps_eng->ui16_free_entities_in_sector_frags = ps_eng->rgs_entities_in_sector_frags[ ui16_new_entity_in_sector ].ui16_next_entity;
	ps_eng->ui16_free_sectors_of_entity_frags = ps_eng->rgs_sectors_of_entitiy_frags[ ui16_new_sector_for_entity ].ui16_next_sector;

	ps_eng->rgs_entities_in_sector_frags[ ui16_new_entity_in_sector ].ui16_next_entity = ps_eng->pui16_sector_entities[ ui16_sector ];
	ps_eng->rgs_entities_in_sector_frags[ ui16_new_entity_in_sector ].ui8_entity = ps_entity->ui8_entity_id;
	ps_eng->pui16_sector_entities[ ui16_sector ] = ui16_new_entity_in_sector;


	ps_eng->rgs_sectors_of_entitiy_frags[ ui16_new_sector_for_entity ].ui16_next_sector = ps_entity->ui16_sectors_of_entity;
	ps_eng->rgs_sectors_of_entitiy_frags[ ui16_new_sector_for_entity ].ui16_sector = ui16_sector;
	ps_entity->ui16_sectors_of_entity = ui16_new_sector_for_entity;

}

static void remove_entity_from_sectors( engine_t *ps_eng, entity_t *ps_entity )
{
	UInt16 ui16_next_sector, *pui16_next_entity, ui16_sector, ui16_entity_group_sector;

	ui16_next_sector = ps_entity->ui16_sectors_of_entity;
	ps_entity->ui16_sectors_of_entity = ENTITY_MAPPING_SENTINEL;

	while( ui16_next_sector != ENTITY_MAPPING_SENTINEL )
	{
		ui16_sector = ui16_next_sector;
		ui16_next_sector = ps_eng->rgs_sectors_of_entitiy_frags[ ui16_sector ].ui16_next_sector;
		ui16_entity_group_sector = ps_eng->rgs_sectors_of_entitiy_frags[ ui16_sector ].ui16_sector;

		ps_eng->rgs_sectors_of_entitiy_frags[ ui16_sector ].ui16_next_sector = ps_eng->ui16_free_sectors_of_entity_frags;
		ps_eng->ui16_free_sectors_of_entity_frags = ui16_sector;

		pui16_next_entity = &ps_eng->pui16_sector_entities[ ui16_entity_group_sector ];
		while( *pui16_next_entity != ENTITY_MAPPING_SENTINEL )
		{
			if( ps_eng->rgs_entities_in_sector_frags[ *pui16_next_entity ].ui8_entity == ps_entity->ui8_entity_id )
			{
				UInt16 ui16_free;
				ui16_free = *pui16_next_entity;

				*pui16_next_entity = ps_eng->rgs_entities_in_sector_frags[ *pui16_next_entity ].ui16_next_entity;

				ps_eng->rgs_entities_in_sector_frags[ ui16_free ].ui16_next_entity = ps_eng->ui16_free_entities_in_sector_frags;
				ps_eng->ui16_free_entities_in_sector_frags = ui16_free;
			}
			else
			{
				pui16_next_entity = &ps_eng->rgs_entities_in_sector_frags[ *pui16_next_entity ].ui16_next_entity;
			}
		}
	}
}


void entity_height_placement( engine_t *ps_eng, entity_t *ps_entity )
{
	Int16 i_dist1;
	Int16 i_in_solid;
	Int16 i_node, i_radius;
	Int16 i_max_sector_z, i_min_sector_z;
	plane_t *p_plane;
	Int16 i_num_cnodes;
	UInt16 rgi_cnodes[ 40 ];
	Int16 i_placement_sectors;

	i_radius = ps_entity->i_radius << 4;
	if( i_radius < 16 )
	{
		i_radius = 16;
	}

	i_placement_sectors = 0;
	i_num_cnodes = 0;
	i_node = 0;
	i_in_solid = 0;

	i_max_sector_z = -2048;
	i_min_sector_z = 2048;

	remove_entity_from_sectors( ps_eng, ps_entity );

	/* use drawtree and reuse entity sorting when drawing world */
	while( 1 )
	{
		if( ps_eng->s_map.p_nodes[ i_node ].i_plane >= 0xfffe ) /* leaf */
		{
			UInt16 ui16_sec, ui16_secnum, ui16_zvert;
			UInt16 i_num_sectors = ps_eng->s_map.p_nodes[ i_node ].i_plane - 0xfffe;
			for( ui16_sec = 0; ui16_sec < i_num_sectors; ui16_sec++ )
			{
				ui16_secnum = ps_eng->s_map.p_nodes[ i_node ].i_sector_offset + ui16_sec;
				ui16_zvert = ps_eng->s_map.p_sectors[ ui16_secnum ].i_zvert;

				if( ps_eng->s_map.p_vertices[ ui16_zvert ][ 0 ] > i_max_sector_z )
				{
					i_max_sector_z = ps_eng->s_map.p_vertices[ ui16_zvert ][ 0 ];
				}
				if( ps_eng->s_map.p_vertices[ ui16_zvert ][ 1 ] < i_min_sector_z )
				{
					i_min_sector_z = ps_eng->s_map.p_vertices[ ui16_zvert ][ 1 ];
				}

				{
					add_entity_to_sector( ps_eng, ps_entity, ui16_secnum );
				}
			}
			if( i_num_cnodes > 0 )
			{
				i_num_cnodes--;
				i_node = rgi_cnodes[ i_num_cnodes ];
			}
			else
			{
				break;
			}
		}
		else
		{
			p_plane = &ps_eng->s_map.p_planes[ ps_eng->s_map.p_nodes[ i_node ].i_plane ];
			i_dist1 = map_point_plane_dist( ps_eng, p_plane, ps_entity->v_origin );

			if( i_dist1 > i_radius )
			{
				i_node = i_node + 1;
			}
			else if( i_dist1 < -i_radius )
			{
				i_node = ps_eng->s_map.p_nodes[ i_node ].i_backnode;
			}
			else
			{
				rgi_cnodes[ i_num_cnodes ] = ps_eng->s_map.p_nodes[ i_node ].i_backnode;
				i_node = i_node + 1;
				i_num_cnodes++;
				if( i_num_cnodes >= 40 )
				{
					break;
				}
			}
		}
	}

	/* fixme: midair ? */
	if( !i_in_solid )
	{
		if( ( i_min_sector_z - i_max_sector_z ) < ( Int16 )ps_entity->i_height )
		{
#ifdef WIN32
			printf("entity at %d %d -> height placement ended up with entity top above ceiling\n", ps_entity->v_origin[ 0 ], ps_entity->v_origin[ 1 ] );
#endif
		}
		else
		{
			ps_entity->v_origin[ 2 ] = i_max_sector_z;
		}
	}
}



void spawn_entity( engine_t *ps_eng, mentity_t *p_mentity )
{
	Int16 i_idx, i_class, i_origin;
	entity_t *p_entity;

	i_idx = 0;
	i_class = -1;
	while( rgs_entity_classes[ i_idx ].f_spawn )
	{
		if( strncmp( ps_eng->s_map.rgpui8_textures[ p_mentity->ui8_classname ], rgs_entity_classes[ i_idx ].rgui8_name, 16 ) == 0 )
		{
			i_class = i_idx;
			break;
		}
		i_idx++;
	}
	if( i_class < 0 )
	{
		return;
	}
	p_entity = &ps_eng->rgs_entities[ ps_eng->i_num_entities ];
	memset( p_entity, 0, sizeof( entity_t ) );
	p_entity->ui8_entity_id = ( UInt8 )ps_eng->i_num_entities++;
	p_entity->i_entityclass = i_class;
	i_origin = p_mentity->i_origin;
	p_entity->v_origin[ 0 ] = ps_eng->s_map.p_vertices[ i_origin ][ 0 ];
	p_entity->v_origin[ 1 ] = ps_eng->s_map.p_vertices[ i_origin ][ 1 ];
	p_entity->i_radius = rgs_entity_classes[ i_class ].ui8_radius;
	p_entity->i_height = rgs_entity_classes[ i_class ].ui8_height;
	p_entity->d8_yaw = 0;
	p_entity->ui_flags = 0;
	p_entity->ui8_anim_state = 0;
	p_entity->ui8_anim_state_duration = 0;
	p_entity->ui16_sectors_of_entity = ENTITY_MAPPING_SENTINEL;

	entity_height_placement( ps_eng, p_entity );

	rgs_entity_classes[ i_class ].f_spawn( ps_eng, p_entity, p_mentity );

}

void spawn_entities( engine_t *ps_eng )
{
	Int16 i_idx;

	/* reset entity to sector mapping */
	for( i_idx = 0; i_idx < MAX_NUM_ENTITIES_IN_SECTOR; i_idx++ )
	{
		ps_eng->rgs_entities_in_sector_frags[ i_idx ].ui16_next_entity = i_idx + 1;
	}
	ps_eng->rgs_entities_in_sector_frags[ MAX_NUM_ENTITIES_IN_SECTOR - 1 ].ui16_next_entity = 0xffff;
	ps_eng->ui16_free_entities_in_sector_frags = 0;

	for( i_idx = 0; i_idx < MAX_NUM_SECTORS_OF_ENTITY; i_idx++ )
	{
		ps_eng->rgs_sectors_of_entitiy_frags[ i_idx ].ui16_next_sector = i_idx + 1;
	}
	ps_eng->rgs_sectors_of_entitiy_frags[ MAX_NUM_SECTORS_OF_ENTITY - 1 ].ui16_next_sector = 0xffff;
	ps_eng->ui16_free_sectors_of_entity_frags = 0;

	/* no entity in any sector */
	for( i_idx = 0; i_idx < ps_eng->s_map.i_num_entity_group_sectors; i_idx++ )
	{
		ps_eng->pui16_sector_entities[ i_idx ] = ENTITY_MAPPING_SENTINEL;
	}

	/* spawn entities */
	ps_eng->i_num_entities = 0;
	for( i_idx = 0; i_idx < ps_eng->s_map.i_num_entities; i_idx++ )
	{
		spawn_entity( ps_eng, &ps_eng->s_map.p_entities[ i_idx ] );
	}
}

Int16 entity_move_edge_dist( vec2_t edge_v, vec2_t v_check, vec2_t edge_normal )
{
	Int16 i_dist;
	i_dist = ( ( mul_16_16( edge_v[ 0 ], edge_normal[ 0 ] ) + mul_16_16( edge_v[ 1 ], edge_normal[ 1 ] ) ) >> 15 );
	i_dist = ( ( mul_16_16( v_check[ 0 ], edge_normal[ 0 ] ) + mul_16_16( v_check[ 1 ], edge_normal[ 1 ] ) ) >> 15 ) - i_dist;
	return i_dist;
}




Int32 entity_move_r_clip_f( Int16 i_dist1, Int16 i_dist2, vec2_t v1, vec2_t v2, vec2_t clipv1, vec2_t clipv2 )
{
	vec2_t split;
	if( i_dist1 <= 0 && i_dist2 <= 0 )
	{
		return 0;
	}
	load_vector2v( clipv1, v1 );
	load_vector2v( clipv2, v2 );
	if( i_dist1 >= -3 && i_dist2 >= -3 )
	{
		return 1;
	}

	load_vector2v( split, v1 );
	eng_clip_interp( 2, split, i_dist1, v2, i_dist2 );
	if( i_dist1 <= 0 )
	{
		load_vector2v( clipv1, split );
	}
	else
	{
		load_vector2v( clipv2, split );
	}

	return 2;
}

Int32 entity_move_r_clip_b( Int16 i_dist1, Int16 i_dist2, vec2_t v1, vec2_t v2, vec2_t clipv1, vec2_t clipv2 )
{
	vec2_t split;
	if( i_dist1 >= 0 && i_dist2 >= 0 )
	{
		return 0;
	}
	load_vector2v( clipv1, v1 );
	load_vector2v( clipv2, v2 );
	if( i_dist1 <= 3 && i_dist2 <= 3 )
	{
		return 1;
	}

	load_vector2v( split, v1 );
	eng_clip_interp( 2, split, i_dist1, v2, i_dist2 );
	if( i_dist1 >= 0 )
	{
		load_vector2v( clipv1, split );
	}
	else
	{
		load_vector2v( clipv2, split );
	}

	return 2;
}

Int16 entity_move_r( engine_t *ps_eng, Int16 i_node )
{
	entity_move_r_args_t *ps_entity_move_r_args = &ps_eng->s_entity_move_r_args;
	Int16 i_hit = 0, i_planesplit = 0;
	Int16 i_dist1, i_dist2;
	Int16 i_offset;

	ps_eng->b_verbose = ps_entity_move_r_args->b_verbose;
	while( 1 )
	{
		if( ps_eng->s_map.p_clipnodes[ i_node ].i_plane & 0x8000 ) /* leaf */
		{
			if( ps_eng->s_map.p_clipnodes[ i_node ].i_backnode == 0xffff ) /* outside walkable */
			{
				ps_entity_move_r_args->i_catch_plane = 0;
				load_vector2v( ps_entity_move_r_args->v_solid_vec, ps_entity_move_r_args->cv1 );
				return 1;
			}
			else
			{
				if( ps_eng->s_map.p_clipnodes[ i_node ].i_backnode != 0xfffe )
				{
					if( ps_eng->s_map.p_vertices[ ps_eng->s_map.p_clipnodes[ i_node ].i_backnode ][ 0 ] - ( 8 * 16 ) <= ps_entity_move_r_args->i_entity_z &&
						ps_eng->s_map.p_vertices[ ps_eng->s_map.p_clipnodes[ i_node ].i_backnode ][ 1 ] - ( (int)ps_entity_move_r_args->i_entity_height << 4 ) >= ps_entity_move_r_args->i_entity_z )
					{
			    		if( ps_entity_move_r_args->i_entity_z < ps_eng->s_map.p_vertices[ ps_eng->s_map.p_clipnodes[ i_node ].i_backnode ][ 0 ] )
			    		{
			    			ps_entity_move_r_args->i_entity_z = ps_eng->s_map.p_vertices[ ps_eng->s_map.p_clipnodes[ i_node ].i_backnode ][ 0 ];
			    		}
						return 0;
					}
					else
					{
						ps_entity_move_r_args->i_catch_plane = 0;
						load_vector2v( ps_entity_move_r_args->v_solid_vec, ps_entity_move_r_args->cv1 );
						return 1;
					}
				}
				return 0;
			}
		}
		else
		{
			plane_t *p_plane;

			p_plane = &ps_eng->s_map.p_planes[ ps_eng->s_map.p_clipnodes[ i_node ].i_plane ];
			i_dist1 = map_point_plane_dist( ps_eng, p_plane, ps_entity_move_r_args->cv1 );
			i_dist2 = map_point_plane_dist( ps_eng, p_plane, ps_entity_move_r_args->cv2 );

			if( ps_eng->s_map.p_vertices[ p_plane->i_normal ][ 0 ] == 0 || ps_eng->s_map.p_vertices[ p_plane->i_normal ][ 1 ] == 0 )
			{
				i_offset = ps_entity_move_r_args->i_sradius;
			}
			else
			{
				UInt16 i_scale;

				i_scale = abs( ps_eng->s_map.p_vertices[ p_plane->i_normal ][ 0 ] >> 7 ) + abs( ps_eng->s_map.p_vertices[ p_plane->i_normal ][ 1 ] >> 7 );
				i_scale += 0;
				i_offset = mul_u16_u16( i_scale, ps_entity_move_r_args->i_sradius ) >> 8;
			}

			if( i_dist1 >= i_offset && i_dist2 >= i_offset )
			{
				i_node = i_node + 1;
				continue;
			}
			else if( i_dist1 <= -i_offset && i_dist2 <= -i_offset )
			{
				i_node = ps_eng->s_map.p_clipnodes[ i_node ].i_backnode;
				continue;
			}
			else
			{
				Int32 i_lseg, i_hit_seg;
				vec2_t ocv1, ocv2;
				vec3_t v_splitd[ 2 ];

				i_offset += 2;

				load_vector2v( ocv1, ps_entity_move_r_args->cv1 );
				load_vector2v( ocv2, ps_entity_move_r_args->cv2 );
				if( ( i_dist1 < 0 && i_dist1 > i_dist2 ) || ( i_dist1 >= 0 && i_dist1 > i_dist2 ) )
				{
					i_lseg = entity_move_r_clip_f( i_dist1 + i_offset, i_dist2 + i_offset, ocv1, ocv2, v_splitd[ 0 ], v_splitd[ 1 ] );
					if( i_lseg )
					{
						ps_entity_move_r_args->cv1[ 0 ] = v_splitd[ 0 ][ 0 ];
						ps_entity_move_r_args->cv1[ 1 ] = v_splitd[ 0 ][ 1 ];
						ps_entity_move_r_args->cv2[ 0 ] = v_splitd[ 1 ][ 0 ];
						ps_entity_move_r_args->cv2[ 1 ] = v_splitd[ 1 ][ 1 ];
						i_hit = entity_move_r( ps_eng, i_node + 1 );
						if( i_hit )
						{
							load_vector2v( ocv2, ps_entity_move_r_args->v_solid_vec );
							i_dist2 = map_point_plane_dist( ps_eng, p_plane, ocv2 );
						}
					}
					if( 1 )
					{
						i_lseg = entity_move_r_clip_b( i_dist1 - i_offset, i_dist2 - i_offset, ocv1, ocv2, v_splitd[ 0 ], v_splitd[ 1 ] );
						if( i_lseg )
						{
							ps_entity_move_r_args->cv1[ 0 ] = v_splitd[ 0 ][ 0 ];
							ps_entity_move_r_args->cv1[ 1 ] = v_splitd[ 0 ][ 1 ];
							ps_entity_move_r_args->cv2[ 0 ] = v_splitd[ 1 ][ 0 ];
							ps_entity_move_r_args->cv2[ 1 ] = v_splitd[ 1 ][ 1 ];
							i_hit |= i_hit_seg = entity_move_r( ps_eng, ps_eng->s_map.p_clipnodes[ i_node ].i_backnode );
							if( i_hit_seg )
							{
								i_hit_seg = map_point_plane_dist( ps_eng, p_plane, ps_entity_move_r_args->v_solid_vec );
								if( i_hit_seg - i_offset < 4 && i_hit_seg - i_offset > -4 )
									i_planesplit = 1;
							}
						}
					}
				}
				else
				{
					i_lseg = entity_move_r_clip_b( i_dist1 - i_offset, i_dist2 - i_offset, ocv1, ocv2, v_splitd[ 0 ], v_splitd[ 1 ] );
					if( i_lseg )
					{
						ps_entity_move_r_args->cv1[ 0 ] = v_splitd[ 0 ][ 0 ];
						ps_entity_move_r_args->cv1[ 1 ] = v_splitd[ 0 ][ 1 ];
						ps_entity_move_r_args->cv2[ 0 ] = v_splitd[ 1 ][ 0 ];
						ps_entity_move_r_args->cv2[ 1 ] = v_splitd[ 1 ][ 1 ];
						i_hit = entity_move_r( ps_eng, ps_eng->s_map.p_clipnodes[ i_node ].i_backnode );
						if( i_hit )
						{
							load_vector2v( ocv2, ps_entity_move_r_args->v_solid_vec );
							i_dist2 = map_point_plane_dist( ps_eng, p_plane, ocv2 );
						}
					}
					if( 1 )
					{
						i_lseg = entity_move_r_clip_f( i_dist1 + i_offset, i_dist2 + i_offset, ocv1, ocv2, v_splitd[ 0 ], v_splitd[ 1 ] );
						if( i_lseg )
						{
							ps_entity_move_r_args->cv1[ 0 ] = v_splitd[ 0 ][ 0 ];
							ps_entity_move_r_args->cv1[ 1 ] = v_splitd[ 0 ][ 1 ];
							ps_entity_move_r_args->cv2[ 0 ] = v_splitd[ 1 ][ 0 ];
							ps_entity_move_r_args->cv2[ 1 ] = v_splitd[ 1 ][ 1 ];
							i_hit |= i_hit_seg = entity_move_r( ps_eng, i_node + 1 );
							if( i_hit_seg )
							{
								i_hit_seg = map_point_plane_dist( ps_eng, p_plane, ps_entity_move_r_args->v_solid_vec );
								if( i_hit_seg + i_offset < 4 && i_hit_seg + i_offset > -4 )
									i_planesplit = 1;
							}
						}
					}
				}

				if( i_hit && i_planesplit )
				{
					ps_entity_move_r_args->p_catched_plane = p_plane;
					ps_entity_move_r_args->i_catch_plane = 1;
				}
				return i_hit;
			}
		}
	}
}

Int16 line_against_entity_check( engine_t *ps_eng, vec2_t v_origin, vec2_t v_end, entity_t *ps_other_entity, Int16 i_additional_radius )
{
	entity_move_r_args_t *ps_entity_move_r_args = &ps_eng->s_entity_move_r_args;
	vec3_t v1;
	Int16 i16_entity_radius;
	vec2_t v_entityspace_v1, v_entityspace_v2;
	Int16 i_dist;

	i16_entity_radius = i_additional_radius + ( ps_other_entity->i_radius << 4 );
	v_entityspace_v1[ 0 ] = v_origin[ 0 ] - ps_other_entity->v_origin[ 0 ];
	v_entityspace_v1[ 1 ] = v_origin[ 1 ] - ps_other_entity->v_origin[ 1 ];

	v_entityspace_v2[ 0 ] = v_end[ 0 ] - ps_other_entity->v_origin[ 0 ];
	v_entityspace_v2[ 1 ] = v_end[ 1 ] - ps_other_entity->v_origin[ 1 ];

	load_vector2v( v1, v_entityspace_v1 );
	load_vector2v( ps_entity_move_r_args->cv1, v_entityspace_v1 );
	load_vector2v( ps_entity_move_r_args->cv2, v_entityspace_v2 );

	ps_entity_move_r_args->i_sradius = i16_entity_radius;
	ps_entity_move_r_args->i_entity_height = 0;
	ps_entity_move_r_args->i_entity_z = 0;

	i_dist = entity_move_r( ps_eng, ps_eng->s_map.i_entity_clip_tree_root );


	return i_dist;
}

Int16 entity_move( engine_t *ps_eng, entity_t *ps_entity, vec3_t v_target, Int16 i_can_slide )
{
	entity_move_r_args_t *ps_entity_move_r_args = &ps_eng->s_entity_move_r_args;
	vec3_t v1, v2;
	Int16 i_idx, i_blocked, i_counter, i_iter;
	Int16 i_node, i_radius;
	plane_t *p_plane;
	Int16 i_num_cnodes;
	Int16 i_clip_sectors;
	vec2_t v_clipped_move, v_original_move_normal = { 0, };
	Int32 i_moved_dist;

	i_radius = ps_entity->i_radius << 4;
	i_radius += 8;
	if( i_radius < 16 )
	{
		i_radius = 16;
	}

	v_clipped_move[ 0 ] = v_target[ 0 ] - ps_entity->v_origin[ 0 ];
	v_clipped_move[ 1 ] = v_target[ 1 ] - ps_entity->v_origin[ 1 ];

	i_counter = 0;
	i_iter = 0;
	while( v_clipped_move[ 0 ] || v_clipped_move[ 1 ] )
	{
		Int16 i_new_z;
		vec3_t v_target_origin;

		i_clip_sectors = 0;
		i_num_cnodes = 0;
		i_node = 0;
		i_blocked = 0;

		//DrawStr( 24, 20, "X", A_REPLACE );

		load_vector2v( v1, ps_entity->v_origin );
		load_vector2v( ps_entity_move_r_args->cv1, ps_entity->v_origin );

		v2[ 0 ] = v1[ 0 ] + v_clipped_move[ 0 ];
		v2[ 1 ] = v1[ 1 ] + v_clipped_move[ 1 ];
		load_vector2v( v_target_origin, v2 );

		load_vector2v( ps_entity_move_r_args->cv2, v2 );
		ps_entity_move_r_args->i_sradius = i_radius;
		ps_entity_move_r_args->i_entity_height = ps_entity->i_height;
		ps_entity_move_r_args->i_entity_z = ps_entity->v_origin[ 2 ];


		ps_entity_move_r_args->p_catched_plane = 0;
		ps_entity_move_r_args->b_verbose = i_can_slide;

		i_moved_dist = entity_move_r( ps_eng, 0 );

		i_new_z = ps_entity_move_r_args->i_entity_z;

		if( i_moved_dist )
		{
			load_vector2v( ps_entity_move_r_args->v_new_origin, ps_entity_move_r_args->v_solid_vec );
		}
		else
		{
			load_vector2v( ps_entity_move_r_args->v_new_origin, ps_entity_move_r_args->cv2 );
		}

		{
			for( i_idx = 0; i_idx < ps_eng->i_num_entities; i_idx++ )
			{
				if( ps_eng->rgs_entities[ i_idx ].ui_flags & ENTITY_FLAGS_ACTIVE &&
					ps_eng->rgs_entities[ i_idx ].ui_flags & ENTITY_FLAGS_CLIP &&
					i_idx != ps_entity->ui8_entity_id )
				{

					i_moved_dist = line_against_entity_check( ps_eng, ps_entity->v_origin, ps_entity_move_r_args->v_new_origin, &ps_eng->rgs_entities[ i_idx ], i_radius );
					if( i_moved_dist )
					{
						entity_t *ps_other_entity;
						ps_other_entity = &ps_eng->rgs_entities[ i_idx ];

						load_vector2v( ps_entity_move_r_args->v_new_origin, ps_entity_move_r_args->v_solid_vec );
						ps_entity_move_r_args->v_new_origin[ 0 ] += ps_other_entity->v_origin[ 0 ];
						ps_entity_move_r_args->v_new_origin[ 1 ] += ps_other_entity->v_origin[ 1 ];
					}
				}
			}
		}

		if( i_can_slide && ps_entity_move_r_args->p_catched_plane )
		{
			v_clipped_move[ 0 ] = v_target_origin[ 0 ] - ps_entity_move_r_args->v_new_origin[ 0 ];
			v_clipped_move[ 1 ] = v_target_origin[ 1 ] - ps_entity_move_r_args->v_new_origin[ 1 ];
		}
		else
		{
			v_clipped_move[ 0 ] = 0;
			v_clipped_move[ 1 ] = 0;
		}

		/* convert to slide along blocker if movement left ( check against original movement normal for later slides )*/
		if( ( v_clipped_move[ 0 ] != 0 || v_clipped_move[ 1 ] != 0 ) )
		{
			UInt16 i_scale;
			Int32 i_angle, i_move_length;
			Int32 v_move_normal[ 2 ];

			p_plane = ps_entity_move_r_args->p_catched_plane;

			i_move_length = isqrt( ( mul_16_16( v_clipped_move[ 0 ], v_clipped_move[ 0 ] ) + mul_16_16( v_clipped_move[ 1 ], v_clipped_move[ 1 ] ) ) );
			i_move_length = ( i_move_length << 1 ) + 1;
			v_move_normal[ 0 ] = div_32_u16( ( ( ( Int32 )v_clipped_move[ 0 ] ) << 16 ), i_move_length );
			v_move_normal[ 1 ] = div_32_u16( ( ( ( Int32 )v_clipped_move[ 1 ] ) << 16 ), i_move_length );

			if( v_move_normal[ 0 ] > 0x7fffL )
			{
				v_move_normal[ 0 ] = 0x7fffL;
			}
			if( v_move_normal[ 1 ] > 0x7fffL )
			{
				v_move_normal[ 1 ] = 0x7fffL;
			}

			i_angle = mul_16_16( ps_eng->s_map.p_vertices[ p_plane->i_normal ][ 0 ], v_move_normal[ 0 ] ) + mul_16_16( ps_eng->s_map.p_vertices[ p_plane->i_normal ][ 1 ], v_move_normal[ 1 ] );
			i_angle = ( ( -i_angle ) >> 14 ) + 1;

			if( i_angle > 0xff00 )
			{
				/* movement vector on normal, decimate completly */
				v_clipped_move[ 0 ] = 0;
				v_clipped_move[ 1 ] = 0;
			}
			else
			{
				i_scale = ( mul_32_u16( i_angle, ( UInt16 )i_move_length ) );
				i_scale += i_angle < 0 ? -3 : 3; /* make sure we dont bump into it again */
				v_clipped_move[ 0 ] += mul_16_16( ps_eng->s_map.p_vertices[ p_plane->i_normal ][ 0 ], i_scale ) >> 16;
				v_clipped_move[ 1 ] += mul_16_16( ps_eng->s_map.p_vertices[ p_plane->i_normal ][ 1 ], i_scale ) >> 16;
			}
			if( i_iter == 0 )
			{
				load_vector2( v_original_move_normal, v_move_normal[ 0 ], v_move_normal[ 1 ] );
			}
			else
			{
				i_angle = mul_16_16( v_clipped_move[ 0 ], v_original_move_normal[ 0 ] ) + mul_16_16( v_clipped_move[ 1 ], v_original_move_normal[ 1 ] );

				if( i_angle <= 0 )
				{
					/* dont slide in circles */
					v_clipped_move[ 0 ] = 0;
					v_clipped_move[ 1 ] = 0;
				}
			}
		}

		ps_entity->v_origin[ 0 ] = ps_entity_move_r_args->v_new_origin[ 0 ];
		ps_entity->v_origin[ 1 ] = ps_entity_move_r_args->v_new_origin[ 1 ];
		ps_entity->v_origin[ 2 ] = i_new_z;
		/*
		remove_entity_from_sector( ps_entity );
		sort_entity_into_sector( ps_entity );
		*/

		i_iter++;

		/* sadly, due to limited precision we can repeat twice or so */
		if( i_iter > 3 )
		{
			break;
		}
	}


	entity_height_placement( ps_eng, ps_entity );

/*
	if( i_counter > 0 )
	{
		unsigned i_start, i_end;
		eng_draw_commit_drawbuffer( );

		i_start = eng_get_time();
		while( eng_get_time() - i_start < 2000 )
		{

		}
	}
*/
	return i_blocked;
}

/* specialized function removed due to executable size constraints */
#if 0
Int16 entity_line_of_sight_r( Int16 i_node, vec3_t v_v1, vec3_t v_v2 )
{
	while( 1 )
	{
		if( ps_eng->s_map.p_clipnodes[ i_node ].i_plane & 0x8000 ) /* leaf */
		{
			if( ps_eng->s_map.p_clipnodes[ i_node ].i_backnode == 0xffff ) /* outside walkable */
			{
				return 0;
			}
			else
			{
				return 1;
			}
		}
		else
		{
			plane_t *p_plane;
			Int16 i_dist1, i_dist2;

			p_plane = &ps_eng->s_map.p_planes[ ps_eng->s_map.p_clipnodes[ i_node ].i_plane ];
			ps_eng->s_map.p_vertices[ p_plane->i_normal ];
			i_dist1 = map_point_plane_dist( ps_eng, p_plane, v_v1 );
			i_dist2 = map_point_plane_dist( ps_eng, p_plane, v_v2 );

			if( i_dist1 >= -2 && i_dist2 >= -2 )
			{
				i_node = i_node + 1;
				continue;
			}
			else if( i_dist1 <= 2 && i_dist2 <= 2 )
			{
				i_node = ps_eng->s_map.p_clipnodes[ i_node ].i_backnode;
				continue;
			}
			else
			{
				Int16 i_front, i_back;
				vec3_t v_split;
				load_vector2v( v_split, v_v1 );
				eng_clip_interp( v_split, i_dist1, v_v2, i_dist2 );

				if( i_dist1 > 0 )
				{
					if( entity_line_of_sight_r( i_node + 1, v_v1, v_split ) )
					{
						return entity_line_of_sight_r( ps_eng->s_map.p_clipnodes[ i_node ].i_backnode, v_split, v_v2 );
					}
				}
				else
				{
					if( entity_line_of_sight_r( ps_eng->s_map.p_clipnodes[ i_node ].i_backnode, v_v1, v_split ) )
					{
						return entity_line_of_sight_r( i_node + 1, v_split, v_v2 );
					}
				}
				return 0;
			}
		}
	}
	return 0;
}

#endif

Int16 entity_line_of_sight( engine_t *ps_eng, entity_t *ps_entity, entity_t *ps_other_entity )
{
	entity_t *ps_entity_hit;
	Int16 i_idx;
	UInt16 ui_sector_of_entity, ui_sector_of_other_entity;
	ps_eng->ui8_num_entity_cast_entities = 0;
	ps_eng->ui16_entity_cast_additional_radius = 0;


	ui_sector_of_other_entity = ps_other_entity->ui16_sectors_of_entity;
	while( ui_sector_of_other_entity != ENTITY_MAPPING_SENTINEL )
	{
		Int16 ui_secnum, ui_sec_idx, ui_sec_bit_idx;

		ui_secnum = ps_eng->rgs_sectors_of_entitiy_frags[ ui_sector_of_other_entity ].ui16_sector;
		ui_sec_idx = ui_secnum >> 3;
		ui_sec_bit_idx = ( 1 << ( ui_secnum & 0x7 ) );

		ui_sector_of_entity = ps_entity->ui16_sectors_of_entity;
		while( ui_sector_of_entity != ENTITY_MAPPING_SENTINEL )
		{
			sector_t *ps_sec;

			ps_sec = &ps_eng->s_map.p_sectors[ ps_eng->rgs_sectors_of_entitiy_frags[ ui_sector_of_entity ].ui16_sector ];
			if( ( &ps_eng->s_map.pui8_visibility[ ps_sec->i_visibility ] )[ ui_sec_idx ] & ui_sec_bit_idx )
			{
				for( i_idx = 0; i_idx < ps_eng->i_num_entities; i_idx++ )
				{
					ps_eng->rgs_entities[ i_idx ].ui_flags &= ~ENTITY_FLAGS_ENTITY_CHECKED;
				}

				ps_entity_hit = get_entity_through_cast( ps_eng, ps_entity, ps_other_entity->v_origin, 1 );

				if( ps_entity_hit && ps_entity_hit->ui8_entity_id == ps_other_entity->ui8_entity_id )
				{
					return 1;
				}
				return 0;
			}
			ui_sector_of_entity = ps_eng->rgs_sectors_of_entitiy_frags[ ui_sector_of_entity ].ui16_next_sector;
		}
		ui_sector_of_other_entity = ps_eng->rgs_sectors_of_entitiy_frags[ ui_sector_of_other_entity ].ui16_next_sector;
	}

	return 0;
}


sector_t *eng_get_visibility_sector( engine_t *ps_eng )
{
	Int16 i_dist;
	UInt16 i_node;
	UInt16 i_sec, i_secnum;
	plane_t *p_plane;

	i_node = 0;

	while( 1 )
	{
		if( ps_eng->s_map.p_nodes[ i_node ].i_plane >= 0xfffe ) /* leaf */
		{
			UInt16 i_num_sectors = ps_eng->s_map.p_nodes[ i_node ].i_plane - 0xfffe;

			for( i_sec = 0; i_sec < i_num_sectors; i_sec++ )
			{
				i_secnum = ps_eng->s_map.p_nodes[ i_node ].i_sector_offset + i_sec;
				return &ps_eng->s_map.p_sectors[ i_secnum ];
			}
			break;
		}
		else
		{
			p_plane = &ps_eng->s_map.p_planes[ ps_eng->s_map.p_nodes[ i_node ].i_plane ];

			i_dist = map_point_plane_dist( ps_eng, p_plane, ps_eng->origin );

			if( i_dist >= 0 )
			{
				i_node = i_node + 1;
			}
			else
			{
				i_node = ps_eng->s_map.p_nodes[ i_node ].i_backnode;
			}
		}
	}
	return 0;
}
