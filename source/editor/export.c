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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "editor.h"

int g_little_endian_export = 0;

extern editor_t s_ed;

#define MAX_NUM_VERTICES 4096
int i_num_vertices;
short rgi16_vertices[ MAX_NUM_VERTICES ][ 2 ];

#define MAX_NUM_PLANES 2048
int i_num_planes;
unsigned short rgi16_plane_normalvec[ MAX_NUM_PLANES ];
short rgi16_plane_lengths[ MAX_NUM_PLANES ];

#define MAX_NUM_TEXTURES 64
int i_num_textures;
unsigned char rgui8_textures[ MAX_NUM_TEXTURES ][ 16 ];

#define MAX_NUM_LINES 2048
int i_num_lines;
unsigned short rgi16_line_vert1[ MAX_NUM_LINES ];
unsigned short rgi16_line_vert2[ MAX_NUM_LINES ];
unsigned short rgi16_line_plane[ MAX_NUM_LINES ];
unsigned char rgui8_line_flags[ MAX_NUM_LINES ];
csector_t *rgp_line_backsector[ MAX_NUM_LINES ];
unsigned char rgui8_line_tex_upper[ MAX_NUM_LINES ];
unsigned char rgui8_line_tex_lower[ MAX_NUM_LINES ];

#define MAX_NUM_SECTORS 512
int i_num_sectors;
unsigned char rgi16_sector_num_lines[ MAX_NUM_SECTORS ];
unsigned short rgi16_sector_line_index[ MAX_NUM_SECTORS ];
unsigned short rgi16_sector_zvert[ MAX_NUM_SECTORS ];
unsigned char rgui8_sectore_tex_ceil[ MAX_NUM_SECTORS ];
unsigned char rgui8_sectore_tex_floor[ MAX_NUM_SECTORS ];
unsigned short rgi16_sector_visibility[ MAX_NUM_SECTORS ];
unsigned short rgi16_sector_connector[ MAX_NUM_SECTORS ];

#define MAX_NUM_BSP_NODES 2048
int i_num_bsp_nodes;
unsigned short rgi16_bsp_node_plane[ MAX_NUM_BSP_NODES ];
unsigned char rgi16_bsp_node_num_sectors[ MAX_NUM_BSP_NODES ];
unsigned short rgi16_bsp_node_sector_index[ MAX_NUM_BSP_NODES ];
unsigned short rgi16_bsp_node_back_index[ MAX_NUM_BSP_NODES ];
unsigned short rgi16_bsp_node_bbox_min[ MAX_NUM_BSP_NODES ];
unsigned short rgi16_bsp_node_bbox_max[ MAX_NUM_BSP_NODES ];

int i_num_clip_nodes;
int i_num_clip_leafnodes;
int i_entity_clip_tree_start;
unsigned short rgi16_clip_node_plane[ MAX_NUM_BSP_NODES ];
unsigned short rgi16_clip_node_back_index[ MAX_NUM_BSP_NODES ];


#define MAX_SECTOR_VISIBILITIES ( MAX_NUM_SECTORS * ( MAX_NUM_SECTORS / 8 ) )
int i_num_sector_visibility;
unsigned char rgui8_sector_visibility[ MAX_SECTOR_VISIBILITIES ];

/* connectors */
int rgi16_connector_verts[ MAX_NUM_CONNECTORS ];
unsigned char rgui8_connector_num_connection[ MAX_NUM_CONNECTORS ];
int rgi16_connector_connection_offset[ MAX_NUM_CONNECTORS ];

/* connections */
int i_num_connector_connections;
int rgi16_connector_connections[ MAX_NUM_CONNECTORS * MAX_CONNECTOR_CONNECTIONS ];
unsigned char rgui8_connector_connection_distance[ MAX_NUM_CONNECTORS * MAX_CONNECTOR_CONNECTIONS ];


int i_num_entities;
unsigned char rgui8_entity_classname[ MAX_NUM_ENTITIES ];
unsigned short rgui16_entity_origin[ MAX_NUM_ENTITIES ];


extern int i_num_csectors_in_tree;
extern csector_t *p_csectors_in_tree[ 0x10000 ];
extern int i_num_connectors;
extern connector_t rgs_connectors[ MAX_NUM_CONNECTORS ];

