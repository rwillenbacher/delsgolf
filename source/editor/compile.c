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
#include <memory.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "editor.h"

extern editor_t s_ed;

int i_nodenum;
int i_leafnum;
int i_num_csectors_in_tree;
csector_t *p_csectors_in_tree[ 0x10000 ];
int i_num_connectors;
connector_t rgs_connectors[ MAX_NUM_CONNECTORS ];

int i_debug_counter3 = 0;


float point_dist( vec2_t v1, vec2_t v2 )
{
	vec2_t delt;

	delt[ 0 ] = v1[ 0 ] - v2[ 0 ];
	delt[ 1 ] = v1[ 1 ] - v2[ 1 ];
	return sqrt( delt[ 0 ] * delt[ 0 ] + delt[ 1 ] * delt[ 1 ] );
}

void get_cline_plane( csector_t *p_csec, int i_line )
{
	cplane_t *p_list;
	cline_t *p_line;
	vec2_t norm;
	float f_scale, f_dist;

	p_line = &p_csec->rgs_lines[ i_line ];
	norm[ 1 ] = p_csec->v_vertices[ ( i_line + 1 ) % p_csec->i_num_vertices ][ 0 ] - p_csec->v_vertices[ i_line ][ 0 ];
	norm[ 0 ] = -( p_csec->v_vertices[ ( i_line + 1 ) % p_csec->i_num_vertices ][ 1 ] - p_csec->v_vertices[ i_line ][ 1 ] );
	f_scale = 1.0 / sqrt( ( norm[ 0 ] * norm[ 0 ] ) + ( norm[ 1 ] * norm[ 1 ] ) );
	norm[ 0 ] = norm[ 0 ] * f_scale;
	norm[ 1 ] = norm[ 1 ] * f_scale;

	f_dist = ( p_csec->v_vertices[ i_line ][ 0 ] * norm[ 0 ] ) + ( p_csec->v_vertices[ i_line ][ 1 ] * norm[ 1 ] );

	p_list = s_ed.p_planelist;
	while( p_list )
	{
		if( fabs( p_list->norm[ 0 ] - norm[ 0 ] ) < 0.001 && fabs( p_list->norm[ 1 ] - norm[ 1 ] ) < 0.001 && fabs( p_list->length - f_dist ) < 0.1 )
		{
			p_line->plane = p_list;
			return;
		}
		p_list = p_list->p_next;
	}
	p_list = malloc( sizeof( cplane_t ) );
	p_list->norm[ 0 ] = norm[ 0 ];
	p_list->norm[ 1 ] = norm[ 1 ];
	p_list->length = f_dist;
	if( p_list->length > 2048.0 )
	{
		p_list = p_list;
	}
	p_line->plane = p_list;

	p_list->p_next = s_ed.p_planelist;
	s_ed.p_planelist = p_list;
}

cplane_t *get_generic_plane( vec2_t v_normal, float f_dist )
{
	cplane_t *p_list;
	p_list = s_ed.p_planelist;
	while( p_list )
	{
		if( fabs( p_list->norm[ 0 ] - v_normal[ 0 ] ) < 0.001 && fabs( p_list->norm[ 1 ] - v_normal[ 1 ] ) < 0.001 && fabs( p_list->length - f_dist ) < 0.1 )
		{
			return p_list;
		}
		p_list = p_list->p_next;
	}
	p_list = malloc( sizeof( cplane_t ) );
	p_list->norm[ 0 ] = v_normal[ 0 ];
	p_list->norm[ 1 ] = v_normal[ 1 ];
	p_list->length = f_dist;
	if( p_list->length > 2048.0 )
	{
		p_list = p_list;
	}
	return p_list;
}


int plane_compare( cplane_t *p_plane1, cplane_t *p_plane2 )
{
	if( p_plane1 == p_plane2 )
	{
		return 1;
	}

	if( fabs( p_plane1->norm[ 0 ] - p_plane2->norm[ 0 ] ) < 0.01 &&
		fabs( p_plane1->norm[ 1 ] - p_plane2->norm[ 1 ] ) < 0.01 &&
		fabs( p_plane1->length - p_plane2->length ) < 0.01 )
	{
		return 1;
	}
	else if( fabs( p_plane1->norm[ 0 ] + p_plane2->norm[ 0 ] ) < 0.01 &&
		fabs( p_plane1->norm[ 1 ] + p_plane2->norm[ 1 ] ) < 0.01 &&
		fabs( p_plane1->length + p_plane2->length ) < 0.01 )
	{
		return -1;
	}
	return 0;
}

int split_csector( csector_t *p_sec, cplane_t *p_plane, csector_t **pp_front, csector_t **pp_back );
int split_csector_numsplits( csector_t *p_sec, cplane_t *p_plane );


int convex_vert( csector_t *p_sec, int i_vert )
{
	float f_dist;
	int i_lineidx, i_vertidx;

	i_lineidx = ( i_vert + p_sec->i_num_vertices - 1 ) % p_sec->i_num_vertices;
	i_vertidx = ( i_vert + 1 ) % p_sec->i_num_vertices;

	f_dist = ( p_sec->rgs_lines[ i_lineidx ].plane->norm[ 0 ] * p_sec->v_vertices[ i_vertidx ][ 0 ] +
			   p_sec->rgs_lines[ i_lineidx ].plane->norm[ 1 ] * p_sec->v_vertices[ i_vertidx ][ 1 ] ) - p_sec->rgs_lines[ i_lineidx ].plane->length;

	if( f_dist < -0.01 )
	{
		return 1;
	}
	return 0;
}

int splitless_line( csector_t *p_sec, int i_vert1, int i_vert2 )
{
	float f_dist, f_dist2[ 2 ], delt1[ 2 ], delt2[ 2 ], f1_v1, f1_v2, f2_v1, f2_v2;
	int i_v1, i_v2;

	delt2[ 0 ] = p_sec->v_vertices[ i_vert1 ][ 0 ] - p_sec->v_vertices[ i_vert2 ][ 0 ];
	delt2[ 1 ] = p_sec->v_vertices[ i_vert1 ][ 1 ] - p_sec->v_vertices[ i_vert2 ][ 1 ];
	f_dist = sqrt( delt2[ 0 ] * delt2[ 0 ] + delt2[ 1 ] * delt2[ 1 ] );
	delt2[ 0 ] /= f_dist;
	delt2[ 1 ] /= f_dist;
	f_dist2[ 0 ] = ( delt2[ 0 ] * p_sec->v_vertices[ i_vert1 ][ 0 ] ) + ( delt2[ 1 ] * p_sec->v_vertices[ i_vert1 ][ 1 ] );
	f_dist2[ 1 ] = ( delt2[ 0 ] * p_sec->v_vertices[ i_vert2 ][ 0 ] ) + ( delt2[ 1 ] * p_sec->v_vertices[ i_vert2 ][ 1 ] );

	delt1[ 1 ] = p_sec->v_vertices[ i_vert1 ][ 0 ] - p_sec->v_vertices[ i_vert2 ][ 0 ];
	delt1[ 0 ] = -( p_sec->v_vertices[ i_vert1 ][ 1 ] - p_sec->v_vertices[ i_vert2 ][ 1 ] );
	f_dist = sqrt( delt1[ 0 ] * delt1[ 0 ] + delt1[ 1 ] * delt1[ 1 ] );
	delt1[ 0 ] /= f_dist;
	delt1[ 1 ] /= f_dist;
	f_dist = ( delt1[ 0 ] * p_sec->v_vertices[ i_vert1 ][ 0 ] ) + ( delt1[ 1 ] * p_sec->v_vertices[ i_vert1 ][ 1 ] );


	for( i_v1 = 0; i_v1 < p_sec->i_num_vertices; i_v1++ )
	{
		i_v2 = ( i_v1 + 1 ) % p_sec->i_num_vertices;

		f1_v1 = ( delt1[ 0 ] * p_sec->v_vertices[ i_v1 ][ 0 ] ) + ( delt1[ 1 ] * p_sec->v_vertices[ i_v1 ][ 1 ] ) - f_dist;
		f1_v2 = ( delt1[ 0 ] * p_sec->v_vertices[ i_v2 ][ 0 ] ) + ( delt1[ 1 ] * p_sec->v_vertices[ i_v2 ][ 1 ] ) - f_dist;

		if( fabs( f1_v1 ) < 0.01 && fabs( f1_v2 ) < 0.01 ) /* handle on plane as a split */
		{
			return 0;
		}
		if( ( f1_v1 <= -0.01 && f1_v2 >= 0.01 ) ||
			( f1_v1 >= 0.01 && f1_v2 <= -0.01 ) )
		{
			vec2_t v_split;
			float f_scale;

			/* check endpoints of split line against split location */
			f_scale = f1_v2 / ( f1_v2 - f1_v1 );

			v_split[ 0 ] = p_sec->v_vertices[ i_v2 ][ 0 ] + ( ( p_sec->v_vertices[ i_v1 ][ 0 ] - p_sec->v_vertices[ i_v2 ][ 0 ] ) * f_scale );
			v_split[ 1 ] = p_sec->v_vertices[ i_v2 ][ 1 ] + ( ( p_sec->v_vertices[ i_v1 ][ 1 ] - p_sec->v_vertices[ i_v2 ][ 1 ] ) * f_scale );

			f1_v1 = ( delt2[ 0 ] * v_split[ 0 ] ) + ( delt2[ 1 ] * v_split[ 1 ] ) - f_dist2[ 0 ];
			f1_v2 = ( delt2[ 0 ] * v_split[ 0 ] ) + ( delt2[ 1 ] * v_split[ 1 ] ) - f_dist2[ 1 ];
			if( f1_v1 <= -0.01 && f1_v2 >= 0.01 )
			{
				return 0;
			}
		}
	}

	return 1;
}

int i_convcounter = 0;

