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

void map_nodes_set_sectorlink_r( map_t *ps_map, Int16 i_node, Int16 i_parent_node )
{
	ps_map->p_nodes[ i_node ].i_parent_node = i_parent_node;
	if( ps_map->p_nodes[ i_node ].i_plane < 0xfffe )
	{
		map_nodes_set_sectorlink_r( ps_map, i_node + 1, i_node );
		map_nodes_set_sectorlink_r( ps_map, ps_map->p_nodes[ i_node ].i_backnode, i_node );
	}
}

Int16 map_load( engine_t *ps_eng, Int16 i16_map )
{
	Int16 i16_idx;
	void *p_loaded_map;
	map_t *ps_map, *ps_rmap_header;
	repository_map_t *ps_rmap;

	ps_rmap = &ps_eng->s_repository.rgs_maps[ i16_map ];

	if( ps_eng->ui16_size_for_map < ps_rmap->i_map_length )
	{
		return 0;
	}

	p_loaded_map = ps_eng->p_memory_for_map;

	repo_get_map( ps_eng, i16_map, p_loaded_map );

	ps_rmap_header = ( map_t * ) p_loaded_map;

	ps_eng->s_map = *ps_rmap_header; /* get header */
	ps_eng->s_map.p_loaded_map = p_loaded_map;
	ps_map = &ps_eng->s_map;

	ps_map->p_vertices = ( vec2_t *)( ( &ps_rmap_header->i_padding ) + 1 );
	ps_map->p_planes = ( plane_t *)( ps_map->p_vertices + ps_map->i_num_vertices );
	ps_map->p_lines = ( line_t * )( ps_map->p_planes + ps_map->i_num_planes );
	ps_map->p_sectors = ( sector_t *)( ps_map->p_lines + ps_map->i_num_lines );
	ps_map->p_nodes = ( node_t *)( ps_map->p_sectors + ps_map->i_num_sectors );
	ps_map->p_entities = ( mentity_t *)( ps_map->p_nodes + ps_map->i_num_nodes );
	ps_map->rgpui8_textures = ( UInt8 (*)[16] )( ps_map->p_entities + ps_map->i_num_entities );
	ps_map->pui8_visibility = ( UInt8 * )( ps_map->rgpui8_textures + ps_map->i_num_textures );
	ps_map->p_connectors = ( connector_t *)( ps_map->pui8_visibility + ps_map->i_num_visibility );
	ps_map->pi_connections = ( UInt16 * ) (ps_map->p_connectors + ps_map->i_num_connectors );
	ps_map->pui8_connection_distance = ( UInt8 * )( ps_map->pi_connections + ps_map->i_num_connector_connections );
	ps_map->p_clipnodes = ( clipnode_t * )( ps_map->pui8_connection_distance + ps_map->i_num_connector_connections );

	ps_eng->pui16_sector_entities = ( UInt16 * )( ps_map->p_clipnodes + ps_map->i_num_clip_nodes );

	ps_eng->i_num_node_visibility = ( ps_map->i_num_nodes + 7 ) >> 3;
	ps_eng->pui8_node_visibility = ( UInt8 * )( ps_eng->pui16_sector_entities + ps_map->i_num_sectors );

	if( ( ps_eng->pui8_node_visibility + ps_map->i_num_nodes ) > ( ( UInt8 * )ps_eng->p_memory_for_map + ps_eng->ui16_size_for_map ) )
	{
#ifdef WIN32
		assert( FALSE );
#endif
		return 0;
	}
	
#if 1
	for( i16_idx = 0; i16_idx < ps_map->i_num_nodes; i16_idx++ )
	{
		if( ps_map->p_nodes[ i16_idx ].i_plane == 0xffff )
		{
			Int16 i16_sec_offset = ps_map->p_nodes[ i16_idx ].i_sector_offset;
#ifdef WIN32
			assert( ps_map->p_nodes[ i16_idx ].i_parent_node >= 0 && ps_map->p_nodes[ i16_idx ].i_parent_node < 2 );
#endif
			ps_map->p_nodes[ i16_idx ].i_plane--;
			ps_map->p_nodes[ i16_idx ].i_plane += ps_map->p_nodes[ i16_idx ].i_parent_node; /* is sector count */

			ps_map->p_sectors[ i16_sec_offset ].i_connector = i16_idx;
		}
	}

	map_nodes_set_sectorlink_r( ps_map, 0, 0 );
#endif

	return 1;
}


Int16 map_point_plane_dist( engine_t *ps_eng, plane_t *p_plane, vec2_t v1 )
{
/*	return ( ( ( ( Int32 )v1[ 0 ] * ps_eng->s_map.p_vertices[ p_plane->i_normal ][ 0 ] ) + 0x4000 ) >>15 ) + ( ( ( ( Int32 )v1[ 1 ] * ps_eng->s_map.p_vertices[ p_plane->i_normal ][ 1 ] ) + 0x4000 ) >> 15 ) - p_plane->i_length;*/
	return ( ( mul_16_16( v1[ 0 ], ps_eng->s_map.p_vertices[ p_plane->i_normal ][ 0 ] ) + 0x4000 ) >>15 ) + ( ( mul_16_16(v1[ 1 ], ps_eng->s_map.p_vertices[ p_plane->i_normal ][ 1 ] ) + 0x4000 ) >> 15 ) - p_plane->i_length;
}