int i_max_valid_sector_reference;


int export_get_texture( unsigned char *p_texname )
{
	int i_idx;

	for( i_idx = 0; i_idx < i_num_textures; i_idx++ )
	{
		if( strncmp( (const char *)p_texname, ( const char *)&rgui8_textures[ i_idx ][ 0 ], 16 ) == 0 )
		{
			return i_idx;
		}
	}
	strncpy( ( char * )&rgui8_textures[ i_num_textures ][ 0 ], ( const char * )p_texname, 16 );
	return i_num_textures++;
}


int export_get_vert( vec2_t v )
{
	int i_idx;
	short x, y;

	x = ( short )floor( ( v[ 0 ] * 16 ) + 0.5 );
	y = ( short )floor( ( v[ 1 ] * 16 ) + 0.5 );

	assert( x >= -0x8000 && x < 0x8000 );
	assert( y >= -0x8000 && y < 0x8000 );

	x = x < -0x8000 ? -0x8000 : ( x > 0x7fff ? 0x7fff : x );
	y = y < -0x8000 ? -0x8000 : ( y > 0x7fff ? 0x7fff : y );

	for( i_idx = 0; i_idx < i_num_vertices; i_idx++ )
	{
		if( rgi16_vertices[ i_idx ][ 0 ] == x && rgi16_vertices[ i_idx ][ 1 ] == y )
		{
			return i_idx;
		}
	}
	rgi16_vertices[ i_num_vertices ][ 0 ] = x;
	rgi16_vertices[ i_num_vertices ][ 1 ] = y;
	return i_num_vertices++;

}

int export_get_plane( cplane_t *p_plane )
{
	vec2_t v;
	int i_vert, i_len, i_idx;
	
	v[ 0 ] = ( p_plane->norm[ 0 ] * (2047.7) );
	v[ 1 ] = ( p_plane->norm[ 1 ] * (2047.7) );
	i_vert = export_get_vert( v );
	i_len = floor( p_plane->length * 16 + 0.5 );

	assert( i_len >= -0x8000 && i_len < 0x8000 );

	for( i_idx = 0; i_idx < i_num_planes; i_idx++ )
	{
		if( rgi16_plane_normalvec[ i_idx ] == i_vert &&
			rgi16_plane_lengths[ i_idx ] == i_len )
		{
			return i_idx;
		}
	}
	rgi16_plane_normalvec[ i_num_planes ] = i_vert;
	rgi16_plane_lengths[ i_num_planes ] = i_len;

	return i_num_planes++;
}


void export_line( cline_t *p_line )
{
	int i_line;

	i_line = i_num_lines++;
	rgi16_line_vert1[ i_line ] = export_get_vert( p_line->v1 );
	rgi16_line_vert2[ i_line ] = export_get_vert( p_line->v2 );
	rgi16_line_plane[ i_line ] = export_get_plane( p_line->plane );
	rgui8_line_flags[ i_line ] = p_line->ui8_flags;
	rgp_line_backsector[ i_line ] = p_line->p_backsec;
	rgui8_line_tex_lower[ i_line ] = export_get_texture( &p_line->rgui8_lower_texture[ 0 ] );
	rgui8_line_tex_upper[ i_line ] = export_get_texture( &p_line->rgui8_upper_texture[ 0 ] );
}



void export_sector( csector_t *p_sec )
{
	int i_sector, i_line;
	cline_t *p_line;

	i_sector = i_num_sectors++;
	rgi16_sector_num_lines[ i_sector ] = 0;
	rgi16_sector_line_index[ i_sector ] = i_num_lines;
	rgi16_sector_zvert[ i_sector ] = export_get_vert( p_sec->z );
	rgui8_sectore_tex_floor[ i_sector ] = export_get_texture( &p_sec->rgui8_floor_texture[ 0 ] );
	rgui8_sectore_tex_ceil[ i_sector ] = export_get_texture( &p_sec->rgui8_ceiling_texture[ 0 ] );
	rgi16_sector_connector[ i_sector ] = p_sec->i_connector;
	p_sec->i_secnum = i_sector;
	p_csectors_in_tree[ i_sector ] = p_sec;
	i_num_csectors_in_tree = i_num_sectors;

	for( p_line = p_sec->p_bsp_lines; p_line; p_line = p_line->p_next )
	{
		if( !p_line->just_portal )
		{
			export_line( p_line );
			rgi16_sector_num_lines[ i_sector ]++;
		}
	}
}