csector_t *convex_sectors( csector_t *p_original_sec )
{
	int i_line, i_split, i_vert, i_vert2, i_vertidx;
	float f_dist;

	csector_t *p_list, *p_sec, *p_nsec1, *p_nsec2, *p_new_list, *p_next;

	p_list = p_original_sec;
	p_new_list = 0;
	for( p_sec = p_list; p_sec; p_sec = p_next )
	{
		p_next = p_sec->p_next;
		i_split = 0;
		i_convcounter++;
		if( i_convcounter == 23 )
		{
			i_convcounter = i_convcounter;
		}
		p_sec->i_secnum = i_convcounter;

		/* try double concave verts */
		for( i_vert = 0; i_vert < p_sec->i_num_vertices; i_vert++  )
		{
			if( convex_vert( p_sec, i_vert ) )
			{
				i_split = 1;
				for( i_vert2 = 0; i_vert2 < p_sec->i_num_vertices; i_vert2++ )
				{
					if( abs( i_vert2 - i_vert ) <= 1 || abs( i_vert2 - i_vert ) >= p_sec->i_num_vertices - 1 )
					{
						continue;
					}
					if( convex_vert( p_sec, i_vert2 ) && splitless_line( p_sec, i_vert, i_vert2 ) )
					{
						break;
					}
				}
				if( i_vert2 != p_sec->i_num_vertices )
				{
					break;
				}
			}
		}
		/* try single concave vertex */
		if( i_split && i_vert == p_sec->i_num_vertices )
		{
			for( i_vert = 0; i_vert < p_sec->i_num_vertices; i_vert++  )
			{
				if( convex_vert( p_sec, i_vert ) )
				{
					for( i_vert2 = 0; i_vert2 < p_sec->i_num_vertices; i_vert2++ )
					{
						if( abs( i_vert2 - i_vert ) <= 1 || abs( i_vert2 - i_vert ) >= p_sec->i_num_vertices - 1 )
						{
							continue;
						}
						if( splitless_line( p_sec, i_vert, i_vert2 ) )
						{
							break;
						}
					}
					if( i_vert2 != p_sec->i_num_vertices )
					{
						break;
					}
				}
			}
			if( i_vert == p_sec->i_num_vertices || i_vert2 == p_sec->i_num_vertices )
			{
				int *pi_null = 0;
				*pi_null = 0;
			}
		}
		if( i_split ) 
		{
			p_nsec1 = malloc( sizeof( csector_t ) );
			p_nsec2 = malloc( sizeof( csector_t ) );
			*p_nsec1 = *p_nsec2 = *p_sec;
			p_nsec1->i_num_vertices = p_nsec2->i_num_vertices = 0;

			i_vertidx = i_vert;
			while( i_vertidx != ( ( i_vert2 + 1 ) % p_sec->i_num_vertices ) )
			{
				i_line = ( i_vertidx + p_sec->i_num_vertices - 1 ) % p_sec->i_num_vertices;
				p_nsec1->v_vertices[ p_nsec1->i_num_vertices ][ 0 ] = p_sec->v_vertices[ i_vertidx ][ 0 ];
				p_nsec1->v_vertices[ p_nsec1->i_num_vertices ][ 1 ] = p_sec->v_vertices[ i_vertidx ][ 1 ];
				p_nsec1->i_num_vertices++;
				if( i_vertidx != i_vert )
				{
					p_nsec1->rgs_lines[ p_nsec1->i_num_vertices - 2 ] = p_sec->rgs_lines[ i_line ];
					get_cline_plane( p_nsec1, p_nsec1->i_num_vertices - 2 );
				}
				i_vertidx = ( i_vertidx + 1 ) % p_sec->i_num_vertices;
			}
			p_nsec1->rgs_lines[ p_nsec1->i_num_vertices - 1 ] = p_sec->rgs_lines[ i_line ];
			get_cline_plane( p_nsec1, p_nsec1->i_num_vertices - 1 );

			i_vertidx = ( i_vertidx + p_sec->i_num_vertices - 1 ) % p_sec->i_num_vertices;
			while( i_vertidx != ( ( i_vert + 1 ) % p_sec->i_num_vertices ) )
			{
				i_line = ( i_vertidx + p_sec->i_num_vertices - 1 ) % p_sec->i_num_vertices;
				p_nsec2->v_vertices[ p_nsec2->i_num_vertices ][ 0 ] = p_sec->v_vertices[ i_vertidx ][ 0 ];
				p_nsec2->v_vertices[ p_nsec2->i_num_vertices ][ 1 ] = p_sec->v_vertices[ i_vertidx ][ 1 ];
				p_nsec2->i_num_vertices++;
				if( i_vertidx != i_vert2 )
				{
					p_nsec2->rgs_lines[ p_nsec2->i_num_vertices - 2 ] = p_sec->rgs_lines[ i_line ];
					get_cline_plane( p_nsec2, p_nsec2->i_num_vertices - 2 );
				}
				i_vertidx = ( i_vertidx + 1 ) % p_sec->i_num_vertices;
			}
			p_nsec2->rgs_lines[ p_nsec2->i_num_vertices - 1 ] = p_sec->rgs_lines[ i_line ];
			get_cline_plane( p_nsec2, p_nsec2->i_num_vertices - 1 );

			free( p_sec );
			p_sec = p_nsec1;
			p_nsec1->p_next = p_nsec2;
			p_nsec2->p_next = p_next;
			p_next = p_nsec1;
		}
		if( !i_split )
		{
			p_sec->p_next = p_new_list;
			p_new_list = p_sec;
		}
	}
	p_list = p_new_list;

	return p_list;
}

int i_dbgcounter = 0;

csector_t *generate_csectors( )
{
	int i_sec_idx, i_line_idx, i_vert_idx, i_valid_sec_count;
	sector_t *p_sec;
	csector_t *p_list, *p_csec, *p_tail;
	cline_t *p_line, *p_nextline;

	p_list = 0;
	i_valid_sec_count = 0;
	s_ed.i_num_valid_sectors = 0;
	for( i_sec_idx = 0; i_sec_idx < s_ed.i_num_sectors; i_sec_idx++ )
	{
		p_csec = malloc( sizeof( csector_t ) );
		memset( p_csec, 0, sizeof( csector_t ) );


		p_sec = &s_ed.sectors[ i_sec_idx ];
		if( p_sec->i_numverts < 3 )
		{
			continue;
		}

		p_csec->z[ 0 ] = p_sec->z[ 0 ];
		p_csec->z[ 1 ] = p_sec->z[ 1 ];
		p_csec->i_secnum = -1;
		p_csec->i_sectorid = i_valid_sec_count++ + 1; /* 0 is special so move away from it */
		s_ed.i_num_valid_sectors = MAX( s_ed.i_num_valid_sectors, p_csec->i_sectorid + 1 );
		strcpy( p_csec->rgui8_floor_texture, p_sec->rgui8_texture_floor );
		strcpy( p_csec->rgui8_ceiling_texture, p_sec->rgui8_texture_ceiling );

		p_csec->i_num_vertices = 0;
		for( i_line_idx = 0; i_line_idx < p_sec->i_numverts; i_line_idx++ )
		{
			p_line = &p_csec->rgs_lines[ i_line_idx ];
			p_line->onsplitplane = 0;
			p_line->p_backsec = 0;
			strcpy( p_line->rgui8_lower_texture, p_sec->lines[ i_line_idx ].rgi8_texture_lower );
			strcpy( p_line->rgui8_upper_texture, p_sec->lines[ i_line_idx ].rgi8_texture_upper );
			p_csec->v_vertices[ i_line_idx ][ 0 ] = p_sec->verts[ i_line_idx ][ 0 ];
			p_csec->v_vertices[ i_line_idx ][ 1 ] = p_sec->verts[ i_line_idx ][ 1 ];
		}
		p_csec->i_num_vertices = p_sec->i_numverts;
		for( i_line_idx = 0; i_line_idx < p_csec->i_num_vertices; i_line_idx++ )
		{
			get_cline_plane( p_csec, i_line_idx );
		}

		//p_csec = convex_sectors( p_csec );

		p_tail = p_csec;
		while( p_tail->p_next )
		{
			for( i_vert_idx = 0; i_vert_idx < p_tail->i_num_vertices; i_vert_idx++ )
			{
				if( p_tail->v_vertices[ i_vert_idx ][ 0 ] < 10.0 && p_tail->v_vertices[ i_vert_idx ][ 1 ] < 10.0 )
				{
					p_tail = p_tail;
				}
			}

			p_tail = p_tail->p_next;
		}
		p_tail->p_next = p_list;
		p_list = p_csec;
	}

	/* unmark splitplane flag, copy vertices */
	for( p_csec = p_list; p_csec; p_csec = p_csec->p_next )
	{
		for( i_line_idx = 0; i_line_idx < p_csec->i_num_vertices; i_line_idx++ )
		{
			p_csec->rgs_lines[ i_line_idx ].onsplitplane = 0;
			p_csec->rgs_lines[ i_line_idx ].v1[ 0 ] = p_csec->v_vertices[ i_line_idx ][ 0 ];
			p_csec->rgs_lines[ i_line_idx ].v1[ 1 ] = p_csec->v_vertices[ i_line_idx ][ 1 ];
			p_csec->rgs_lines[ i_line_idx ].v2[ 0 ] = p_csec->v_vertices[ ( i_line_idx + 1 ) % p_csec->i_num_vertices ][ 0 ];
			p_csec->rgs_lines[ i_line_idx ].v2[ 1 ] = p_csec->v_vertices[ ( i_line_idx + 1 ) % p_csec->i_num_vertices ][ 1 ];
		}
	}

	return p_list;
}

void split_cline( cline_t *p_line, cplane_t *p_plane, cline_t **pp_front, cline_t **pp_back )
{
	int i_idx;
	float f_dist1, f_dist2;
	cline_t *p_frontl, *p_backl;

	*pp_front = *pp_back = 0;

	if(p_plane->length == 288 )
	{
		p_plane = p_plane;
	}

	f_dist1 = ( p_line->v1[ 0 ] * p_plane->norm[ 0 ] ) + ( p_line->v1[ 1 ] * p_plane->norm[ 1 ] ) - p_plane->length;
	f_dist2 = ( p_line->v2[ 0 ] * p_plane->norm[ 0 ] ) + ( p_line->v2[ 1 ] * p_plane->norm[ 1 ] ) - p_plane->length;

	i_idx = plane_compare( p_plane, p_line->plane );
	if( i_idx == 1 )
	{
		p_line->onsplitplane = 1;
		*pp_front = p_line;
	}
	else if( i_idx == -1 )
	{
		p_line->onsplitplane = 1;
		*pp_back = p_line;
	}
	else if( f_dist1 <= 0.1 && f_dist2 <= 0.1 )
	{
		*pp_back = p_line;
	}
	else if( f_dist1 >= -0.1 && f_dist2 >= -0.1 )
	{
		*pp_front = p_line;
	}
	else
	{
		vec2_t v_split;
		float f_scale;
		f_scale = f_dist2 / ( f_dist2 - f_dist1 );

		p_frontl = malloc( sizeof( cline_t ) );
		p_backl = malloc( sizeof( cline_t ) );
		*p_frontl = *p_line;
		*p_backl = *p_line;

		v_split[ 0 ] = p_line->v2[ 0 ] + ( ( p_line->v1[ 0 ] - p_line->v2[ 0 ] ) * f_scale );
		v_split[ 1 ] = p_line->v2[ 1 ] + ( ( p_line->v1[ 1 ] - p_line->v2[ 1 ] ) * f_scale );

		if( f_dist1 <= 0.1 )
		{
			p_backl->v1[ 0 ] = p_line->v1[ 0 ];
			p_backl->v1[ 1 ] = p_line->v1[ 1 ];
			p_backl->v2[ 0 ] = v_split[ 0 ];
			p_backl->v2[ 1 ] = v_split[ 1 ];

			p_frontl->v1[ 0 ] = v_split[ 0 ];
			p_frontl->v1[ 1 ] = v_split[ 1 ];
			p_frontl->v2[ 0 ] = p_line->v2[ 0 ];
			p_frontl->v2[ 1 ] = p_line->v2[ 1 ];
		}
		else
		{
			p_frontl->v1[ 0 ] = p_line->v1[ 0 ];
			p_frontl->v1[ 1 ] = p_line->v1[ 1 ];
			p_frontl->v2[ 0 ] = v_split[ 0 ];
			p_frontl->v2[ 1 ] = v_split[ 1 ];

			p_backl->v1[ 0 ] = v_split[ 0 ];
			p_backl->v1[ 1 ] = v_split[ 1 ];
			p_backl->v2[ 0 ] = p_line->v2[ 0 ];
			p_backl->v2[ 1 ] = p_line->v2[ 1 ];
		}
		*pp_front = p_frontl;
		*pp_back = p_backl;
		free( p_line );
	}
}

