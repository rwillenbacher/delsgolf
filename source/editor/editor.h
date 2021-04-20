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

#define MAX_SECTOR_VERTS 32
#define MAX_NUM_SECTORS  512
#define MAX_NUM_ENTITIES 512

#define MIN( x, y ) ( (x) < (y) ? (x) : (y) )
#define MAX( x, y ) ( (x) > (y) ? (x) : (y) )

typedef float vec2_t[ 2 ];

typedef struct {
	char rgi8_texture_upper[ 64 ];
	char rgi8_texture_lower[ 64 ];
} line_t;

typedef struct {
	int i_numverts;
	vec2_t z;
	vec2_t verts[ MAX_SECTOR_VERTS ];
	line_t lines[ MAX_SECTOR_VERTS ];
	char rgui8_texture_floor[ 64 ];
	char rgui8_texture_ceiling[ 64 ];
}sector_t;

typedef struct {
	char rgi8_class_name[ 64 ];
	vec2_t origin;
}entity_t;

typedef struct cplane_s {
	struct cplane_s *p_next;
	int i_planenum;
	vec2_t norm;
	float length;
} cplane_t;

#define MAX_CONNECTOR_CONNECTIONS 16
#define MAX_NUM_CONNECTORS 2048

typedef struct connector_s {
	vec2_t v_origin;
	float f_radius;
	int i_num_connection;
	int rgi_connections[ MAX_CONNECTOR_CONNECTIONS ];
	float rgf_connections_dist[ MAX_CONNECTOR_CONNECTIONS ];

	/* pathfinding test vars */
	float f_path_length;
	int i_path_previous;
	int i_path_handled;
} connector_t;

typedef struct cline_s {
	struct cline_s *p_next;
	struct csector_s *p_backsec;
	int onsplitplane;
	int just_portal;
	char rgui8_upper_texture[ 64 ];
	char rgui8_lower_texture[ 64 ];
	cplane_t *plane;
	vec2_t v1, v2;

#define LINEFLAG_TEX_Y   1
#define LINEFLAG_PATH    2
#define LINEFLAG_UPPER   4
#define LINEFLAG_LOWER   8
#define LINEFLAG_MIDDLE  16

	unsigned char ui8_flags;

} cline_t;

typedef struct csector_s {
	struct csector_s *p_next;
	int i_secnum;
	int i_sectorid; /* right right, to keep memory consumption down and to sync draw and clip tree we note the original sector idx which is used to group entities together in the engine */
	vec2_t z;
	char rgui8_ceiling_texture[ 64 ];
	char rgui8_floor_texture[ 64 ];
	int i_num_vertices;
	vec2_t v_vertices[ MAX_SECTOR_VERTS ];
	cline_t rgs_lines[ MAX_SECTOR_VERTS ];
	cline_t *p_bsp_lines;
	int i_has_visibility;
	unsigned char rgui8_visibility[ MAX_NUM_SECTORS ];
	struct bspnode_s *p_node;

	int i_connector;
} csector_t;

typedef struct bspnode_s {
#define BSPNODE_SPLIT 1
#define BSPNODE_LEAF  2
#define BSPNODE_LEAF_SOLID 3
#define BSPNODE_LEAF_EMPTY 4
	int i_type;
	int i_nodenum;
	int i_leafnum;
	cplane_t *p_plane;
	struct bspnode_s *p_parent;
	struct bspnode_s *p_front;
	struct bspnode_s *p_back;
	csector_t *p_sec;
	int i_valid_bbox;
	vec2_t bbox[ 2 ];
	int i_no_portal;
	struct portal_s *p_portals;
} bspnode_t;

typedef struct portal_s {
	struct portal_s *p_nextf, *p_nextb;
	vec2_t v1, v2;
	cplane_t s_plane;
	bspnode_t *p_front;
	bspnode_t *p_back;
	int i_connector;
} portal_t;

typedef struct {
	vec2_t origin;
	vec2_t vorigin;
	int i_draw_path_queued;
	vec2_t test_path_start;
	vec2_t test_path_end;
	int i_num_sectors;
	sector_t sectors[ MAX_NUM_SECTORS ];
	int i_num_entities;
	entity_t entities[ MAX_NUM_ENTITIES ];

	int e2d_width, e2d_height;

	int i_grab_keyboard;
	int e2d_mx, e2d_my;
	int i_snap;
	int i_selected_sector;
	int i_selected_line;
	int i_selected_vert;
	int i_selected_entity;

	bspnode_t *p_tree;
	bspnode_t *p_cliptree;
	bspnode_t *p_entitycliptree;
	int i_num_valid_sectors;
	cplane_t *p_planelist;

	int i_which_view;
}editor_t;

editor_t s_ed;