int export_bsp_node_r( bspnode_t *p_node )
{
	csector_t *p_secl;
	int i_node;

	i_node = i_num_bsp_nodes++;
	if( p_node->i_type == BSPNODE_LEAF )
	{
		rgi16_bsp_node_plane[ i_node ] = 0xffff;
		rgi16_bsp_node_num_sectors[ i_node ] = 0;
		rgi16_bsp_node_sector_index[ i_node ] = i_num_sectors;
		rgi16_bsp_node_back_index[ i_node ] = p_node->p_sec ? p_node->p_sec->i_sectorid + 1 : 0;
		i_max_valid_sector_reference = MAX( i_max_valid_sector_reference, rgi16_bsp_node_back_index[ i_node ] );

		for( p_secl = p_node->p_sec; p_secl; p_secl = p_secl->p_next )
		{
			assert( p_secl->p_next == NULL );
			export_sector( p_secl );
			rgi16_bsp_node_num_sectors[ i_node ]++;
		}
	}
	else
	{
		rgi16_bsp_node_plane[ i_node ] = export_get_plane( p_node->p_plane );
		rgi16_bsp_node_num_sectors[ i_node ] = 0;
		rgi16_bsp_node_sector_index[ i_node ] = 0;
		export_bsp_node_r( p_node->p_front );
		rgi16_bsp_node_back_index[ i_node ] = export_bsp_node_r( p_node->p_back );
	}
	rgi16_bsp_node_bbox_min[ i_node ] = export_get_vert( p_node->bbox[ 0 ] );
	rgi16_bsp_node_bbox_max[ i_node ] = export_get_vert( p_node->bbox[ 1 ] );

	return i_node;
}

int export_clipnode_r( bspnode_t *p_node )
{
	int i_node;
	i_node = i_num_clip_nodes++;
	p_node->i_nodenum = i_node;
	if( p_node->i_type != BSPNODE_SPLIT )
	{
		rgi16_clip_node_plane[ i_node ] = p_node->p_sec ? p_node->p_sec->i_sectorid : 0;
		i_max_valid_sector_reference = MAX( i_max_valid_sector_reference, rgi16_clip_node_plane[ i_node ] ); 
		rgi16_clip_node_plane[ i_node ] |= 0x8000;

		if( p_node->i_type == BSPNODE_LEAF && p_node->p_sec )
		{
			rgi16_clip_node_back_index[ i_node ] = export_get_vert( p_node->p_sec->z );
		}
		else if( p_node->i_type == BSPNODE_LEAF_SOLID )
		{
			rgi16_clip_node_back_index[ i_node ] = 0xffff;
		}
		else /*if( p_node->i_type == BSPNODE_LEAF_EMPTY )*/
		{
			rgi16_clip_node_back_index[ i_node ] = 0xfffe;
		}
		
	}
	else
	{
		rgi16_clip_node_plane[ i_node ] = export_get_plane( p_node->p_plane );
		export_clipnode_r( p_node->p_front );
		rgi16_clip_node_back_index[ i_node ] = export_clipnode_r( p_node->p_back );
	}
	return i_node;
}

void export_connectors( )
{
	int i_idx, i_idx2;

	for( i_idx = 0; i_idx < i_num_connectors; i_idx++ )
	{
		rgi16_connector_verts[ i_idx ] = export_get_vert( rgs_connectors[ i_idx ].v_origin );
		rgui8_connector_num_connection[ i_idx ] = rgs_connectors[ i_idx ].i_num_connection;
		rgi16_connector_connection_offset[ i_idx ] = i_num_connector_connections;

		for( i_idx2 = 0; i_idx2 < rgs_connectors[ i_idx ].i_num_connection; i_idx2++ )
		{
			rgi16_connector_connections[ i_num_connector_connections ] = rgs_connectors[ i_idx ].rgi_connections[ i_idx2 ];
			rgui8_connector_connection_distance[ i_num_connector_connections++ ] = MIN( 255, (int)( rgs_connectors[ i_idx ].rgf_connections_dist[ i_idx2 ] / 4.0 ) );
		}
	}
}