int split_csector( csector_t *p_sec, cplane_t *p_plane, csector_t **pp_front, csector_t **pp_back )
{
	cline_t *p_front_list, *p_front, *p_back_list, *p_back, *p_list, *p_next;
	csector_t *p_secfront, *p_secback;

	if( fabs( p_plane->length - 520.43 ) < 0.01 && fabs( p_plane->norm[ 0 ] - 0.70710 ) < 0.01 )
	{
		p_plane = p_plane;
	}

	p_front_list = p_back_list = 0;
	for( p_list = p_sec->p_bsp_lines; p_list; p_list = p_next )
	{
		p_next = p_list->p_next;

		split_cline( p_list, p_plane, &p_front, &p_back );

		if( p_front )
		{
			p_front->p_next = p_front_list;
			p_front_list = p_front;
		}
		if( p_back )
		{
			p_back->p_next = p_back_list;
			p_back_list = p_back;
		}
	}

	*pp_front = *pp_back = 0;

	if( p_front_list )
	{
		p_secfront = malloc( sizeof( csector_t ) );
		*p_secfront = *p_sec;

		p_secfront->p_bsp_lines = p_front_list;
		*pp_front = p_secfront;
	}

	if( p_back_list )
	{
		p_secback = malloc( sizeof( csector_t ) );
		*p_secback = *p_sec;

		p_secback->p_bsp_lines = p_back_list;
		*pp_back = p_secback;
	}

	free( p_sec );

#if 0
	int i_idx, i_idx2, i_side, i_onfront, i_onback;
	int rgi_sides[ MAX_SECTOR_VERTS ];
	float rgf_dists[ MAX_SECTOR_VERTS ];
	float i_dist1, i_dist2, f_scale;
	csector_t *p_sec_frontlist = NULL, *p_secfront, *p_secback;

	i_onfront = i_onback = 0;
	i_dbgcounter++;

	for( i_idx = 0; i_idx < p_sec->i_num_vertices; i_idx++ )
	{
		i_dist1 = ( p_sec->v_vertices[ i_idx ][ 0 ] * p_plane->norm[ 0 ] ) + ( p_sec->v_vertices[ i_idx ][ 1 ] * p_plane->norm[ 1 ] ) - p_plane->length;
		rgf_dists[ i_idx ] = i_dist1;
		if( i_dist1 < -0.01 )
		{
			rgi_sides[ i_idx ] = -1;
			i_onback++;
		}
		else if( i_dist1 > 0.01 )
		{
			rgi_sides[ i_idx ] = 1;
			i_onfront++;
		}
		else
		{
			rgi_sides[ i_idx ] = 0;
		}
	}

	if( i_onfront == 0 && i_onback == 0 )
	{
		int *pi_null = 0;
		*pi_null = 0;
	}
	else if( i_onfront > 0 && i_onback == 0 )
	{
		*pp_front = p_sec;
		*pp_back = 0;
		for( i_idx = 0; i_idx < p_sec->i_num_vertices; i_idx++ )
		{
			if( rgi_sides[ i_idx ] == 0 && rgi_sides[ ( i_idx + 1 ) % p_sec->i_num_vertices ] == 0 )
			{
				p_sec->rgs_lines[ i_idx ].onsplitplane = 1;
			}
		}
		return 0;
	}
	else if( i_onback > 0 && i_onfront == 0 )
	{
		*pp_front = 0;
		*pp_back = p_sec;
		for( i_idx = 0; i_idx < p_sec->i_num_vertices; i_idx++ )
		{
			if( rgi_sides[ i_idx ] == 0 && rgi_sides[ ( i_idx + 1 ) % p_sec->i_num_vertices ] == 0 )
			{
				p_sec->rgs_lines[ i_idx ].onsplitplane = 1;
			}
		}
		return 0;
	}
	else /* something splits */
	{
	if( p_sec->i_secnum == 20 )
	{
		p_sec = p_sec;
	}
		p_secfront = malloc( sizeof( csector_t ) );
		p_secback = malloc( sizeof( csector_t ) );

		memset( p_secfront, 0, sizeof( csector_t ) );
		memset( p_secback, 0, sizeof( csector_t ) );

		*p_secfront = *p_secback = *p_sec;
		p_secfront->i_num_vertices = p_secback->i_num_vertices = 0; /*p_secfront->i_secnum = p_secback->i_secnum = 0 */;
		p_secfront->p_next = p_secback->p_next = NULL;

		i_side = rgi_sides[ p_sec->i_num_vertices - 1 ];
		i_idx = 0;
		i_idx2 = p_sec->i_num_vertices - 1;
		do
		{
			if( rgi_sides[ i_idx ] != i_side && rgi_sides[ i_idx ] != 0 && i_side != 0 )
			{
				p_secfront->rgs_lines[ p_secfront->i_num_vertices ] = p_sec->rgs_lines[ i_idx2 ];
				p_secback->rgs_lines[ p_secback->i_num_vertices ] = p_sec->rgs_lines[ i_idx2 ];

				i_dist2 = rgf_dists[ ( i_idx + p_sec->i_num_vertices - 1 ) % p_sec->i_num_vertices ];
				i_dist1 = rgf_dists[ ( i_idx ) % p_sec->i_num_vertices ];

				f_scale = i_dist2 / ( i_dist2 - i_dist1 );

				p_secback->v_vertices[ p_secback->i_num_vertices ][ 0 ] = p_sec->v_vertices[ i_idx2 ][ 0 ] + ( ( p_sec->v_vertices[ i_idx ][ 0 ] - p_sec->v_vertices[ i_idx2 ][ 0 ] ) * f_scale );
				p_secback->v_vertices[ p_secback->i_num_vertices ][ 1 ] = p_sec->v_vertices[ i_idx2 ][ 1 ] + ( ( p_sec->v_vertices[ i_idx ][ 1 ] - p_sec->v_vertices[ i_idx2 ][ 1 ] ) * f_scale );
				p_secfront->v_vertices[ p_secfront->i_num_vertices ][ 0 ] = p_secback->v_vertices[ p_secback->i_num_vertices ][ 0 ];
				p_secfront->v_vertices[ p_secfront->i_num_vertices ][ 1 ] = p_secback->v_vertices[ p_secback->i_num_vertices ][ 1 ];

				p_secfront->i_num_vertices++;
				p_secback->i_num_vertices++;
				i_side = 0;
			}
			if( ( rgi_sides[ i_idx ] == 1 && i_side >= 0 ) || ( rgi_sides[ i_idx ] == 0 ) )
			{
				p_secfront->rgs_lines[ p_secfront->i_num_vertices ] = p_sec->rgs_lines[ i_idx ];
				p_secfront->v_vertices[ p_secfront->i_num_vertices ][ 0 ] = p_sec->v_vertices[ i_idx ][ 0 ];
				p_secfront->v_vertices[ p_secfront->i_num_vertices++ ][ 1 ] = p_sec->v_vertices[ i_idx ][ 1 ];
			}
			if( ( rgi_sides[ i_idx ] <= 0 && i_side <= 0 ) || ( rgi_sides[ i_idx ] == 0 ) )
			{
				p_secback->rgs_lines[ p_secback->i_num_vertices ] = p_sec->rgs_lines[ i_idx ];
				p_secback->v_vertices[ p_secback->i_num_vertices ][ 0 ] = p_sec->v_vertices[ i_idx ][ 0 ];
				p_secback->v_vertices[ p_secback->i_num_vertices++ ][ 1 ] = p_sec->v_vertices[ i_idx ][ 1 ];
			}
			i_side = rgi_sides[ i_idx ];
			i_idx++;
			i_idx2 = ( i_idx2 + 1 )  % p_sec->i_num_vertices;
		}while( i_idx < p_sec->i_num_vertices );
	}

	if( p_secfront->i_num_vertices < 3 )
	{
		free( p_secfront );
		p_secfront = NULL;
	}
	else
	{
		for( i_idx = 0; i_idx < p_secfront->i_num_vertices; i_idx++ )
		{
			get_cline_plane( p_secfront, i_idx );
			if( plane_compare( p_secfront->rgs_lines[ i_idx ].plane, p_plane ) )
			{
				p_secfront->rgs_lines[ i_idx ].onsplitplane = 1;
			}
		}
	}
	if( p_secback->i_num_vertices < 3 )
	{
		free( p_secback );
		p_secback = NULL;
	}
	else
	{
		for( i_idx = 0; i_idx < p_secback->i_num_vertices; i_idx++ )
		{
			get_cline_plane( p_secback, i_idx );
			if( plane_compare( p_secback->rgs_lines[ i_idx ].plane, p_plane ) )
			{
				p_secback->rgs_lines[ i_idx ].onsplitplane = 1;
			}
		}
	}

	*pp_front = p_secfront;
	*pp_back = p_secback;
	free( p_sec );
#endif

	return 0;
}

int split_csector_numsplits( csector_t *p_sec, cplane_t *p_plane )
{
	int i_idx, i_idx2, i_split, i_onfront, i_onback;
	int rgi_sides[ MAX_SECTOR_VERTS ];
	float i_dist1, i_dist2;
	cline_t *p_line;

	i_split = i_onfront = i_onback = 0;
	for( p_line = p_sec->p_bsp_lines; p_line; p_line = p_line->p_next )
	{
		i_idx = plane_compare( p_plane, p_line->plane );
		if( !i_idx )
		{
			i_dist1 = ( p_line->v1[ 0 ] * p_plane->norm[ 0 ] ) + ( p_line->v1[ 1 ] * p_plane->norm[ 1 ] ) - p_plane->length;
			i_dist2 = ( p_line->v2[ 0 ] * p_plane->norm[ 0 ] ) + ( p_line->v2[ 1 ] * p_plane->norm[ 1 ] ) - p_plane->length;
			if( i_dist1 < 0.01 && i_dist2 < 0.01 )
			{
				i_onback++;
			}
			else if( i_dist1 > -0.01 && i_dist2 >= -0.01 )
			{
				i_onfront++;
			}
			else
			{
				i_split++;
			}
		}
	}
	return i_split;
}

int i_counter = 0;

void split_csector_list( csector_t *p_list, csector_t **p_front, csector_t **p_back, cplane_t **p_splitplane )
{
	int i_idx, i_numsplits, i_bnumsplits;
	csector_t *p_seclist, *p_splitlist, *p_frontlist, *p_backlist, *p_fronts, *p_backs, *p_nextsec;
	cplane_t *p_plane, *p_bplane;
	cline_t *p_llist;

	i_counter++;

	if( i_counter == 32  )
	{
		i_counter = i_counter;
	}

	i_bnumsplits = -1;
	p_bplane = NULL;
	for( p_seclist = p_list; p_seclist; p_seclist = p_seclist->p_next )
	{
		for( p_llist = p_seclist->p_bsp_lines; p_llist; p_llist = p_llist->p_next )
		{
			if( p_llist->onsplitplane || p_llist->just_portal )
				continue;
			p_plane = p_llist->plane;

			p_splitlist = p_list;
			i_numsplits = 0;
			while( p_splitlist )
			{
				i_numsplits += split_csector_numsplits( p_splitlist, p_plane );
				p_splitlist = p_splitlist->p_next;
			}
			if( i_numsplits < i_bnumsplits || i_bnumsplits == -1 )
			{
				p_bplane = p_plane;
				i_bnumsplits = i_numsplits;
			}
		}
	}

	if( p_bplane )
	{
		p_frontlist = p_backlist = 0;
		for( p_seclist = p_list; p_seclist; p_seclist = p_nextsec )
		{
			p_nextsec = p_seclist->p_next;

			p_fronts = p_backs = 0;

			split_csector( p_seclist, p_bplane, &p_fronts, &p_backs );
			if( p_fronts )
			{
				p_fronts->p_next = p_frontlist;
				p_frontlist = p_fronts;
			}
			if( p_backs )
			{
				p_backs->p_next = p_backlist;
				p_backlist = p_backs;
			}
		}
	}
	else
	{
		p_frontlist = p_seclist;
		p_backlist = 0;
	}
	*p_front = p_frontlist;
	*p_back = p_backlist;
	*p_splitplane = p_bplane;
}

int i_counter_split = 0;

