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


void spawn_entities( engine_t *ps_eng );
void entities_think( engine_t *ps_eng );

Int16 entity_line_of_sight( engine_t *ps_eng, entity_t *ps_entity, entity_t *ps_other_entity );

Int16 entity_move( engine_t *ps_eng, entity_t *ps_entity, vec3_t v_target, Int16 i_can_slide );

sector_t *eng_get_visibility_sector( engine_t *ps_eng );

#define SPRITE_INDEX_SPECIAL_BEGIN    248

#define SPRITE_INDEX_SPECIAL_SEARGENT 248

Int16 entities_resolve_sprite_index( engine_t *ps_eng, entity_t *ps_entity );

void entities_fixup_player( engine_t *ps_eng, player_t *ps_player );


#define ENTITIES_THINK_STATE_IDLE       0
#define ENTITIES_THINK_STATE_ADVANCE    1
#define ENTITIES_THINK_STATE_FIRE       2
#define ENTITIES_THINK_STATE_WALK       3
#define ENTITIES_THINK_STATE_HIT        4
#define ENTITIES_THINK_STATE_DYING      5
#define ENTITIES_THINK_STATE_DEAD       6


#define SPRITE_ANIM_SEARGENT_WALK0      0
#define SPRITE_ANIM_SEARGENT_WALK1      1
#define SPRITE_ANIM_SEARGENT_WALK2      2
#define SPRITE_ANIM_SEARGENT_WALK3      3
#define SPRITE_ANIM_SEARGENT_FIRE0      4
#define SPRITE_ANIM_SEARGENT_FIRE1      5
#define SPRITE_ANIM_SEARGENT_HIT        6
#define SPRITE_ANIM_SEARGENT_DEATH0     7
#define SPRITE_ANIM_SEARGENT_DEATH1     8
#define SPRITE_ANIM_SEARGENT_DEATH2     9
#define SPRITE_ANIM_SEARGENT_DEATH3     10
#define SPRITE_ANIM_SEARGENT_DEATH4     11