void prepare_export()
{
	int i_idx;

	i_num_bsp_nodes = i_num_clip_nodes = i_num_clip_leafnodes = i_num_sectors = i_num_lines = i_num_planes = i_num_vertices = i_num_textures = i_num_entities = 0;
	i_max_valid_sector_reference = 0;
	export_bsp_node_r( s_ed.p_tree );
	export_clipnode_r( s_ed.p_cliptree );
	i_entity_clip_tree_start = export_clipnode_r( s_ed.p_entitycliptree );

	i_num_connector_connections = 0;

	for( i_idx = 0; i_idx < s_ed.i_num_entities; i_idx++ )
	{
		rgui8_entity_classname[ i_idx ] = export_get_texture( &s_ed.entities[ i_idx ].rgi8_class_name[ 0 ] );
		rgui16_entity_origin[ i_idx ] = export_get_vert( s_ed.entities[ i_idx ].origin );
	}
	i_num_entities = s_ed.i_num_entities;

	export_connectors();
}


int export_sector_visibility( unsigned char *pui8_visibility, int i_num_vis )
{
	int i_idx = 0;
	int i_start;

	i_start = i_num_sector_visibility;
	
	while( i_idx < i_num_vis )
	{
		unsigned char i_byte, i_bit;
		i_byte = 0;
		for( i_bit = 0; i_bit < MIN( 8, i_num_vis - i_idx ); i_bit++ )
		{
			i_byte |= *(pui8_visibility++) ? 1 << i_bit : 0;
		}
		rgui8_sector_visibility[ i_num_sector_visibility++ ] = i_byte;
		i_idx += i_bit;
	}
	return i_start;
}

void prepare_export_sector_visibility( )
{
	int i_idx;

	i_num_sector_visibility = 0;
	for( i_idx = 0; i_idx < i_num_csectors_in_tree; i_idx++ )
	{
		rgi16_sector_visibility[ i_idx ] = export_sector_visibility( p_csectors_in_tree[ i_idx ]->rgui8_visibility, i_num_csectors_in_tree );
	}

	i_num_sector_visibility = ( i_num_sector_visibility + 1 ) & ~1;
}


void write_i16_file( int i_16, FILE *f )
{
	unsigned char ui8_b;

	if( g_little_endian_export )
	{
		ui8_b = i_16 & 0xff;
		fwrite( &ui8_b, 1, 1, f );
		ui8_b = i_16 >> 8;
		fwrite( &ui8_b, 1, 1, f );
	}
	else
	{
		ui8_b = i_16 >> 8;
		fwrite( &ui8_b, 1, 1, f );
		ui8_b = i_16 & 0xff;
		fwrite( &ui8_b, 1, 1, f );
	}
}

void write_string_file( unsigned char *pui8_str, int i_length, FILE *f )
{
	int i_idx;

	for( i_idx = 0; i_idx < i_length; i_idx++ )
	{
		fwrite( &pui8_str[ i_idx ], 1, 1, f );
	}
}