bspnode_t *build_bsptree_r( csector_t *p_sectors )
{
	bspnode_t *p_node;
	csector_t *p_front, *p_back;
	cplane_t *p_plane;

	p_node = malloc( sizeof( bspnode_t ) );
	memset( p_node, 0, sizeof( bspnode_t ) );

	p_node->i_nodenum = i_nodenum++;

	if( !p_sectors )
	{
		p_node->i_type = BSPNODE_LEAF;
		return p_node;
	}

	i_counter_split++;

	if( i_counter_split == 5 )
	{
		i_counter_split = i_counter_split;
	}

	split_csector_list( p_sectors, &p_front, &p_back, &p_plane );

	p_node->bbox[ 0 ][ 0 ] = 8096.0;
	p_node->bbox[ 0 ][ 1 ] = 8096.0;
	p_node->bbox[ 1 ][ 0 ] = -8096.0;
	p_node->bbox[ 1 ][ 1 ] = -8096.0;
	if( p_plane )
	{
		p_node->i_type = BSPNODE_SPLIT;
		p_node->p_plane = p_plane;
		p_node->p_front = build_bsptree_r( p_front );
		p_node->p_back = build_bsptree_r( p_back );
		p_node->p_front->p_parent = p_node;
		p_node->p_back->p_parent = p_node;

		if( p_node->p_front->i_valid_bbox && p_node->p_back->i_valid_bbox )
		{
			p_node->bbox[ 0 ][ 0 ] = MIN( p_node->p_front->bbox[ 0 ][ 0 ], p_node->p_back->bbox[ 0 ][ 0 ] );
			p_node->bbox[ 0 ][ 1 ] = MIN( p_node->p_front->bbox[ 0 ][ 1 ], p_node->p_back->bbox[ 0 ][ 1 ] );
			p_node->bbox[ 1 ][ 0 ] = MAX( p_node->p_front->bbox[ 1 ][ 0 ], p_node->p_back->bbox[ 1 ][ 0 ] );
			p_node->bbox[ 1 ][ 1 ] = MAX( p_node->p_front->bbox[ 1 ][ 1 ], p_node->p_back->bbox[ 1 ][ 1 ] );
		}
		else if( p_node->p_front->i_valid_bbox )
		{
			p_node->bbox[ 0 ][ 0 ] = p_node->p_front->bbox[ 0 ][ 0 ];
			p_node->bbox[ 0 ][ 1 ] = p_node->p_front->bbox[ 0 ][ 1 ];
			p_node->bbox[ 1 ][ 0 ] = p_node->p_front->bbox[ 1 ][ 0 ];
			p_node->bbox[ 1 ][ 1 ] = p_node->p_front->bbox[ 1 ][ 1 ];
		}
		else if( p_node->p_back->i_valid_bbox )
		{
			p_node->bbox[ 0 ][ 0 ] = p_node->p_back->bbox[ 0 ][ 0 ];
			p_node->bbox[ 0 ][ 1 ] = p_node->p_back->bbox[ 0 ][ 1 ];
			p_node->bbox[ 1 ][ 0 ] = p_node->p_back->bbox[ 1 ][ 0 ];
			p_node->bbox[ 1 ][ 1 ] = p_node->p_back->bbox[ 1 ][ 1 ];
		}
		else
		{
			int *pi_null = 0;
			*pi_null = 0;
		}
		p_node->i_valid_bbox = 1;
	}
	else
	{
		p_node->i_type = BSPNODE_LEAF;
		p_node->p_sec = p_sectors;
		/* collect sectors */
		
		if( i_num_csectors_in_tree == 18 )
		{
			i_num_csectors_in_tree = i_num_csectors_in_tree;
		}

		if( p_node->p_sec )
		{
			while( p_sectors->p_next )
			{
				csector_t *p_sec;
				cline_t *p_cline;
				if( strcmp( p_sectors->rgui8_ceiling_texture, p_sectors->p_next->rgui8_ceiling_texture ) != 0 ||
					strcmp( p_sectors->rgui8_floor_texture, p_sectors->p_next->rgui8_floor_texture ) != 0 ||
					p_sectors->z[ 0 ] != p_sectors->p_next->z[ 0 ] ||
					p_sectors->z[ 1 ] != p_sectors->p_next->z[ 1 ] )
				{
					printf("ERROR: merging non fitting sectors at ( %f %f ), ( %d %d )\n", p_sectors->p_bsp_lines->v1[ 0 ], p_sectors->p_bsp_lines->v1[ 1 ], p_sectors->p_next->p_bsp_lines->v1[ 0 ], p_sectors->p_next->p_bsp_lines->v1[ 0 ] );
				}
				p_cline = p_sectors->p_bsp_lines;
				while( p_cline->p_next )
					p_cline = p_cline->p_next;
				p_cline->p_next = p_sectors->p_next->p_bsp_lines;
				p_sec = p_sectors->p_next->p_next;
				free( p_sectors->p_next );
				p_sectors->p_next = p_sec;
			}
			for( p_front = p_sectors; p_front; p_front = p_front->p_next )
			{
				cline_t *p_line;
				int i_idx;
				p_front->i_secnum = i_num_csectors_in_tree;
				p_csectors_in_tree[ i_num_csectors_in_tree++ ] = p_front;
			
				if( !p_front->p_bsp_lines )
				{
					printf("WARNING: sector without lines\n");
				}
				for( p_line = p_front->p_bsp_lines; p_line; p_line = p_line->p_next )
				{
					p_node->bbox[ 0 ][ 0 ] = MIN( p_node->bbox[ 0 ][ 0 ], p_line->v1[ 0 ] );
					p_node->bbox[ 0 ][ 1 ] = MIN( p_node->bbox[ 0 ][ 1 ], p_line->v1[ 1 ] );
					p_node->bbox[ 1 ][ 0 ] = MAX( p_node->bbox[ 1 ][ 0 ], p_line->v1[ 0 ] );
					p_node->bbox[ 1 ][ 1 ] = MAX( p_node->bbox[ 1 ][ 1 ], p_line->v1[ 1 ] );

					p_node->bbox[ 0 ][ 0 ] = MIN( p_node->bbox[ 0 ][ 0 ], p_line->v2[ 0 ] );
					p_node->bbox[ 0 ][ 1 ] = MIN( p_node->bbox[ 0 ][ 1 ], p_line->v2[ 1 ] );
					p_node->bbox[ 1 ][ 0 ] = MAX( p_node->bbox[ 1 ][ 0 ], p_line->v2[ 0 ] );
					p_node->bbox[ 1 ][ 1 ] = MAX( p_node->bbox[ 1 ][ 1 ], p_line->v2[ 1 ] );
				}
			}
			p_sectors->p_node = p_node;
			p_node->i_valid_bbox = 1;
		}
		else
		{
			p_node->i_valid_bbox = 0;
			p_node->bbox[ 0 ][ 0 ] = 0.0;
			p_node->bbox[ 0 ][ 1 ] = 0.0;
			p_node->bbox[ 1 ][ 0 ] = 0.0;
			p_node->bbox[ 1 ][ 1 ] = 0.0;
		}
	}
	return p_node;
}


bspnode_t s_cliptree_outside;

bspnode_t *build_cliptree_r( csector_t *p_sectors )
{
	bspnode_t *p_node;
	csector_t *p_front, *p_back;
	cplane_t *p_plane;

	p_node = malloc( sizeof( bspnode_t ) );
	memset( p_node, 0, sizeof( bspnode_t ) );


	if( !p_sectors )
	{
		p_node->i_type = BSPNODE_LEAF_SOLID;
		p_node->p_sec = 0;
		return p_node;
	}

	split_csector_list( p_sectors, &p_front, &p_back, &p_plane );

	if( p_plane )
	{
		p_node->i_type = BSPNODE_SPLIT;
		p_node->p_plane = p_plane;
		p_node->p_front = build_cliptree_r( p_front );
		p_node->p_back = build_cliptree_r( p_back );
		p_node->p_front->p_parent = p_node;
		p_node->p_back->p_parent = p_node;
	}
	else
	{
		p_node->i_type = BSPNODE_LEAF;
		p_node->p_sec = p_sectors; /* just need z extends in export and sector ref */
	}
	return p_node;
}


bspnode_t *build_cliptree( csector_t *p_sectors, int i_bbox )
{
	bspnode_t *p_node, *p_nextnode;
	cplane_t *p_plane;

	vec2_t s_extends[] = {
		{ -1.0, 0.0 },
		{ 1.0, 0.0 },
		{ 0.0, -1.0 },
		{ 0.0, 1.0 },
	};

	if( i_bbox == 0 )
	{
		memset( &s_cliptree_outside, 0, sizeof( s_cliptree_outside ) );
		s_cliptree_outside.i_type = BSPNODE_LEAF;
	}


	p_node = malloc( sizeof( bspnode_t ) );
	memset( p_node, 0, sizeof( bspnode_t ) );
	 
	/* fixme: bbox volume instead of maximum extends */
	p_plane = get_generic_plane( s_extends[ i_bbox ], 1023.0 );
	p_node->i_type = BSPNODE_SPLIT;
	p_node->p_plane = p_plane;
	p_node->p_front = &s_cliptree_outside;
	p_node->i_no_portal = 1;
	if( i_bbox == 3 )
	{
		p_node->p_back = build_cliptree_r( p_sectors );
	}
	else
	{
		p_node->p_back = build_cliptree( p_sectors, i_bbox + 1 );
	}
	p_node->p_back->p_parent = p_node;
	return p_node;
}

bspnode_t *build_entitycliptree( int i_bbox )
{
	bspnode_t *p_node, *p_nextnode;
	cplane_t *p_plane;

	vec2_t s_extends[] = {
		{ -1.0, 0.0 },
		{ 1.0, 0.0 },
		{ 0.0, -1.0 },
		{ 0.0, 1.0 },
	};

	p_node = malloc( sizeof( bspnode_t ) );
	memset( p_node, 0, sizeof( bspnode_t ) );

	if( i_bbox <= 3 )
	{
		p_plane = get_generic_plane( s_extends[ i_bbox ], 0.0 );
		p_node->i_type = BSPNODE_SPLIT;
		p_node->p_plane = p_plane;
		p_node->p_front = p_node->p_front = &s_cliptree_outside;
		p_node->p_back = build_entitycliptree( i_bbox + 1 );
	}
	else
	{
		p_node = malloc( sizeof( bspnode_t ) );
		memset( p_node, 0, sizeof( bspnode_t ) );
		p_node->i_type = BSPNODE_LEAF_SOLID;
	}
	return p_node;
}



void bevel_cliptree_r( bspnode_t *p_node )
{
	portal_t *p_portal;
	p_node->bbox[ 0 ][ 0 ] = 8096.0;
	p_node->bbox[ 0 ][ 1 ] = 8096.0;
	p_node->bbox[ 1 ][ 0 ] = -8096.0;
	p_node->bbox[ 1 ][ 1 ] = -8096.0;

	if( p_node->i_type == BSPNODE_SPLIT )
	{
		bevel_cliptree_r( p_node->p_front );
		bevel_cliptree_r( p_node->p_back );
	}
	else if( p_node->i_type == BSPNODE_LEAF_SOLID )
	{
		int i_idx;
		cplane_t rgs_planes[ 4 ];
		vec2_t rgs_normals[ 4 ] = {
			{ -1.0, 0 },
			{ 1.0, 0 },
			{ 0.0, -1.0 },
			{ 0.0, 1.0 }
		};
		int i_add_plane;
		p_portal = p_node->p_portals;

		while( p_portal )
		{
			p_node->bbox[ 0 ][ 0 ] = MIN( p_node->bbox[ 0 ][ 0 ], p_portal->v1[ 0 ] );
			p_node->bbox[ 0 ][ 1 ] = MIN( p_node->bbox[ 0 ][ 1 ], p_portal->v1[ 1 ] );
			p_node->bbox[ 1 ][ 0 ] = MAX( p_node->bbox[ 1 ][ 0 ], p_portal->v1[ 0 ] );
			p_node->bbox[ 1 ][ 1 ] = MAX( p_node->bbox[ 1 ][ 1 ], p_portal->v1[ 1 ] );

			p_node->bbox[ 0 ][ 0 ] = MIN( p_node->bbox[ 0 ][ 0 ], p_portal->v2[ 0 ] );
			p_node->bbox[ 0 ][ 1 ] = MIN( p_node->bbox[ 0 ][ 1 ], p_portal->v2[ 1 ] );
			p_node->bbox[ 1 ][ 0 ] = MAX( p_node->bbox[ 1 ][ 0 ], p_portal->v2[ 0 ] );
			p_node->bbox[ 1 ][ 1 ] = MAX( p_node->bbox[ 1 ][ 1 ], p_portal->v2[ 1 ] );

			p_portal = p_portal->p_front == p_node ? p_portal->p_nextf : p_portal->p_nextb;
		}

		for( i_idx = 0; i_idx < 4; i_idx++ )
		{
			float f_dist;
			switch( i_idx )
			{
			case 0:
				f_dist = -p_node->bbox[ 0 ][ 0 ];
				break;
			case 1:
				f_dist = p_node->bbox[ 1 ][ 0 ];
				break;
			case 2:
				f_dist = -p_node->bbox[ 0 ][ 1 ];
				break;
			case 3:
				f_dist = p_node->bbox[ 1 ][ 1 ];
				break;
			}

			rgs_planes[ i_idx ].length = f_dist;
			rgs_planes[ i_idx ].norm[ 0 ] = rgs_normals[ i_idx ][ 0 ];
			rgs_planes[ i_idx ].norm[ 1 ] = rgs_normals[ i_idx ][ 1 ];
		}

		for( i_idx = 0; i_idx < 4; i_idx++ )
		{
			i_add_plane = 1;
			p_portal = p_node->p_portals;
			while( p_portal )
			{
				if( plane_compare( &p_portal->s_plane, &rgs_planes[ i_idx ] ) )
				{
					i_add_plane = 0;
				}
				p_portal = p_portal->p_front == p_node ? p_portal->p_nextf : p_portal->p_nextb;
			}

			if( i_add_plane )
			{
				float f_dist;
				bspnode_t *p_newnode, *p_newsplit;
				p_newnode = malloc( sizeof( bspnode_t ) );
				memset( p_newnode, 0, sizeof( bspnode_t ) );
				p_newsplit = malloc( sizeof( bspnode_t ) );
				memset( p_newsplit, 0, sizeof( bspnode_t ) );


				p_newnode->i_type = BSPNODE_LEAF_EMPTY;
				p_newsplit->i_type = BSPNODE_SPLIT;

				p_newsplit->p_plane = get_generic_plane( rgs_planes[ i_idx ].norm, rgs_planes[ i_idx ].length );
				p_newsplit->p_front = p_newnode;
				p_newsplit->p_parent = p_node->p_parent;
				if( p_node->p_parent->p_front == p_node )
				{
					p_node->p_parent->p_front = p_newsplit;
				}
				else
				{
					p_node->p_parent->p_back = p_newsplit;
				}
				p_node->p_parent = p_newsplit;
				p_newsplit->p_back = p_node;
			}
		}
	}
}

void prepare_sectors_lines_connectors( csector_t *p_csectors )
{
	csector_t *p_sec1, *p_sec2;
	int i_idx, i_con_idx, i_sec_idx, i_line_idx, i_line_idx2;
	csector_t *p_csec, *p_csec2;
	cline_t *p_line, *p_nextline, *p_line2, *p_secline;

	for( p_sec1 = p_csectors; p_sec1; p_sec1 = p_sec1->p_next )
	{
		p_csec = p_sec1;


		for( i_line_idx = 0; i_line_idx < p_csec->i_num_vertices; i_line_idx++ )
		{
			p_line = &p_csec->rgs_lines[ i_line_idx ];
			p_nextline = p_line->p_next;
			p_line->ui8_flags = LINEFLAG_MIDDLE;

			if( fabs( p_line->plane->norm[ 0 ] ) > fabs( p_line->plane->norm[ 1 ] ) )
			{
				p_line->ui8_flags |= LINEFLAG_TEX_Y;
			}
			p_line->just_portal = 0;
			p_line->onsplitplane = 0;

			for( p_sec2 = p_csectors; p_sec2; p_sec2 = p_sec2->p_next )
			{
				int i_side;
				if( p_sec1 == p_sec2 )
				{
					continue;
				}
				p_csec2 = p_sec2;
				for( i_line_idx2 = 0; i_line_idx2 < p_csec2->i_num_vertices; i_line_idx2++  )
				{
					vec2_t delt1;
					float d1, d2, d3, d4;
					p_line2 = &p_csec2->rgs_lines[ i_line_idx2 ];
					i_side = plane_compare( p_line->plane, p_line2->plane );
					if( i_side == -1 )
					{
						delt1[ 0 ] = p_csec->v_vertices[ ( i_line_idx + 1 ) % p_csec->i_num_vertices ][ 0 ] - p_csec->v_vertices[ i_line_idx ][ 0 ];
						delt1[ 1 ] = ( p_csec->v_vertices[ ( i_line_idx + 1 ) % p_csec->i_num_vertices ][ 1 ] - p_csec->v_vertices[ i_line_idx ][ 1 ] );
						d1 = sqrt( delt1[ 0 ] * delt1[ 0 ] + delt1[ 1 ] * delt1[ 1 ] );
						delt1[ 0 ] /= d1;
						delt1[ 1 ] /= d1;
						d1 = ( delt1[ 0 ] * p_csec->v_vertices[ i_line_idx ][ 0 ] ) + ( delt1[ 1 ] * p_csec->v_vertices[ i_line_idx ][ 1 ] );
						d2 = ( delt1[ 0 ] * p_csec->v_vertices[ ( i_line_idx + 1 ) % p_csec->i_num_vertices ][ 0 ] ) + ( delt1[ 1 ] * p_csec->v_vertices[ ( i_line_idx + 1 ) % p_csec->i_num_vertices ][ 1 ] ) - d1;

						d3 = ( p_csec2->v_vertices[ i_line_idx2 ][ 0 ] * delt1[ 0 ] ) + ( p_csec2->v_vertices[ i_line_idx2 ][ 1 ] * delt1[ 1 ] ) - d1;
						d4 = ( p_csec2->v_vertices[ ( i_line_idx2 + 1 ) % p_csec2->i_num_vertices ][ 0 ] * delt1[ 0 ] ) + ( p_csec2->v_vertices[ ( i_line_idx2 + 1 ) % p_csec2->i_num_vertices ][ 1 ] * delt1[ 1 ] ) - d1;
						if( d4 >= d3 )
						{
							int *pi_null = 0;
							*pi_null = 0;
						}
						if( ( d3 <= 0.1 && d4 < 0.1 ) || ( d3 >= (d2-0.1) && d4 >= (d2-0.1) ) )
						{
							continue;
						}
						/* there is some overlap, check vertices on normal */
						/* we could split it ourselfs but lets just warn myself for now as its bad practice */
						if( d4 <= 0.1 )
						{
						}
						else
						{
							printf("WARNING: lines dont fit at %f %f\n", p_csec->v_vertices[ i_line_idx ][ 0 ], p_csec->v_vertices[ i_line_idx ][ 1 ] );
						}
						if( d3 >= d2+0.1 )
						{
							printf("WARNING: lines dont fit at %f %f\n", p_csec->v_vertices[ i_line_idx ][ 0 ], p_csec->v_vertices[ i_line_idx ][ 1 ] );
						}
						else
						{
						}


						if( p_csec->z[ 0 ] == p_csec2->z[ 0 ] && p_csec->z[ 1 ] == p_csec2->z[ 1 ] &&
							strcmp( p_csec->rgui8_ceiling_texture, p_csec2->rgui8_ceiling_texture ) == 0 &&
							strcmp( p_csec->rgui8_floor_texture, p_csec2->rgui8_floor_texture ) == 0 )
						{
							p_line->onsplitplane = 1; /* dont let nodraw lines split the tree */
							p_line->just_portal = 1;
						}

						if( p_csec->z[ 0 ] >= p_csec2->z[ 0 ] && p_csec->z[ 1 ] <= p_csec2->z[ 1 ] )
						{
							p_line->ui8_flags &= ~LINEFLAG_MIDDLE; /* middle is completely covered */
						}
						if( p_csec->z[ 0 ] < p_csec2->z[ 0 ] && p_csec->z[ 1 ] > p_csec2->z[ 0 ] ) /* needs lower */
						{
							p_line->ui8_flags &= ~LINEFLAG_MIDDLE;
							p_line->ui8_flags |= LINEFLAG_LOWER;
						}
						if( p_csec->z[ 1 ] > p_csec2->z[ 1 ] && p_csec->z[ 0 ] < p_csec2->z[ 1 ] ) /* needs upper */
						{
							p_line->ui8_flags &= ~LINEFLAG_MIDDLE;
							p_line->ui8_flags |= LINEFLAG_UPPER;
						}
						break;
					}
				}
				if( i_line_idx2 < p_csec2->i_num_vertices )
				{
					break;
				}
			}
			if( !p_line->just_portal )
			{
				p_secline = malloc( sizeof( cline_t ) );
				*p_secline = *p_line;

				p_secline->p_next = p_csec->p_bsp_lines;
				p_csec->p_bsp_lines = p_secline;
			}
		}
	}
}


void set_sector_connector( csector_t *p_csec )
{
	cline_t *p_cline;
	portal_t *p_portal, *p_nextp;
	connector_t *ps_sec_connector;
	int i_idx, i_vcount = 0;

	p_csec->i_connector = i_num_connectors++;
	ps_sec_connector = &rgs_connectors[ p_csec->i_connector ];
	ps_sec_connector->i_num_connection = 0;

	ps_sec_connector->v_origin[ 0 ] = 0;
	ps_sec_connector->v_origin[ 1 ] = 0;

	for( p_cline = p_csec->p_bsp_lines; p_cline; p_cline = p_cline->p_next )
	{
		ps_sec_connector->v_origin[ 0 ] += p_cline->v1[ 0 ];
		ps_sec_connector->v_origin[ 1 ] += p_cline->v1[ 1 ];
		ps_sec_connector->v_origin[ 0 ] += p_cline->v2[ 0 ];
		ps_sec_connector->v_origin[ 1 ] += p_cline->v2[ 1 ];
		i_vcount+=2;
	}
	for( p_portal = p_csec->p_node->p_portals; p_portal; p_portal = p_nextp )
	{
		if( p_portal->p_front == p_csec->p_node )
		{
			p_nextp = p_portal->p_nextf;
		}
		else
		{
			p_nextp = p_portal->p_nextb;
		}
		ps_sec_connector->v_origin[ 0 ] += p_portal->v1[ 0 ];
		ps_sec_connector->v_origin[ 1 ] += p_portal->v1[ 1 ];
		ps_sec_connector->v_origin[ 0 ] += p_portal->v2[ 0 ];
		ps_sec_connector->v_origin[ 1 ] += p_portal->v2[ 1 ];
		i_vcount+=2;
	}

	ps_sec_connector->v_origin[ 0 ] /= i_vcount;
	ps_sec_connector->v_origin[ 1 ] /= i_vcount;
	ps_sec_connector->f_radius = 256.0;
}


int generate_portal_connector( portal_t *p_portal )
{
	int i_idx;
	connector_t *ps_sec_connector, *ps_line_connector;
	vec2_t connection_vert1, connection_vert2;
	cline_t *p_line, *p_line2;
	csector_t *rgp_secs[ 2 ];
	float rgf_sec_radius[ 2 ] = { 256.0, 256.0 };

	connection_vert1[ 0 ] = p_portal->v1[ 0 ];
	connection_vert1[ 1 ] = p_portal->v1[ 1 ];
	connection_vert2[ 0 ] = p_portal->v2[ 0 ];
	connection_vert2[ 1 ] = p_portal->v2[ 1 ];

	ps_line_connector = &rgs_connectors[ i_num_connectors ];

	ps_line_connector->v_origin[ 0 ] = ( connection_vert1[ 0 ] + connection_vert2[ 0 ] ) / 2;
	ps_line_connector->v_origin[ 1 ] = ( connection_vert1[ 1 ] + connection_vert2[ 1 ] ) / 2;
	p_portal->i_connector = i_num_connectors;


	/* determine maximum radius that can pass through this portal */
	rgp_secs[ 0 ] = p_portal->p_front->p_sec;
	rgp_secs[ 1 ] = p_portal->p_back->p_sec;

	for( i_idx = 0; i_idx < 2; i_idx++ )
	{
		float f_radius = -1.0;
		if( !rgp_secs[ i_idx ] )
		{
			continue;
		}
		p_line = rgp_secs[ i_idx ]->p_bsp_lines;

		while( p_line )
		{
			float f_dist;

			if( !p_line->p_backsec )
			{
				if( point_dist( p_portal->v1, p_line->v1 ) < 0.1 ||
					point_dist( p_portal->v1, p_line->v2 ) < 0.1 )
				{
					/* check if opposite side of portal is connected to a line also */
					p_line2 = rgp_secs[ i_idx ]->p_bsp_lines;

					while( p_line2 )
					{
						if( p_line2->p_backsec )
						{
							p_line2 = p_line2->p_next;
							continue;
						}
						if( point_dist( p_portal->v2, p_line2->v1 ) < 0.1 ||
							point_dist( p_portal->v2, p_line2->v2 ) < 0.1 )
						{
							f_dist = p_portal->v2[ 0 ] * p_line->plane->norm[ 0 ] + p_portal->v2[ 1 ] * p_line->plane->norm[ 1 ] - p_line->plane->length;
							if( f_dist / 2 > f_radius )
							{
								f_radius = f_dist / 2;
							}
							break;
						}
						p_line2 = p_line2->p_next;
					}
				}

				if( point_dist( p_portal->v2, p_line->v1 ) < 0.1 ||
					point_dist( p_portal->v2, p_line->v2 ) < 0.1 )
				{
					/* check if opposite side of portal is connected to a line also */
					p_line2 = rgp_secs[ i_idx ]->p_bsp_lines;

					while( p_line2 )
					{
						if( p_line2->p_backsec )
						{
							p_line2 = p_line2->p_next;
							continue;
						}
						if( point_dist( p_portal->v1, p_line2->v1 ) < 0.1 ||
							point_dist( p_portal->v1, p_line2->v2 ) < 0.1 )
						{
							f_dist = p_portal->v1[ 0 ] * p_line->plane->norm[ 0 ] + p_portal->v1[ 1 ] * p_line->plane->norm[ 1 ] - p_line->plane->length;
							if( f_dist / 2 > f_radius )
							{
								f_radius = f_dist / 2;
							}
							break;
						}
						p_line2 = p_line2->p_next;
					}
				}
			}
			p_line = p_line->p_next;
		}
		if( f_radius == -1.0 )
		{
			f_radius = 256.0;
		}
		rgf_sec_radius[ i_idx ] = f_radius;
	}

	ps_line_connector->f_radius = rgf_sec_radius[ 0 ] < rgf_sec_radius[ 1 ] ? rgf_sec_radius[ 0 ] : rgf_sec_radius[ 1 ];

	return i_num_connectors++;
}