void export_map( char *fname, int i_little_endian )
{
	int i_idx;
	FILE *f;

	g_little_endian_export = i_little_endian;
#if 0
	f = fopen( fname, "wt" );

	fprintf( f, "vec2_t map_vertices[] = { /* %d vertices */ \n", i_num_vertices );
	for( i_idx = 0; i_idx < i_num_vertices; i_idx++ )
	{
		fprintf(f, "\t{ %d, %d },\n", rgi16_vertices[ i_idx ][ 0 ], rgi16_vertices[ i_idx ][ 1 ] );
	}
	fprintf(f, " };\n\n");

	fprintf( f, "plane_t map_planes[] = { /* %d planes */ \n", i_num_planes );
	for( i_idx = 0; i_idx < i_num_planes; i_idx++ )
	{
		fprintf(f, "\t{ %d, %d },\n", rgi16_plane_normalvec[ i_idx ], rgi16_plane_lengths[ i_idx ] );
	}
	fprintf(f, " };\n\n");

	fprintf( f, "line_t map_lines[] = { /* %d lines */ \n", i_num_lines );
	for( i_idx = 0; i_idx < i_num_lines; i_idx++ )
	{
		fprintf(f, "\t{ %d, %d, %d, 0x%x, %d, %d, %d },\n", rgi16_line_vert1[ i_idx ], rgi16_line_vert2[ i_idx ], rgi16_line_plane[ i_idx ], rgui8_line_flags[ i_idx ], rgp_line_backsector[ i_idx ] ? rgp_line_backsector[ i_idx ]->i_secnum : 0xffff, rgui8_line_tex_upper[ i_idx ], rgui8_line_tex_lower[ i_idx ] );
	}
	fprintf(f, " };\n\n");

	fprintf( f, "unsigned char map_textures[][ 16 ] = { /* %d textures */ \n", i_num_textures );
	for( i_idx = 0; i_idx < i_num_textures; i_idx++ )
	{
		fprintf(f, "\t\"%s\",\n", rgui8_textures[ i_idx ] );
	}
	fprintf( f, " };\n\n" );


	fprintf( f, "sector_t map_sectors[] = { /* %d sectors */ \n", i_num_sectors );
	for( i_idx = 0; i_idx < i_num_sectors; i_idx++ )
	{
		fprintf(f, "\t{ %d, %d, %d, %d, %d, %d, 0 },\n", rgi16_sector_num_lines[ i_idx ], rgi16_sector_line_index[ i_idx ], rgi16_sector_zvert[ i_idx ], rgui8_sectore_tex_ceil[ i_idx ], rgui8_sectore_tex_floor[ i_idx ], rgi16_sector_visibility[ i_idx ] );
	}
	fprintf(f, " };\n\n");


	fprintf( f, "node_t map_nodes[] = { /* %d nodes */ \n", i_num_bsp_nodes );
	for( i_idx = 0; i_idx < i_num_bsp_nodes; i_idx++ )
	{
		fprintf(f, "\t{ %d, %d, %d, %d, %d, %d },\n", rgi16_bsp_node_plane[ i_idx ], rgi16_bsp_node_num_sectors[ i_idx ], rgi16_bsp_node_sector_index[ i_idx ], rgi16_bsp_node_back_index[ i_idx ], rgi16_bsp_node_bbox_min[ i_idx ], rgi16_bsp_node_bbox_max[ i_idx ] );
	}
	fprintf(f, " };\n\n");

	fprintf( f, "mentity_t map_entities[] = { /* %d entities */ \n", i_num_entities );
	for( i_idx = 0; i_idx < i_num_entities; i_idx++ )
	{
		fprintf(f, "\t{ %d, %d },\n", rgui8_entity_classname[ i_idx ], rgui16_entity_origin[ i_idx ] );
	}
	fprintf(f, " };\n\n");

	fprintf( f, "unsigned char map_visibility[] = { /* %d vis */ ", i_num_sector_visibility );
	for( i_idx = 0; i_idx < i_num_sector_visibility; i_idx++ )
	{
		if( ( i_idx & 0xf ) == 0 )
		{
			fprintf( f, "\n");
		}
		fprintf( f, "0x%x,", rgui8_sector_visibility[ i_idx ] );
	}
	fprintf( f, "\n };\n\n");
#else
	f = fopen( fname, "wb" );

	write_i16_file( i_num_vertices, f );
	write_i16_file( i_num_planes, f );
	write_i16_file( i_num_lines, f );
	write_i16_file( i_num_sectors, f );
	write_i16_file( i_num_bsp_nodes, f );
	write_i16_file( i_num_entities, f );
	write_i16_file( i_num_textures, f );
	write_i16_file( i_num_sector_visibility, f );
	write_i16_file( i_num_connectors, f );
	write_i16_file( i_num_connector_connections, f );
	write_i16_file( i_num_clip_nodes, f );
	write_i16_file( i_num_sectors, f ); /* fixme: duplicate */
	write_i16_file( i_entity_clip_tree_start, f );
	write_i16_file( 0, f ); /* padding */

	for( i_idx = 0; i_idx < i_num_vertices; i_idx++ )
	{
		write_i16_file( rgi16_vertices[ i_idx ][ 0 ], f );
		write_i16_file( rgi16_vertices[ i_idx ][ 1 ], f );
	}

	for( i_idx = 0; i_idx < i_num_planes; i_idx++ )
	{
		write_i16_file( rgi16_plane_normalvec[ i_idx ], f );
		write_i16_file( rgi16_plane_lengths[ i_idx ], f );
	}

	for( i_idx = 0; i_idx < i_num_lines; i_idx++ )
	{
		write_i16_file( rgi16_line_vert1[ i_idx ], f );
		write_i16_file( rgi16_line_vert2[ i_idx ], f );
		write_i16_file( rgi16_line_plane[ i_idx ], f );
		write_i16_file( rgp_line_backsector[ i_idx ] ? rgp_line_backsector[ i_idx ]->i_secnum : 0xffff, f );
		write_i16_file( rgui8_line_flags[ i_idx ], f );
		write_string_file( &rgui8_line_tex_upper[ i_idx ], 1, f );
		write_string_file( &rgui8_line_tex_lower[ i_idx ], 1, f );
	}

	
	for( i_idx = 0; i_idx < i_num_sectors; i_idx++ )
	{
		write_i16_file( rgi16_sector_num_lines[ i_idx ], f );
		write_i16_file( rgi16_sector_line_index[ i_idx ], f );
		write_i16_file( rgi16_sector_zvert[ i_idx ], f );
		write_string_file( &rgui8_sectore_tex_ceil[ i_idx ], 1, f );
		write_string_file( &rgui8_sectore_tex_floor[ i_idx ], 1, f );
		write_i16_file( rgi16_sector_connector[ i_idx ], f );
		write_i16_file( rgi16_sector_visibility[ i_idx ], f );
	}


	for( i_idx = 0; i_idx < i_num_bsp_nodes; i_idx++ )
	{
		write_i16_file( rgi16_bsp_node_plane[ i_idx ], f );
		write_i16_file( rgi16_bsp_node_num_sectors[ i_idx ], f );
		write_i16_file( rgi16_bsp_node_sector_index[ i_idx ], f );
		write_i16_file( rgi16_bsp_node_back_index[ i_idx ], f );
		write_i16_file( rgi16_bsp_node_bbox_min[ i_idx ], f );
		write_i16_file( rgi16_bsp_node_bbox_max[ i_idx ], f );
	}

	for( i_idx = 0; i_idx < i_num_entities; i_idx++ )
	{
		write_string_file( &rgui8_entity_classname[ i_idx ], 1, f );
		write_string_file( &rgui8_entity_classname[ i_idx ], 1, f ); /* flags, currently padding */
		write_i16_file( rgui16_entity_origin[ i_idx ], f );
	}

	for( i_idx = 0; i_idx < i_num_textures; i_idx++ )
	{
		write_string_file( rgui8_textures[ i_idx ], 16, f );
	}

	write_string_file( &rgui8_sector_visibility[ 0 ], i_num_sector_visibility, f );

	for( i_idx = 0; i_idx < i_num_connectors; i_idx++ )
	{
		write_i16_file( rgi16_connector_verts[ i_idx ], f );
		write_i16_file ( rgi16_connector_connection_offset[ i_idx ], f );
		write_string_file( &rgui8_connector_num_connection[ i_idx ], 1, f );
		write_string_file( "\0", 1, f ); /* path flags */
		write_i16_file ( 0, f );
		write_i16_file ( 0, f );
	}

	for( i_idx = 0; i_idx < i_num_connector_connections; i_idx++ )
	{
		write_i16_file( rgi16_connector_connections[ i_idx ], f );
	}
	write_string_file( &rgui8_connector_connection_distance[ 0 ], i_num_connector_connections, f );


	for( i_idx = 0; i_idx < i_num_clip_nodes; i_idx++ )
	{
		write_i16_file( rgi16_clip_node_plane[ i_idx ], f );
		write_i16_file( rgi16_clip_node_back_index[ i_idx ], f );
	}

	for( i_idx = 0; i_idx < i_num_sectors; i_idx++ )
	{
		/* null ptr for entity list */
		write_i16_file( 0, f );
	}
#endif

	fclose( f );

}