void prepare_connectors( )
{
	int i_sec1, i_sec2;
	int i_idx, i_con_idx, i_sec_idx, i_line_idx, i_line_idx2;
	csector_t *p_csec, *p_csec2;
	cline_t *p_line, *p_nextline, *p_line2, *p_secline;
	connector_t *ps_sec_connector, *ps_line_connector;
	portal_t *p_portal, *p_nextp;

	for( i_sec1 = 0; i_sec1 < i_num_csectors_in_tree; i_sec1++ )
	{
		p_csec = p_csectors_in_tree[ i_sec1 ];
		p_csec->i_connector = -1;
		for( p_portal = p_csec->p_node->p_portals; p_portal; p_portal = p_nextp )
		{
			if( p_portal->p_front == p_csec->p_node )
			{
				p_nextp = p_portal->p_nextf;
			}
			else
			{
				p_nextp = p_portal->p_nextb;
			}
			p_portal->i_connector = -1;
		}
	}

	for( i_sec1 = 0; i_sec1 < i_num_csectors_in_tree; i_sec1++ )
	{
		p_csec = p_csectors_in_tree[ i_sec1 ];

		if( p_csec->i_secnum == 6 )
		{
			p_csec = p_csec;
		}

		if( p_csec->i_connector < 0 )
		{
			set_sector_connector( p_csec );
		}
		ps_sec_connector = &rgs_connectors[ p_csec->i_connector ];

		for( p_portal = p_csec->p_node->p_portals; p_portal; p_portal = p_nextp )
		{
			if( p_portal->p_front == p_csec->p_node )
			{
				p_nextp = p_portal->p_nextf;
				p_csec2 = p_portal->p_back->p_sec;
			}
			else
			{
				p_nextp = p_portal->p_nextb;
				p_csec2 = p_portal->p_front->p_sec;
			}

			if( p_csec2 == NULL )
			{
				continue;
			}

			if( p_portal->i_connector < 0 )
			{
				int i_connector_num;
				connector_t *ps_backsec_connector;
				vec2_t v_middle;

				i_connector_num = generate_portal_connector( p_portal );
				ps_line_connector = &rgs_connectors[ i_connector_num ];
		
				if( ps_sec_connector->i_num_connection >= MAX_CONNECTOR_CONNECTIONS )
				{
					printf("ERROR MAX_CONNECTOR_CONNECTIONS!!!\n");
				}
				ps_sec_connector->rgi_connections[ ps_sec_connector->i_num_connection++ ] = i_connector_num;

				if( p_csec2->i_connector < 0 )
				{
					set_sector_connector( p_csec2 );
				}
				ps_backsec_connector = &rgs_connectors[ p_csec2->i_connector ];
							
				if( ps_backsec_connector->i_num_connection >= MAX_CONNECTOR_CONNECTIONS )
				{
					printf("ERROR MAX_CONNECTOR_CONNECTIONS!!!\n");
				}
				ps_backsec_connector->rgi_connections[ ps_backsec_connector->i_num_connection++ ] = i_connector_num;

				ps_line_connector->i_num_connection = 2;
				ps_line_connector->rgi_connections[ 0 ] = p_csec->i_connector;
				ps_line_connector->rgi_connections[ 1 ] = p_csec2->i_connector;
			}
		}
	}
	for( i_idx = 0; i_idx < i_num_connectors; i_idx++ )
	{
		for( i_con_idx = 0; i_con_idx < rgs_connectors[ i_idx ].i_num_connection; i_con_idx++ )
		{
			vec2_t delt;
			connector_t *ps_other;

			ps_other = &rgs_connectors[ rgs_connectors[ i_idx ].rgi_connections[ i_con_idx ] ];
			delt[ 0 ] = rgs_connectors[ i_idx ].v_origin[ 0 ] - ps_other->v_origin[ 0 ];
			delt[ 1 ] = rgs_connectors[ i_idx ].v_origin[ 1 ] - ps_other->v_origin[ 1 ];

			rgs_connectors[ i_idx ].rgf_connections_dist[ i_con_idx ] = sqrt( delt[ 0 ] * delt[ 0 ] + delt[ 1 ] * delt[ 1 ] );
		}
	}
}



void connect_sectors( )
{
	int i_sec1, i_sec2;
	int i_idx, i_con_idx, i_sec_idx, i_line_idx, i_line_idx2;
	csector_t *p_csec, *p_csec2;
	cline_t *p_line, *p_nextline, *p_line2;

	for( i_sec1 = 0; i_sec1 < i_num_csectors_in_tree; i_sec1++ )
	{
		p_csec = p_csectors_in_tree[ i_sec1 ];

		for( p_line = p_csec->p_bsp_lines; p_line; p_line = p_line->p_next )
		{
			p_line->p_backsec = 0;
			for( i_sec2 = 0; i_sec2 < i_num_csectors_in_tree; i_sec2++ )
			{
				int i_side;
				if( i_sec1 == i_sec2 )
				{
					continue;
				}
				p_csec2 = p_csectors_in_tree[ i_sec2 ];
				for( p_line2 = p_csec2->p_bsp_lines; p_line2; p_line2 = p_line2->p_next )
				{
					vec2_t delt1;
					float d1, d2, d3, d4;
					i_side = plane_compare( p_line->plane, p_line2->plane );
					if( i_side == -1 )
					{
						delt1[ 0 ] = p_line->v2[ 0 ] - p_line->v1[ 0 ];
						delt1[ 1 ] = p_line->v2[ 1 ] - p_line->v1[ 1 ];
						d1 = sqrt( delt1[ 0 ] * delt1[ 0 ] + delt1[ 1 ] * delt1[ 1 ] );
						delt1[ 0 ] /= d1;
						delt1[ 1 ] /= d1;
						d1 = ( delt1[ 0 ] * p_line->v1[ 0 ] ) + ( delt1[ 1 ] * p_line->v1[ 1 ] );
						d2 = ( delt1[ 0 ] * p_line->v2[ 0 ] ) + ( delt1[ 1 ] * p_line->v2[ 1 ] ) - d1;

						d3 = ( p_line2->v1[ 0 ] * delt1[ 0 ] ) + ( p_line2->v1[ 1 ] * delt1[ 1 ] ) - d1;
						d4 = ( p_line2->v2[ 0 ] * delt1[ 0 ] ) + ( p_line2->v2[ 1 ] * delt1[ 1 ] ) - d1;
						if( d4 >= d3 )
						{
							int *pi_null = 0;
							*pi_null = 0;
						}
						if( ( d3 <= 0.1 && d4 < 0.1 ) || ( d3 >= (d2-0.1) && d4 >= (d2-0.1) ) )
						{
							continue;
						}
						/* there is some overlap, connect line to backsector */
						p_line->p_backsec = p_csec2;
						break;
					}
				}
				if( p_line2 ) /* backsector found */
				{
					break;
				}
			}
		}
	}
}




char rgi8_csector_visibility[ 0x10000 ];
char rgi8_node_checked[ 0x10000 ];
int i_pvs_sector_under_check;

void get_portal_plane( portal_t *p_portal )
{
	float f_length;
	vec2_t norm;
	norm[ 1 ] = ( p_portal->v2[ 0 ] - p_portal->v1[ 0 ] );
	norm[ 0 ] = -( p_portal->v2[ 1 ] - p_portal->v1[ 1 ] );
	f_length = norm[ 0 ] * norm[ 0 ] + norm[ 1 ] * norm[ 1 ];
	f_length = sqrt( f_length );
	norm[ 0 ] = norm[ 0 ] / f_length;
	norm[ 1 ] = norm[ 1 ] / f_length;
	p_portal->s_plane.norm[ 0 ] = norm[ 0 ];
	p_portal->s_plane.norm[ 1 ] = norm[ 1 ];
	p_portal->s_plane.length = norm[ 0 ] * p_portal->v1[ 0 ] + norm[ 1 ] * p_portal->v1[ 1 ];
}

portal_t *get_portal_from_plane( cplane_t *p_plane )
{
	portal_t *p_portal;
	vec2_t v1, delt;

	v1[ 0 ] = p_plane->norm[ 0 ] * p_plane->length;
	v1[ 1 ] = p_plane->norm[ 1 ] * p_plane->length;

	delt[ 0 ] = p_plane->norm[ 1 ];
	delt[ 1 ] = -p_plane->norm[ 0 ];

	p_portal = malloc( sizeof( portal_t ) );
	memset( p_portal, 0, sizeof( portal_t ) );
	p_portal->v1[ 0 ] = v1[ 0 ] - ( delt[ 0 ] * 16000.0 );
	p_portal->v1[ 1 ] = v1[ 1 ] - ( delt[ 1 ] * 16000.0 );
	p_portal->v2[ 0 ] = v1[ 0 ] + ( delt[ 0 ] * 16000.0 );
	p_portal->v2[ 1 ] = v1[ 1 ] + ( delt[ 1 ] * 16000.0 );
	p_portal->s_plane = *p_plane;

	return p_portal;
}


void clip_portal( portal_t *p_portal, cplane_t *p_clipplane, portal_t **pp_front, portal_t **pp_back )
{
	portal_t *p_front, *p_back;
	float f_scale, f_dist1, f_dist2;

	f_dist1 = p_portal->v1[ 0 ] * p_clipplane->norm[ 0 ] + p_portal->v1[ 1 ] * p_clipplane->norm[ 1 ] - p_clipplane->length;
	f_dist2 = p_portal->v2[ 0 ] * p_clipplane->norm[ 0 ] + p_portal->v2[ 1 ] * p_clipplane->norm[ 1 ] - p_clipplane->length;

	if( pp_back )
	{
		*pp_back = 0;
	}
	*pp_front = 0;

	if( f_dist1 < 0.01 && f_dist2 < 0.01 )
	{
		if( pp_back )
		{
			*pp_back = p_portal;
		}
		else
		{
			free( p_portal );
		}
		return;
	}
	else if( f_dist1 > -0.01 && f_dist2 > -0.01 )
	{
		*pp_front = p_portal;
		return;
	}
	else
	{
		vec2_t v_split;
		p_front = malloc( sizeof( portal_t ) );
		p_back = malloc( sizeof( portal_t ) );

		*p_front = *p_portal;
		*p_back = *p_portal;

		f_scale = f_dist2 / ( f_dist2 - f_dist1 );

		v_split[ 0 ] = p_portal->v2[ 0 ] + ( p_portal->v1[ 0 ] - p_portal->v2[ 0 ] ) * f_scale;
		v_split[ 1 ] = p_portal->v2[ 1 ] + ( p_portal->v1[ 1 ] - p_portal->v2[ 1 ] ) * f_scale;

		if( f_dist1 < 0 )
		{
			(p_front)->v1[ 0 ] = v_split[ 0 ];
			(p_front)->v1[ 1 ] = v_split[ 1 ];
			(p_back)->v2[ 0 ] = v_split[ 0 ];
			(p_back)->v2[ 1 ] = v_split[ 1 ];
		}
		else
		{
			(p_front)->v2[ 0 ] = v_split[ 0 ];
			(p_front)->v2[ 1 ] = v_split[ 1 ];
			(p_back)->v1[ 0 ] = v_split[ 0 ];
			(p_back)->v1[ 1 ] = v_split[ 1 ];
		}

		free( p_portal );
	}

	if( pp_front )
	{
		*pp_front = p_front;
	}
	else if( p_front )
	{
		free( p_front );
	}
	if( pp_back )
	{
		*pp_back = p_back;
	}
	else if( p_back )
	{
		free( p_back );
	}
}

void get_norm( vec2_t from, vec2_t to, vec2_t normal )
{
	normal[ 1 ] = ( to[ 0 ] - from[ 0 ] );
	normal[ 0 ] = -( to[ 1 ] - from[ 1 ] );
}


void unlink_portal( portal_t *ps_portal )
{
	portal_t *p_portals, **pp_ptr, *p_next;
	bspnode_t *ps_node;

	ps_node = ps_portal->p_front;
	pp_ptr = &ps_node->p_portals;
	while( *pp_ptr != ps_portal && *pp_ptr != 0 )
	{
		pp_ptr = ( ps_node == (*pp_ptr)->p_front ) ? &(*pp_ptr)->p_nextf : &(*pp_ptr)->p_nextb;
	}
	if( *pp_ptr )
	{
		*pp_ptr = ps_portal->p_nextf;
	}
	else
	{
		printf("BADLY LINKED PORTAL\n");
	}

	ps_node = ps_portal->p_back;
	pp_ptr = &ps_node->p_portals;
	while( *pp_ptr != ps_portal && *pp_ptr != 0 )
	{
		pp_ptr = ( ps_node == (*pp_ptr)->p_front ) ? &(*pp_ptr)->p_nextf : &(*pp_ptr)->p_nextb;
	}
	if( *pp_ptr )
	{
		*pp_ptr = ps_portal->p_nextb;
	}
	else
	{
		printf("BADLY LINKED PORTAL\n");
	}
}



portal_t *clip_portal_against_line( portal_t *p_portals, cline_t *p_line, portal_t **pp_split )
{
	int i_side;
	if( ( i_side = plane_compare( &p_portals->s_plane, p_line->plane ) ) )
	{
		vec2_t delt1, connection_vert1, connection_vert2, lv1, lv2;
		float d1, d2, d3, d4;
		delt1[ 0 ] = p_portals->v2[ 0 ] - p_portals->v1[ 0 ];
		delt1[ 1 ] = p_portals->v2[ 1 ] - p_portals->v1[ 1 ];
		d1 = sqrt( delt1[ 0 ] * delt1[ 0 ] + delt1[ 1 ] * delt1[ 1 ] );
		delt1[ 0 ] /= d1;
		delt1[ 1 ] /= d1;
		d1 = ( delt1[ 0 ] * p_portals->v1[ 0 ] ) + ( delt1[ 1 ] * p_portals->v1[ 1 ] );
		d2 = ( delt1[ 0 ] * p_portals->v2[ 0 ] ) + ( delt1[ 1 ] * p_portals->v2[ 1 ] ) - d1;

		if( i_side == -1 )
		{
			lv1[ 0 ] = p_line->v1[ 0 ];
			lv1[ 1 ] = p_line->v1[ 1 ];
			lv2[ 0 ] = p_line->v2[ 0 ];
			lv2[ 1 ] = p_line->v2[ 1 ];
		}
		else
		{
			lv1[ 0 ] = p_line->v2[ 0 ];
			lv1[ 1 ] = p_line->v2[ 1 ];
			lv2[ 0 ] = p_line->v1[ 0 ];
			lv2[ 1 ] = p_line->v1[ 1 ];
		}

		d3 = ( lv1[ 0 ] * delt1[ 0 ] ) + ( lv1[ 1 ] * delt1[ 1 ] ) - d1;
		d4 = ( lv2[ 0 ] * delt1[ 0 ] ) + ( lv2[ 1 ] * delt1[ 1 ] ) - d1;

		if( d4 >= d3 )
		{
			int *pi_null = 0;
			*pi_null = 0;
		}
		if( ( d3 <= 0.1 && d4 < 0.1 ) || ( d3 >= (d2-0.1) && d4 >= (d2-0.1) ) )
		{
			return p_portals;
		}

		if( d4 <= 0.1 && d3 >= d2-0.1 )
		{
			free( p_portals );
			return NULL;
		}
		else if( d4 <= 0.1 && d3 < d2-0.1 )
		{
			p_portals->v1[ 0 ] = lv1[ 0 ];
			p_portals->v1[ 1 ] = lv1[ 1 ];
			return p_portals;
		}
		else if( d4 > 0.1 && d3 >= d2-0.1 )
		{
			p_portals->v2[ 0 ] = lv2[ 0 ];
			p_portals->v2[ 1 ] = lv2[ 1 ];
			return p_portals;
		}
		else
		{
			*pp_split = malloc( sizeof( portal_t ) );
			**pp_split = *p_portals;

			p_portals->v2[ 0 ] = lv2[ 0 ];
			p_portals->v2[ 1 ] = lv2[ 1 ];
			(*pp_split)->v1[ 0 ] = lv1[ 0 ];
			(*pp_split)->v1[ 1 ] = lv1[ 1 ];
			return p_portals;
		}
	}
	else
	{
		return p_portals;
	}
}

void check_portal( portal_t *p_portal )
{
	vec2_t delt;
	float f_dist;
	delt[ 0 ] = p_portal->v1[ 0 ] - p_portal->v2[ 0 ];
	delt[ 1 ] = p_portal->v1[ 1 ] - p_portal->v2[ 1 ];
	f_dist = sqrt( delt[ 0 ] * delt[ 0 ] + delt[ 1 ] * delt[ 1 ] );

	if( f_dist < 0.5 )
	{
		printf("detected bad portal\n");
	}
}

int i_counter_portalize = 0;
void portalize_nodes_r( bspnode_t *p_node, int i_skip_lineclip )
{
	portal_t *p_frontl, *p_backl, *p_portal, *p_frontp, *p_backp, *p_nextp, *p_portals;
	if( p_node->i_type == BSPNODE_SPLIT )
	{
		bspnode_t *p_parent, *p_one_below;

		if( !p_node->i_no_portal )
		{
			p_portal = get_portal_from_plane( p_node->p_plane );
			p_portal->p_front = p_node->p_front;
			p_portal->p_back = p_node->p_back;

			p_one_below = p_node;
			i_counter_portalize++;
			if( i_counter_portalize == 1 )
			{
				i_counter_portalize = i_counter_portalize;
			}
			for( p_parent = p_node->p_parent; p_parent; p_parent = p_parent->p_parent )
			{
				clip_portal( p_portal, p_parent->p_plane, &p_frontp, &p_backp );
				if( p_parent->p_front == p_one_below )
				{
					p_portal = p_frontp;
					free( p_backp );
				}
				else
				{
					p_portal = p_backp;
					free( p_frontp );
				}
				if( !p_portal )
				{
					printf("WARNING: PORTAL WAS CLIPPED AWAY\n");
					break;
				}
				p_one_below = p_parent;
			}
			if( fabs( p_portal->v1[ 0 ] ) >= 1300 ||
				fabs( p_portal->v1[ 1 ] ) >= 1300 ||
				fabs( p_portal->v2[ 0 ] ) >= 1300 ||
				fabs( p_portal->v2[ 1 ] ) >= 1300 )
			{
				p_portal = p_portal; /* this shall not happen for the cliptree as the cliptree needs valid portal planes*/
			}
		}
		else
		{
			p_portal = 0;
		}

		p_frontl = p_backl = 0;

		for( p_portals = p_node->p_portals; p_portals; p_portals = p_nextp )
		{
			if( p_portals->p_front == p_node )
			{
				p_nextp = p_portals->p_nextf;
			}
			else if( p_portals->p_back == p_node )
			{
				p_nextp = p_portals->p_nextb;
			}
			else
			{
				int *pi_null = 0;
				printf("MISSLINKED PORTAL\n");
				*pi_null = 0;
			}
			unlink_portal( p_portals );
			clip_portal( p_portals, p_node->p_plane, &p_frontp, &p_backp );
			if( p_frontp )
			{
				/* update linked node */
				if( p_frontp->p_front == p_node )
				{
					p_frontp->p_nextf = p_frontl;
					p_frontp->p_front = p_node->p_front;

					p_frontp->p_nextb = p_frontp->p_back->p_portals;
					p_frontp->p_back->p_portals = p_frontp;
				}
				else if( p_frontp->p_back == p_node )
				{
					p_frontp->p_nextb = p_frontl;
					p_frontp->p_back = p_node->p_front;

					p_frontp->p_nextf = p_frontp->p_front->p_portals;
					p_frontp->p_front->p_portals = p_frontp;
				}
				else
				{
					int *pi_null = 0;
					printf("MISSLINKED PORTAL\n");
					*pi_null = 0;
				}
				p_frontl = p_frontp;
			}
			if( p_backp )
			{
				/* update linked node */
				if( p_backp->p_front == p_node )
				{
					p_backp->p_nextf = p_backl;
					p_backp->p_front = p_node->p_back;

					p_backp->p_nextb = p_backp->p_back->p_portals;
					p_backp->p_back->p_portals = p_backp;
				}
				else if( p_backp->p_back == p_node )
				{
					p_backp->p_nextb = p_backl;
					p_backp->p_back = p_node->p_back;

					p_backp->p_nextf = p_backp->p_front->p_portals;
					p_backp->p_front->p_portals = p_backp;
				}
				else
				{
					int *pi_null = 0;
					printf("MISSLINKED PORTAL\n");
					*pi_null = 0;
				}
				p_backl = p_backp;
			}
		}

		if( p_portal )
		{
			p_portal->p_nextf = p_frontl;
			p_portal->p_nextb = p_backl;
			p_node->p_front->p_portals = p_portal;
			p_node->p_back->p_portals = p_portal;
		}
		portalize_nodes_r( p_node->p_front, i_skip_lineclip );
		portalize_nodes_r( p_node->p_back, i_skip_lineclip );
	}
	else
	{
		cline_t *p_line;

		if( p_node->p_sec && !i_skip_lineclip )
		{
			if( p_node->p_sec->i_secnum == 21 )
			{
				p_node = p_node;
			}
			p_line = p_node->p_sec->p_bsp_lines;
			for( p_line; p_line; p_line = p_line->p_next )
			{
				int i_side, i_idx;
				portal_t *rgp_ports[ 2 ];

				if( p_line->p_backsec )
				{
					continue;
				}

				for( p_portals = p_node->p_portals; p_portals; p_portals = p_nextp )
				{
					if( p_portals->p_front == p_node )
					{
						p_nextp = p_portals->p_nextf;
					}
					else if( p_portals->p_back == p_node )
					{
						p_nextp = p_portals->p_nextb;
					}
					else
					{
						int *pi_null = 0;
						printf("MISSLINKED PORTAL\n");
						*pi_null = 0;
					}
					unlink_portal( p_portals );

					i_debug_counter3++;

					if( i_debug_counter3 == 18 )
					{
						i_debug_counter3 = i_debug_counter3;
					}
					rgp_ports[ 0 ] = rgp_ports[ 1 ] = 0;
					rgp_ports[ 0 ] = clip_portal_against_line( p_portals, p_line, &rgp_ports[ 1 ] );

					for( i_idx = 0; i_idx < 2; i_idx++ )
					{
						if( rgp_ports[ i_idx ] )
						{
							check_portal( rgp_ports[ i_idx ] );
							/* update linked node */
							if( rgp_ports[ i_idx ]->p_front == p_node )
							{
								rgp_ports[ i_idx ]->p_nextf = p_node->p_portals;
								p_node->p_portals = rgp_ports[ i_idx ];

								rgp_ports[ i_idx ]->p_nextb = rgp_ports[ i_idx ]->p_back->p_portals;
								rgp_ports[ i_idx ]->p_back->p_portals = rgp_ports[ i_idx ];
							}
							else if( rgp_ports[ i_idx ]->p_back == p_node )
							{
								rgp_ports[ i_idx ]->p_nextb = p_node->p_portals;
								p_node->p_portals = rgp_ports[ i_idx ];

								rgp_ports[ i_idx ]->p_nextf = rgp_ports[ i_idx ]->p_front->p_portals;
								rgp_ports[ i_idx ]->p_front->p_portals = rgp_ports[ i_idx ];
							}
							else
							{
								int *pi_null = 0;
								printf("MISSLINKED PORTAL\n");
								*pi_null = 0;
							}
						}
					}
				}
			}
		}
	}
}



int i_curr_src_sector;

void pvs_sector_r( portal_t *p_portal, portal_t *p_fromportal, bspnode_t *p_currnode, int i_depth )
{
#if 1
	bspnode_t *p_other_node;
	portal_t *p_portal_copy, *p_nportal, *p_node_portals, *p_leftovers, *p_nextp;

	int i_line_idx, i_line_idx2, i_side;
	float f_flip;
	rgi8_node_checked[ p_currnode->i_nodenum ] = 1;

	if( p_currnode->p_sec )
	{
		rgi8_csector_visibility[ p_currnode->p_sec->i_secnum ] = 1;
	}

	p_node_portals = p_currnode->p_portals;
	while( p_node_portals )
	{
		int i_valid_p1, i_valid_p2, i_inverted_from_portal;
		vec2_t norm;
		float f_length;
		cplane_t s_splitplane1, s_splitplane2;
		csector_t *p_backsec;

		if( p_node_portals->p_front == p_currnode )
		{
			p_other_node = p_node_portals->p_back;
			p_nextp = p_node_portals->p_nextf;
		}
		else
		{
			p_other_node = p_node_portals->p_front;
			p_nextp = p_node_portals->p_nextb;
		}

		if( rgi8_node_checked[ p_other_node->i_nodenum ] )
		{
			p_node_portals = p_nextp;
			continue;
		}

		if( p_other_node->p_sec && (
			p_other_node->p_sec->i_secnum == 2 ||
			p_other_node->p_sec->i_secnum == 3 ||
			p_other_node->p_sec->i_secnum == 99 ||
			p_other_node->p_sec->i_secnum == 99 ||
			p_other_node->p_sec->i_secnum == 99
			 ) )
		{
			p_other_node = p_other_node;
		}


		p_portal_copy = malloc( sizeof( portal_t ) );
		*p_portal_copy = *p_portal;

		p_nportal = malloc( sizeof( portal_t ) );
		*p_nportal = *p_node_portals;

		if( p_node_portals->p_front == p_currnode )
		{
			clip_portal( p_portal_copy, &p_nportal->s_plane, &p_portal_copy, NULL );
		}
		else
		{
			clip_portal( p_portal_copy, &p_nportal->s_plane, &p_leftovers, &p_portal_copy );
			if( p_leftovers )
			{
				free( p_leftovers );
			}
		}
		if( !p_portal_copy )
		{
			p_node_portals = p_nextp;
			continue;
		}
		

		if( i_depth > 0 )
		{
			i_inverted_from_portal = 0;
			i_valid_p1 = 0;
			get_norm( p_fromportal->v1, p_portal->v1, norm );
			f_length = norm[ 0 ] * norm[ 0 ] + norm[ 1 ] * norm[ 1 ];
			if( f_length > 0.01 )
			{
				f_length = sqrt( f_length );
				norm[ 0 ] = norm[ 0 ] / f_length;
				norm[ 1 ] = norm[ 1 ] / f_length;
				s_splitplane1.i_planenum = -1;
				s_splitplane1.norm[ 0 ] = norm[ 0 ];
				s_splitplane1.norm[ 1 ] = norm[ 1 ];
				s_splitplane1.length = norm[ 0 ] * p_portal->v1[ 0 ] + norm[ 1 ] * p_portal->v1[ 1 ];

				f_length = ( p_portal->v2[ 0 ] * norm[ 0 ] + p_portal->v2[ 1 ] * norm[ 1 ] - s_splitplane1.length );
				if( f_length >= 0.01 )
				{
					s_splitplane1.norm[ 0 ] = -s_splitplane1.norm[ 0 ];
					s_splitplane1.norm[ 1 ] = -s_splitplane1.norm[ 1 ];
					s_splitplane1.length = -s_splitplane1.length;
				}

				if( ( p_fromportal->v2[ 0 ] * s_splitplane1.norm[ 0 ] + p_fromportal->v2[ 1 ] * s_splitplane1.norm[ 1 ] - s_splitplane1.length ) < 0.01 )
				{
					if( fabs( f_length ) < 0.01 )
					{
						s_splitplane1.norm[ 0 ] = -s_splitplane1.norm[ 0 ];
						s_splitplane1.norm[ 1 ] = -s_splitplane1.norm[ 1 ];
						s_splitplane1.length = -s_splitplane1.length;
						i_valid_p1 = 1;
					}
					else
					{
						i_inverted_from_portal = 1;
					}
				}
				else
				{
					i_valid_p1 = 1;
				}
			}
			else
			{
				i_inverted_from_portal = 1;
			}

			if( i_inverted_from_portal )
			{
				get_norm( p_fromportal->v2, p_portal->v1, norm );
				f_length = norm[ 0 ] * norm[ 0 ] + norm[ 1 ] * norm[ 1 ];
				if( f_length > 0.01 )
				{
					f_length = sqrt( f_length );
					norm[ 0 ] = norm[ 0 ] / f_length;
					norm[ 1 ] = norm[ 1 ] / f_length;
					s_splitplane1.i_planenum = -1;
					s_splitplane1.norm[ 0 ] = norm[ 0 ];
					s_splitplane1.norm[ 1 ] = norm[ 1 ];
					s_splitplane1.length = norm[ 0 ] * p_portal->v1[ 0 ] + norm[ 1 ] * p_portal->v1[ 1 ];

					f_length = ( p_portal->v2[ 0 ] * norm[ 0 ] + p_portal->v2[ 1 ] * norm[ 1 ] - s_splitplane1.length );
					if( f_length >= 0.01 )
					{
						s_splitplane1.norm[ 0 ] = -s_splitplane1.norm[ 0 ];
						s_splitplane1.norm[ 1 ] = -s_splitplane1.norm[ 1 ];
						s_splitplane1.length = -s_splitplane1.length;
					}

					if( ( p_fromportal->v1[ 0 ] * s_splitplane1.norm[ 0 ] + p_fromportal->v1[ 1 ] * s_splitplane1.norm[ 1 ] - s_splitplane1.length ) > 0.01 )
					{
						i_valid_p1 = 1;
					}
					else if( fabs( f_length ) < 0.01 )
					{
						s_splitplane1.norm[ 0 ] = -s_splitplane1.norm[ 0 ];
						s_splitplane1.norm[ 1 ] = -s_splitplane1.norm[ 1 ];
						s_splitplane1.length = -s_splitplane1.length;
						i_valid_p1 = 1;
					}
				}
			}

			if( i_valid_p1 )
			{
				clip_portal( p_nportal, &s_splitplane1, &p_nportal, NULL );
				if( !p_nportal )
				{
					p_node_portals = p_nextp;
					continue;
				}
			}



			i_inverted_from_portal = 0;
			i_valid_p1 = 0;
			get_norm( p_fromportal->v1, p_portal->v2, norm );
			f_length = norm[ 0 ] * norm[ 0 ] + norm[ 1 ] * norm[ 1 ];
			if( f_length > 0.01 )
			{
				f_length = sqrt( f_length );
				norm[ 0 ] = norm[ 0 ] / f_length;
				norm[ 1 ] = norm[ 1 ] / f_length;
				s_splitplane1.i_planenum = -1;
				s_splitplane1.norm[ 0 ] = norm[ 0 ];
				s_splitplane1.norm[ 1 ] = norm[ 1 ];
				s_splitplane1.length = norm[ 0 ] * p_portal->v2[ 0 ] + norm[ 1 ] * p_portal->v2[ 1 ];

				f_length = ( p_portal->v1[ 0 ] * norm[ 0 ] + p_portal->v1[ 1 ] * norm[ 1 ] - s_splitplane1.length );
				if( f_length >= 0.01 )
				{
					s_splitplane1.norm[ 0 ] = -s_splitplane1.norm[ 0 ];
					s_splitplane1.norm[ 1 ] = -s_splitplane1.norm[ 1 ];
					s_splitplane1.length = -s_splitplane1.length;
				}

				if( ( p_fromportal->v2[ 0 ] * s_splitplane1.norm[ 0 ] + p_fromportal->v2[ 1 ] * s_splitplane1.norm[ 1 ] - s_splitplane1.length ) < 0.01 )
				{
					if( fabs( f_length ) > 0.01 )
					{
						i_inverted_from_portal = 1;
					}
					else
					{
						s_splitplane1.norm[ 0 ] = -s_splitplane1.norm[ 0 ];
						s_splitplane1.norm[ 1 ] = -s_splitplane1.norm[ 1 ];
						s_splitplane1.length = -s_splitplane1.length;
						i_valid_p1 = 1;
					}
				}
				else
				{
					i_valid_p1 = 1;
				}
			}
			else
			{
				i_inverted_from_portal = 1;
			}

			if( i_inverted_from_portal )
			{
				get_norm( p_fromportal->v2, p_portal->v2, norm );
				f_length = norm[ 0 ] * norm[ 0 ] + norm[ 1 ] * norm[ 1 ];
				if( f_length > 0.01 )
				{
					f_length = sqrt( f_length );
					norm[ 0 ] = norm[ 0 ] / f_length;
					norm[ 1 ] = norm[ 1 ] / f_length;
					s_splitplane1.i_planenum = -1;
					s_splitplane1.norm[ 0 ] = norm[ 0 ];
					s_splitplane1.norm[ 1 ] = norm[ 1 ];
					s_splitplane1.length = norm[ 0 ] * p_portal->v2[ 0 ] + norm[ 1 ] * p_portal->v2[ 1 ];

					f_length = ( p_portal->v1[ 0 ] * norm[ 0 ] + p_portal->v1[ 1 ] * norm[ 1 ] - s_splitplane1.length );
					if( f_length >= 0.01 )
					{
						s_splitplane1.norm[ 0 ] = -s_splitplane1.norm[ 0 ];
						s_splitplane1.norm[ 1 ] = -s_splitplane1.norm[ 1 ];
						s_splitplane1.length = -s_splitplane1.length;
					}

					if( ( p_fromportal->v1[ 0 ] * s_splitplane1.norm[ 0 ] + p_fromportal->v1[ 1 ] * s_splitplane1.norm[ 1 ] - s_splitplane1.length ) > 0.01 )
					{
						i_valid_p1 = 1;
					}
					else if( fabs( f_length ) < 0.01 )
					{
						s_splitplane1.norm[ 0 ] = -s_splitplane1.norm[ 0 ];
						s_splitplane1.norm[ 1 ] = -s_splitplane1.norm[ 1 ];
						s_splitplane1.length = -s_splitplane1.length;
						i_valid_p1 = 1;
					}
				}
			}

			if( i_valid_p1 )
			{
				clip_portal( p_nportal, &s_splitplane1, &p_nportal, NULL );
				if( !p_nportal )
				{
					p_node_portals = p_nextp;
					continue;
				}
			}
		}

		if( p_nportal )
		{
			pvs_sector_r( p_portal_copy, p_nportal, p_other_node, i_depth + 1 );
			free( p_portal_copy );
			free( p_nportal );
		}
		p_node_portals = p_nextp;
	}
	rgi8_node_checked[ p_currnode->i_nodenum ] = 0;
#endif
}


void pvs_tree_r( bspnode_t *ps_node )
{
	int i_sec_idx, i_line_idx, i_line_idx2;
	csector_t *p_csec;
	cline_t *p_line, *p_nextline, *p_line2;
	portal_t *ps_portals, *p_nextp;
	int i_sec1, i_sec2;

	if( ps_node->i_type == BSPNODE_LEAF  )
	{
		if( !ps_node->p_sec )
		{
			return;
		}
		p_csec = ps_node->p_sec;
		i_pvs_sector_under_check = p_csec->i_secnum;
		i_curr_src_sector = i_pvs_sector_under_check;
		if( i_pvs_sector_under_check == 17 )
		{
			i_pvs_sector_under_check = 17;
		}

		memset( rgi8_csector_visibility, 0, sizeof( rgi8_csector_visibility ) );
		rgi8_csector_visibility[ i_pvs_sector_under_check ] = 1;

		ps_portals = ps_node->p_portals;
		while( ps_portals )
		{
			portal_t *p_portal;
			p_portal = ( portal_t * )malloc( sizeof( portal_t ) );
			*p_portal = *ps_portals;

			if( ps_portals->p_front == ps_node )
			{
				pvs_sector_r( p_portal, p_portal, ps_portals->p_back, 0 );
				ps_portals = ps_portals->p_nextf;
			}
			else
			{
				pvs_sector_r( p_portal, p_portal, ps_portals->p_front, 0 );
				ps_portals = ps_portals->p_nextb;
			}
		}
		memcpy( p_csec->rgui8_visibility, rgi8_csector_visibility, i_num_csectors_in_tree );
		p_csec->i_has_visibility = 1;
	}
	else
	{
		pvs_tree_r( ps_node->p_front );
		pvs_tree_r( ps_node->p_back );
	}

}


void prepare_export();
void prepare_export_sector_visibility( );

void map_compile()
{
	csector_t *p_csectors;

	i_counter = 0;
	i_counter_split = 0;
	i_convcounter = 0;
	i_dbgcounter = 0;

	/* FIXME: memleaks everywhere */

	/* cliptree */
	p_csectors = generate_csectors( );
	prepare_sectors_lines_connectors( p_csectors );
	s_ed.p_cliptree = build_cliptree( p_csectors, 0 );
	s_ed.p_cliptree->p_parent = 0;
	portalize_nodes_r( s_ed.p_cliptree, 1 );
	bevel_cliptree_r( s_ed.p_cliptree );

	/* entity cliptree */
	s_ed.p_entitycliptree = build_entitycliptree( 0 );

	/* drawtree */
	s_ed.p_planelist = 0;
	p_csectors = generate_csectors( );
	i_num_connectors = 0;
	prepare_sectors_lines_connectors( p_csectors );
	i_num_csectors_in_tree = 0;
	i_nodenum = 0;
	s_ed.p_tree = build_bsptree_r( p_csectors );
	s_ed.p_tree->p_parent = 0;

	connect_sectors();

	/* prepare export updates the csectors_in_tree sector list */
	prepare_export();

	/* build pvs */
	i_debug_counter3 = 0;
	portalize_nodes_r( s_ed.p_tree, 0 );

	pvs_tree_r( s_ed.p_tree );

	prepare_export_sector_visibility();

	prepare_connectors();
	i_num_connectors = 0;
}


