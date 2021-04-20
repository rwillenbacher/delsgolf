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
#include <math.h>
#include <string.h>
#include <memory.h>
#include "gtk/gtk.h"

#include "editor.h"

/* Backing pixmap for drawing area */
GdkPixmap *p_offscreen_buffer = NULL;
GtkWidget *p_d2dview;
PangoLayout *p_text_layout;

GtkAdjustment *adjustment_ztop;
GtkAdjustment *adjustment_zbot;
GtkAdjustment *adjustment_snap;
GtkWidget  *window;
GdkGC *p_lgrey;

GtkEntry *p_entry_linetex_top;
GtkEntry *p_entry_linetex_bottom;
GtkEntry *p_entry_sector_floor;
GtkEntry *p_entry_sector_ceil;
GtkEntry *p_entry_entity_class;

char *pui8_current_filename;


extern int i_num_connectors;
extern connector_t rgs_connectors[ MAX_NUM_CONNECTORS ];


void ed2dview_draw_line( GdkGC *col, vec2_t v1, vec2_t v2 )
{
	int i_x1, i_y1, i_x2, i_y2;

	i_x1 = v1[ 0 ] - s_ed.origin[ 0 ];
	i_x2 = v2[ 0 ] - s_ed.origin[ 0 ];
	i_y1 = v1[ 1 ] - s_ed.origin[ 1 ];
	i_y2 = v2[ 1 ] - s_ed.origin[ 1 ];

	gdk_draw_line( p_offscreen_buffer, col, i_x1, i_y1, i_x2, i_y2 );
}

void ed2dview_draw_vert( GdkGC *col, vec2_t v1 )
{
	int i_x1, i_y1;

	i_x1 = v1[ 0 ] - s_ed.origin[ 0 ] - 1;
	i_y1 = v1[ 1 ] - s_ed.origin[ 1 ] - 1;

	gdk_draw_rectangle( p_offscreen_buffer, col, TRUE, i_x1, i_y1, 3, 3 );
}


bspnode_t *ed2dview_csector_for_origin( bspnode_t *p_node, vec2_t origin )
{
	csector_t *p_sec;
	if( p_node->i_type == BSPNODE_SPLIT )
	{
		if( ( p_node->p_plane->norm[ 0 ] * origin[ 0 ] + p_node->p_plane->norm[ 1 ] * origin[ 1 ] - p_node->p_plane->length ) < 0.0 )
		{
			return ed2dview_csector_for_origin( p_node->p_back, origin );
		}
		else
		{
			return ed2dview_csector_for_origin( p_node->p_front, origin );
		}
	}
	else
	{
		return p_node;
	}
}

void ed2dview_draw_tree_r( bspnode_t *p_node, bspnode_t *p_viewnode )
{
	int i_idx;
	GdkColor s_color;
	csector_t *p_secl;
	cline_t *p_linel;

	if( p_node->i_type == BSPNODE_SPLIT )
	{
		ed2dview_draw_tree_r( p_node->p_back, p_viewnode );
		ed2dview_draw_tree_r( p_node->p_front, p_viewnode );
	}
	else
	{
		p_secl = p_node->p_sec;
		while( p_secl )
		{
			i_idx = 0;
			p_linel = p_secl->p_bsp_lines;
			while( p_linel )
			{
				vec2_t v1, v2;
				v1[ 0 ] = p_linel->v1[ 0 ] + ( p_linel->plane->norm[ 0 ] * 2 );
				v1[ 1 ] = p_linel->v1[ 1 ] + ( p_linel->plane->norm[ 1 ] * 2 );
				v2[ 0 ] = p_linel->v2[ 0 ] + ( p_linel->plane->norm[ 0 ] * 2 );
				v2[ 1 ] = p_linel->v2[ 1 ] + ( p_linel->plane->norm[ 1 ] * 2 );
				if( p_viewnode && p_viewnode->p_sec && p_viewnode->p_sec->rgui8_visibility[ p_secl->i_secnum ] )
				{
					s_color.red = 192<<8; s_color.green = s_color.blue = 0;
				}
				else
				{
					s_color.red = 0; s_color.green = 192<<8; s_color.blue = 0;
				}
				gdk_gc_set_rgb_fg_color( p_lgrey, &s_color );
				ed2dview_draw_line( p_lgrey, v1, v2 );

				s_color.red = 0; s_color.green = s_color.blue = 64<<8;
				gdk_gc_set_rgb_fg_color( p_lgrey, &s_color );
				ed2dview_draw_vert( p_lgrey, v1 );
				ed2dview_draw_vert( p_lgrey, v2 );
				p_linel = p_linel->p_next;
				i_idx++;
			}
			p_secl = p_secl->p_next;
		}
	}
}


void ed2dview_draw_clipnodes_r( bspnode_t *p_node, vec2_t v_target )
{
	int i_idx;
	GdkColor s_color;
	csector_t *p_secl;
	cline_t *p_linel;

	if( p_node->i_type == BSPNODE_SPLIT )
	{
		float f_dist;
		portal_t *p_portal;

		p_node->p_plane->length += 12.0;
		p_portal = get_portal_from_plane( p_node->p_plane );
		s_color.red = 128<<8; s_color.green = s_color.blue = 0;
		gdk_gc_set_rgb_fg_color( p_lgrey, &s_color );
		ed2dview_draw_line( p_lgrey, p_portal->v1, p_portal->v2 );
		p_node->p_plane->length -= 12.0;
		free( p_portal );

		p_node->p_plane->length -= 12.0;
		p_portal = get_portal_from_plane( p_node->p_plane );
		s_color.red = 0; s_color.green = s_color.blue = 128<<8;
		gdk_gc_set_rgb_fg_color( p_lgrey, &s_color );
		ed2dview_draw_line( p_lgrey, p_portal->v1, p_portal->v2 );
		p_node->p_plane->length += 12.0;
		free( p_portal );

		f_dist = ( p_node->p_plane->norm[ 0 ] * v_target[ 0 ] ) + ( p_node->p_plane->norm[ 1 ] * v_target[ 1 ] ) - p_node->p_plane->length;

		if( f_dist <= 0.0 )
		{
			ed2dview_draw_clipnodes_r( p_node->p_back, v_target );
		}
		else
		{
			ed2dview_draw_clipnodes_r( p_node->p_front, v_target );
		}
	}
	else
	{
		i_idx = 0;
		printf("node: %d\n", p_node->i_nodenum );
	}
}

void ed2dview_draw_portals_from_viewsec( bspnode_t *p_node )
{
	portal_t *p_portal, *p_next;
	GdkColor s_color;

	s_color.red = 128; s_color.green = 0; s_color.blue = 256<<8;
	gdk_gc_set_rgb_fg_color( p_lgrey, &s_color );
	for( p_portal = p_node->p_portals; p_portal; p_portal = p_next )
	{
		ed2dview_draw_line( p_lgrey, p_portal->v1, p_portal->v2 );
		if( p_portal->p_front == p_node )
		{
			p_next = p_portal->p_nextf;
		}
		else
		{
			p_next = p_portal->p_nextb;
		}
	}
}


void ed2dview_calculate_path( vec2_t v_start, vec2_t v_end )
{
	csector_t *p_start, *p_end;
	GdkColor s_color;
	int i_best_connector, i_idx, i_found;
	float f_shortest;
	connector_t *p_connector_start, *p_connector_end;

	p_start = ed2dview_csector_for_origin( s_ed.p_tree, v_start )->p_sec;
	p_end = ed2dview_csector_for_origin( s_ed.p_tree, v_end )->p_sec;

	s_color.red = 255<<8; s_color.green = s_color.blue = 0;
	gdk_gc_set_rgb_fg_color( p_lgrey, &s_color );
	ed2dview_draw_vert( p_lgrey, v_start );
	ed2dview_draw_vert( p_lgrey, v_end );

	if( !p_start || !p_end )
	{
		return;
	}
	p_connector_start = &rgs_connectors[ p_start->i_connector ];
	p_connector_end = &rgs_connectors[ p_end->i_connector ];

	ed2dview_draw_vert( p_lgrey, p_connector_start->v_origin );
	ed2dview_draw_vert( p_lgrey, p_connector_end->v_origin );

	s_color.red = 200<<8; s_color.green = s_color.blue = 0;
	gdk_gc_set_rgb_fg_color( p_lgrey, &s_color );

	ed2dview_draw_line( p_lgrey, p_connector_start->v_origin, v_start );
	ed2dview_draw_line( p_lgrey, p_connector_end->v_origin, v_end );

	for( i_idx = 0; i_idx < i_num_connectors; i_idx++ )
	{
		rgs_connectors[ i_idx ].f_path_length = -1.0;
		rgs_connectors[ i_idx ].i_path_previous = 0;
		rgs_connectors[ i_idx ].i_path_handled = 0;
	}

	p_connector_start->f_path_length = 0.0;
	p_connector_start->i_path_previous = -1;
	i_found = 0;
	while( !i_found )
	{
		f_shortest = 0xffff;
		i_best_connector = -1;
		for( i_idx = 0; i_idx < i_num_connectors; i_idx++ )
		{
			if( rgs_connectors[ i_idx ].f_path_length >= 0.0 && rgs_connectors[ i_idx ].i_path_handled == 0 )
			{
				if( rgs_connectors[ i_idx ].f_path_length < f_shortest  )
				{
					i_best_connector = i_idx;
					f_shortest = rgs_connectors[ i_idx ].f_path_length;
				}
			}
		}
		if( i_best_connector == -1 )
		{
			break;
		}
		rgs_connectors[ i_best_connector ].i_path_handled = 1;
		for( i_idx = 0; i_idx < rgs_connectors[ i_best_connector ].i_num_connection; i_idx++ )
		{
			if( rgs_connectors[ rgs_connectors[ i_best_connector ].rgi_connections[ i_idx ] ].f_path_length < 0 )
			{
				rgs_connectors[ rgs_connectors[ i_best_connector ].rgi_connections[ i_idx ] ].i_path_previous = i_best_connector;
				if( rgs_connectors[ i_best_connector ].rgi_connections[ i_idx ] == p_end->i_connector )
				{
					int i_con;
					i_con = p_end->i_connector;
					while( i_con != -1 )
					{
						ed2dview_draw_line( p_lgrey, rgs_connectors[ rgs_connectors[ i_con ].i_path_previous ].v_origin, rgs_connectors[ i_con ].v_origin );
						i_con = rgs_connectors[ i_con ].i_path_previous;
					}
					i_found = 1;
				}
				else
				{
					rgs_connectors[ rgs_connectors[ i_best_connector ].rgi_connections[ i_idx ] ].f_path_length =
						rgs_connectors[ i_best_connector ].f_path_length + rgs_connectors[ i_best_connector ].rgf_connections_dist[ i_idx ];
				}
			}
		}
	}
}


void ed2dview_refresh()
{
	gchar rgui8_buf[ 0x100 ];
	float f_x, f_y;
	int i_idx, i_idx2;
	vec2_t v1, v2;
	GdkColor s_color;
	vec2_t equad[ 4 ] = { { -5, -5 }, { 5, -5 }, { 5, 5 }, { -5, 5 } };
	vec2_t connx[ 4 ] = { { -2, -2 }, { 2, 2 }, { 2, -2 }, { -2, 2 } };

	p_lgrey = gdk_gc_new( p_offscreen_buffer );

	gdk_draw_rectangle( p_offscreen_buffer, p_d2dview->style->white_gc, TRUE, 0, 0, p_d2dview->allocation.width, p_d2dview->allocation.height);

	s_color.red = s_color.green = s_color.blue = 224<<8;
	gdk_gc_set_rgb_fg_color( p_lgrey, &s_color );

	f_x = floor( s_ed.origin[ 0 ] / 16 ) * 16;
	for( ; f_x < s_ed.e2d_width + s_ed.origin[ 0 ]; f_x += 16 )
	{
		v1[ 0 ] = f_x;
		v2[ 0 ] = f_x;
		v1[ 1 ] = 0 + s_ed.origin[ 1 ];
		v2[ 1 ] = s_ed.e2d_height + s_ed.origin[ 1 ];
		ed2dview_draw_line( p_lgrey, v1, v2 );

		if( ( ( ( int ) f_x ) & 0x30 ) == 0 )
		{
			sprintf( rgui8_buf, "%d", ( int ) f_x );
			pango_layout_set_text( p_text_layout, rgui8_buf, strlen( rgui8_buf ) );
			gdk_draw_layout( p_offscreen_buffer, p_d2dview->style->black_gc, f_x - s_ed.origin[ 0 ], 10, p_text_layout );
		}
	}


	f_y = floor( s_ed.origin[ 1 ] / 16 ) * 16;
	for( ; f_y < s_ed.e2d_height + s_ed.origin[ 1 ]; f_y += 16 )
	{
		v1[ 0 ] = 0 + s_ed.origin[ 0 ];
		v2[ 0 ] = s_ed.e2d_width + s_ed.origin[ 0 ];
		v1[ 1 ] = f_y;
		v2[ 1 ] = f_y;
		ed2dview_draw_line( p_lgrey, v1, v2 );

		if( ( ( ( int ) f_y ) & 0x30 ) == 0 )
		{
			v1[ 0 ] = s_ed.origin[ 0 ];
			v2[ 0 ] = s_ed.origin[ 0 ] + 20;
			ed2dview_draw_line( p_d2dview->style->black_gc, v1, v2 );
			sprintf( rgui8_buf, "%d", ( int ) f_y );
			pango_layout_set_text( p_text_layout, rgui8_buf, strlen( rgui8_buf ) );
			gdk_draw_layout( p_offscreen_buffer, p_d2dview->style->black_gc, 10, f_y - s_ed.origin[ 1 ], p_text_layout );
		}
	}

	if( s_ed.i_which_view == 0 )
	{
		s_color.red = s_color.green = s_color.blue = 0;
		gdk_gc_set_rgb_fg_color( p_lgrey, &s_color );

		for( i_idx = 0; i_idx < s_ed.i_num_sectors; i_idx++ )
		{
			for( i_idx2 = 0; i_idx2 < s_ed.sectors[ i_idx ].i_numverts; i_idx2++ )
			{
				ed2dview_draw_line( p_lgrey, s_ed.sectors[ i_idx ].verts[ i_idx2 ], s_ed.sectors[ i_idx ].verts[ ( i_idx2 + 1 ) % s_ed.sectors[ i_idx ].i_numverts ] ); 
			}
		}

		s_color.red = 0; s_color.green = 192<<8; s_color.blue = 0;
		gdk_gc_set_rgb_fg_color( p_lgrey, &s_color );
		for( i_idx = 0; i_idx < s_ed.i_num_entities; i_idx++ )
		{
			vec2_t eorig[ 4 ];

			for( i_idx2 = 0; i_idx2 < 4; i_idx2++ )
			{
				eorig[ i_idx2 ][ 0 ] = equad[ i_idx2 ][ 0 ] + s_ed.entities[ i_idx ].origin[ 0 ];
				eorig[ i_idx2 ][ 1 ] = equad[ i_idx2 ][ 1 ] + s_ed.entities[ i_idx ].origin[ 1 ];
			}
			for( i_idx2 = 0; i_idx2 < 4; i_idx2++ )
			{
				ed2dview_draw_line( p_lgrey, eorig[ i_idx2 ], eorig[ ( i_idx2 + 1 ) % 4 ] ); 
			}
		}

		if( s_ed.i_selected_sector >= 0 )
		{
			s_color.red = 255<<8; s_color.green = s_color.blue = 0;
			gdk_gc_set_rgb_fg_color( p_lgrey, &s_color );
			for( i_idx2 = 0; i_idx2 < s_ed.sectors[ s_ed.i_selected_sector ].i_numverts; i_idx2++ )
			{
				ed2dview_draw_line( p_lgrey, s_ed.sectors[ s_ed.i_selected_sector ].verts[ i_idx2 ], s_ed.sectors[ s_ed.i_selected_sector ].verts[ ( i_idx2 + 1 ) % s_ed.sectors[ s_ed.i_selected_sector ].i_numverts ] ); 
			}
			s_color.red = 0; s_color.green = 128<<8; s_color.blue = 0;
			gdk_gc_set_rgb_fg_color( p_lgrey, &s_color );
			for( i_idx2 = 0; i_idx2 < s_ed.sectors[ s_ed.i_selected_sector ].i_numverts; i_idx2++ )
			{
				ed2dview_draw_vert( p_lgrey, s_ed.sectors[ s_ed.i_selected_sector ].verts[ i_idx2 ] ); 
			}
			if( s_ed.i_selected_line >= 0 )
			{
				s_color.red = s_color.green = 0; s_color.blue = 255<<8;
				gdk_gc_set_rgb_fg_color( p_lgrey, &s_color );
				ed2dview_draw_line( p_lgrey, s_ed.sectors[ s_ed.i_selected_sector ].verts[ s_ed.i_selected_line ], s_ed.sectors[ s_ed.i_selected_sector ].verts[ ( s_ed.i_selected_line + 1 ) % s_ed.sectors[ s_ed.i_selected_sector ].i_numverts ] ); 
			}
			if( s_ed.i_selected_vert >= 0 )
			{
				s_color.red = 0; s_color.green = 255<<8; s_color.blue = 0;
				gdk_gc_set_rgb_fg_color( p_lgrey, &s_color );
				ed2dview_draw_vert( p_lgrey, s_ed.sectors[ s_ed.i_selected_sector ].verts[ s_ed.i_selected_vert ] ); 
			}
		}
		else if( s_ed.i_selected_entity >= 0 )
		{
			vec2_t eorig[ 4 ];
			s_color.red = 0; s_color.green = 255<<8; s_color.blue = 128<<8;
			gdk_gc_set_rgb_fg_color( p_lgrey, &s_color );

			for( i_idx2 = 0; i_idx2 < 4; i_idx2++ )
			{
				eorig[ i_idx2 ][ 0 ] = equad[ i_idx2 ][ 0 ] + s_ed.entities[ s_ed.i_selected_entity ].origin[ 0 ];
				eorig[ i_idx2 ][ 1 ] = equad[ i_idx2 ][ 1 ] + s_ed.entities[ s_ed.i_selected_entity ].origin[ 1 ];
			}
			for( i_idx2 = 0; i_idx2 < 4; i_idx2++ )
			{
				ed2dview_draw_line( p_lgrey, eorig[ i_idx2 ], eorig[ ( i_idx2 + 1 ) % 4 ] ); 
			}
		}
	}
	else
	{
		bspnode_t *p_node;
		p_node = ed2dview_csector_for_origin( s_ed.p_tree, s_ed.vorigin );
		if( p_node )
		{
			printf("view node: %d\n", p_node->p_sec ? p_node->p_sec->i_secnum : -1 );
		}
		ed2dview_draw_tree_r( s_ed.p_tree, p_node );

		//ed2dview_draw_portals_from_viewsec( p_node );

		if( s_ed.i_draw_path_queued )
		{
			ed2dview_calculate_path( s_ed.test_path_start, s_ed.test_path_end );

			s_ed.i_draw_path_queued = 0;
		}

		ed2dview_draw_clipnodes_r( s_ed.p_cliptree, s_ed.vorigin );

		{
			vec2_t v1, v2;

			v1[ 0 ] = 412.18;
			v1[ 1 ] = -72.125;
			v2[ 0 ] = v1[ 0 ] - 5.812;
			v2[ 1 ] = v1[ 1 ] + 2.625;
			s_color.red = 128<<8; s_color.green = 255<<8; s_color.blue = 0<<8;
			gdk_gc_set_rgb_fg_color( p_lgrey, &s_color );

			ed2dview_draw_line( p_lgrey, v1, v2 );

		}

#if 0
		for( i_idx = 0; i_idx < i_num_connectors; i_idx++ )
		{
			vec2_t connx_[ 4 ];
			s_color.red = 255<<8; s_color.green = 0; s_color.blue = 255<<8;
			gdk_gc_set_rgb_fg_color( p_lgrey, &s_color );

			for( i_idx2 = 0; i_idx2 < 4; i_idx2++ )
			{
				connx_[ i_idx2 ][ 0 ] = rgs_connectors[ i_idx ].v_origin[ 0 ] + connx[ i_idx2 ][ 0 ];
				connx_[ i_idx2 ][ 1 ] = rgs_connectors[ i_idx ].v_origin[ 1 ] + connx[ i_idx2 ][ 1 ];
			}
			for( i_idx2 = 0; i_idx2 < 4; i_idx2+=2 )
			{
				ed2dview_draw_line( p_lgrey, connx_[ i_idx2 ], connx_[ ( i_idx2 + 1 ) ] ); 
			}
			for( i_idx2 = 0; i_idx2 < rgs_connectors[ i_idx ].i_num_connection; i_idx2++ )
			{
				ed2dview_draw_line( p_lgrey, rgs_connectors[ i_idx ].v_origin, rgs_connectors[ rgs_connectors[ i_idx ].rgi_connections[ i_idx2 ] ].v_origin );
			}
			{
				sprintf( rgui8_buf, "%d", ( int ) rgs_connectors[ i_idx ].f_radius );
				pango_layout_set_text( p_text_layout, rgui8_buf, strlen( rgui8_buf ) );
				gdk_draw_layout( p_offscreen_buffer, p_d2dview->style->black_gc, rgs_connectors[ i_idx ].v_origin[ 0 ] - s_ed.origin[ 0 ], rgs_connectors[ i_idx ].v_origin[ 1 ] - s_ed.origin[ 1 ], p_text_layout );
			}

		}
#endif
	}

	if( s_ed.i_grab_keyboard )
	{
		s_color.red = s_color.green = 255 << 8; s_color.blue = 0;
		gdk_gc_set_rgb_fg_color( p_lgrey, &s_color );

		gdk_draw_line( p_offscreen_buffer, p_lgrey, 5, 5, s_ed.e2d_width - 5, 5 );
		gdk_draw_line( p_offscreen_buffer, p_lgrey, 5, 6, s_ed.e2d_width - 5, 6 );

		gdk_draw_line( p_offscreen_buffer, p_lgrey, s_ed.e2d_width - 5, 5, s_ed.e2d_width - 5, s_ed.e2d_height - 5 );
		gdk_draw_line( p_offscreen_buffer, p_lgrey, s_ed.e2d_width - 6, 5, s_ed.e2d_width - 6, s_ed.e2d_height - 5 );

		gdk_draw_line( p_offscreen_buffer, p_lgrey, s_ed.e2d_width - 5, s_ed.e2d_height - 5, 5, s_ed.e2d_height - 5 );
		gdk_draw_line( p_offscreen_buffer, p_lgrey, s_ed.e2d_width - 5, s_ed.e2d_height - 6, 5, s_ed.e2d_height - 6 );

		gdk_draw_line( p_offscreen_buffer, p_lgrey, 5, s_ed.e2d_height - 5, 5, 5 );
		gdk_draw_line( p_offscreen_buffer, p_lgrey, 6, s_ed.e2d_height - 5, 6, 5 );
	}

	gdk_gc_destroy( p_lgrey );

	gdk_draw_pixmap( p_d2dview->window, p_d2dview->style->fg_gc[GTK_WIDGET_STATE (p_d2dview)], p_offscreen_buffer,
		0, 0, 0, 0, s_ed.e2d_width, s_ed.e2d_height );
}


void panel_refresh( )
{
	if( s_ed.i_selected_sector >= 0 )
	{
		gtk_adjustment_set_value( adjustment_ztop, ( gdouble ) s_ed.sectors[ s_ed.i_selected_sector ].z[ 1 ] );
		gtk_adjustment_set_value( adjustment_zbot, ( gdouble ) s_ed.sectors[ s_ed.i_selected_sector ].z[ 0 ] );
		gtk_entry_set_text( p_entry_sector_floor, ( const gchar *) s_ed.sectors[ s_ed.i_selected_sector ].rgui8_texture_floor );
		gtk_entry_set_text( p_entry_sector_ceil, ( const gchar *) s_ed.sectors[ s_ed.i_selected_sector ].rgui8_texture_ceiling );
		if( s_ed.i_selected_line >= 0 )
		{
			gtk_entry_set_text( p_entry_linetex_top, s_ed.sectors[ s_ed.i_selected_sector ].lines[ s_ed.i_selected_line ].rgi8_texture_upper );
			gtk_entry_set_text( p_entry_linetex_bottom, s_ed.sectors[ s_ed.i_selected_sector ].lines[ s_ed.i_selected_line ].rgi8_texture_lower );
		}
		else
		{
			gtk_entry_set_text( p_entry_linetex_top, "-" );
			gtk_entry_set_text( p_entry_linetex_bottom, "-" );
		}
		gtk_entry_set_text( p_entry_entity_class, "-" );
	}
	else
	{
		gtk_adjustment_set_value( adjustment_ztop, ( gdouble ) -1 );
		gtk_adjustment_set_value( adjustment_zbot, ( gdouble ) -1 );
 		gtk_entry_set_text( p_entry_sector_floor, "-" );
		gtk_entry_set_text( p_entry_sector_ceil, "-" );
		gtk_entry_set_text( p_entry_linetex_top, "-" );
		gtk_entry_set_text( p_entry_linetex_bottom, "-" );

		if( s_ed.i_selected_entity >= 0 )
		{
			gtk_entry_set_text( p_entry_entity_class, s_ed.entities[ s_ed.i_selected_entity ].rgi8_class_name );
		}
		else
		{
			gtk_entry_set_text( p_entry_entity_class, "-" );
		}
	}


}

G_MODULE_EXPORT void on_imagemenuitem1_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	pui8_current_filename = NULL;
	s_ed.i_num_sectors = 0;
	s_ed.i_num_entities = 0;
	ed2dview_refresh();
}

void load_map( char *fname );
G_MODULE_EXPORT void on_imagemenuitem2_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	GtkWidget *dialog;
	dialog = gtk_file_chooser_dialog_new ("Open Map", GTK_WINDOW( window ),
				      GTK_FILE_CHOOSER_ACTION_OPEN,
				      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				      GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
				      NULL);
	if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
	{
		char *filename;
		filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
		load_map(filename);
		pui8_current_filename = filename;
	}
	gtk_widget_destroy (dialog);
	ed2dview_refresh();
}

void export_map( char *fname, int i_le );
G_MODULE_EXPORT void on_imagemenuitem3_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	GtkWidget *dialog;
	dialog = gtk_file_chooser_dialog_new ("Export Compiled Map",
				      GTK_WINDOW( window ),
				      GTK_FILE_CHOOSER_ACTION_SAVE,
				      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				      GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
				      NULL);
	gtk_file_chooser_set_do_overwrite_confirmation (GTK_FILE_CHOOSER (dialog), TRUE);

	gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (dialog), "../datastore/" );
	gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER (dialog), "mapXX.dat");

	if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
	{
		char *filename;
		filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
		export_map (filename, 0 );
		/* fixme: memleak */
	}
	gtk_widget_destroy (dialog);
}

G_MODULE_EXPORT void on_imagemenuitem12_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	GtkWidget *dialog;
	dialog = gtk_file_chooser_dialog_new ("Export Compiled Map Little Endian",
				      GTK_WINDOW( window ),
				      GTK_FILE_CHOOSER_ACTION_SAVE,
				      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				      GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
				      NULL);
	gtk_file_chooser_set_do_overwrite_confirmation (GTK_FILE_CHOOSER (dialog), TRUE);

	gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (dialog), "../datastore/" );
	gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER (dialog), "mapXX.dat");

	if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
	{
		char *filename;
		filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
		export_map (filename, 1);
		/* fixme: memleak */
	}
	gtk_widget_destroy (dialog);
}

void save_map( char *fname );
G_MODULE_EXPORT void on_imagemenuitem4_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	GtkWidget *dialog;
	dialog = gtk_file_chooser_dialog_new ("Save Map As",
				      GTK_WINDOW( window ),
				      GTK_FILE_CHOOSER_ACTION_SAVE,
				      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				      GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
				      NULL);
	gtk_file_chooser_set_do_overwrite_confirmation (GTK_FILE_CHOOSER (dialog), TRUE);
	if ( pui8_current_filename == NULL )
	{
		gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (dialog), "./" );
		gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER (dialog), "Untitled Map");
	}
	else
		gtk_file_chooser_set_filename (GTK_FILE_CHOOSER (dialog), pui8_current_filename );

	if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
	{
		char *filename;
		filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
		save_map (filename);
		pui8_current_filename = filename;
	}
	gtk_widget_destroy (dialog);
}


void map_compile();
G_MODULE_EXPORT void on_build_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	map_compile();
}

G_MODULE_EXPORT void view_editor_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	s_ed.i_which_view = 0;
}

G_MODULE_EXPORT void view_tree_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	s_ed.i_which_view = 1;
}


void select_contents( int x, int y, int cycle );
G_MODULE_EXPORT gint on_ed2dview_button_press_event(GtkWidget *widget, GdkEventButton *event)
{
	if (event->button == 1 && s_ed.i_which_view == 0 )
	{
		/* select ray */
		s_ed.i_selected_line = -1;
		s_ed.i_selected_vert = -1;
		select_contents( event->x, event->y, 0 );
		panel_refresh();
		ed2dview_refresh();
	}
	else if (event->button == 3 && s_ed.i_which_view == 0 )
	{
		/* select ray cycle sectors */
		s_ed.i_selected_line = -1;
		s_ed.i_selected_vert = -1;
		select_contents( event->x, event->y, 1 );
		panel_refresh();
		ed2dview_refresh();
	}
	else if( event->button == 3 )
	{
		s_ed.vorigin[ 0 ] = event->x + s_ed.origin[ 0 ];
		s_ed.vorigin[ 1 ] = event->y + s_ed.origin[ 1 ];
		ed2dview_refresh();
	}
	else if( event->button == 1 && s_ed.i_which_view == 1 )
	{
		s_ed.test_path_start[ 0 ] = s_ed.test_path_end[ 0 ];
		s_ed.test_path_start[ 1 ] = s_ed.test_path_end[ 1 ];
		s_ed.test_path_end[ 0 ] = event->x + s_ed.origin[ 0 ];
		s_ed.test_path_end[ 1 ] = event->y + s_ed.origin[ 1 ];
		ed2dview_refresh();
	}

	return TRUE;
}

G_MODULE_EXPORT gint on_ed2dview_motion_notify_event(GtkWidget *widget, GdkEventMotion *event)
{
	int x, y;
	GdkModifierType state;

	if (event->is_hint)
		gdk_window_get_pointer (event->window, &x, &y, &state);
	else
    {
		x = event->x;
		y = event->y;
		state = event->state;
    }

	//printf("motion %d %d %d\n", x, y, state );
    
	if (state & GDK_BUTTON1_MASK )
	{
		if( s_ed.i_selected_sector >= 0 && s_ed.i_selected_vert >= 0 )
		{
			s_ed.sectors[ s_ed.i_selected_sector ].verts[ s_ed.i_selected_vert ][ 0 ] -= s_ed.e2d_mx - x;
			s_ed.sectors[ s_ed.i_selected_sector ].verts[ s_ed.i_selected_vert ][ 1 ] -= s_ed.e2d_my - y;
		}
		else if( s_ed.i_selected_entity >= 0 )
		{
			s_ed.entities[ s_ed.i_selected_entity ].origin[ 0 ] -= s_ed.e2d_mx - x;
			s_ed.entities[ s_ed.i_selected_entity ].origin[ 1 ] -= s_ed.e2d_my - y;
		}
		ed2dview_refresh();
		panel_refresh();
	}
	else if( state & GDK_BUTTON2_MASK )  
	{
		s_ed.origin[ 0 ] += s_ed.e2d_mx - x;
		s_ed.origin[ 1 ] += s_ed.e2d_my - y;
		ed2dview_refresh();
	}

	s_ed.e2d_mx = x;
	s_ed.e2d_my = y;

	return TRUE;
}


/* Create a new backing pixmap of the appropriate size */
G_MODULE_EXPORT gint ed2dview_configure_event_cb (GtkWidget *widget, GdkEventConfigure *event)
{
	if (p_offscreen_buffer)
	{
		gdk_pixmap_unref(p_offscreen_buffer);
	}
	if( p_text_layout )
	{
		g_object_unref( p_text_layout );
	}

	p_offscreen_buffer = gdk_pixmap_new( widget->window, widget->allocation.width, widget->allocation.height, -1);

	p_text_layout = gtk_widget_create_pango_layout( widget, NULL );

	s_ed.e2d_width = widget->allocation.width;
	s_ed.e2d_height = widget->allocation.height;
	p_d2dview = widget;

	ed2dview_refresh();

	return TRUE;
}

G_MODULE_EXPORT gint on_ed2dview_expose_event( GtkWidget *widget, GdkEventExpose *event, void *p_user )
{
	gdk_draw_pixmap( widget->window, widget->style->fg_gc[GTK_WIDGET_STATE (widget)], p_offscreen_buffer,
		event->area.x, event->area.y, event->area.x, event->area.y, event->area.width, event->area.height );
	return TRUE;
}


void ed_new_sector( );
void ed_delete_sector( );
void ed_new_entity( );
void ed_delete_entity( );
int snap_selected_vert( );
void delete_selvert();
void split_selline();

G_MODULE_EXPORT gboolean ed_key_pressed_event(GtkWidget *widget, GdkEvent  *event, gpointer user_data)
{
	printf("0x%x\n", event->key.keyval );

	if( event->key.keyval == ' ' )
	{
		s_ed.i_grab_keyboard = !s_ed.i_grab_keyboard;
		ed2dview_refresh();
		return TRUE;
	}

	/* esc */
	if( event->key.keyval == 0xff1b && s_ed.i_selected_sector >= 0 )
	{
		s_ed.i_selected_sector = -1;
		ed2dview_refresh();
		panel_refresh();
		return TRUE;
	}

	if( !s_ed.i_grab_keyboard )
	{
		return FALSE;
	}

	if( event->key.keyval == 0xff1b && s_ed.i_selected_sector >= 0 )
	{
		s_ed.i_selected_sector = -1;
		ed2dview_refresh();
		panel_refresh();
		return TRUE;
	}
	/* del */
	if( event->key.keyval == 0xffff && s_ed.i_selected_sector >= 0 && s_ed.i_selected_vert >= 0 )
	{
		delete_selvert();
		ed2dview_refresh();
		panel_refresh();
		return TRUE;
	}
	/* insert */
	if( event->key.keyval == 0xff63 && s_ed.i_selected_sector >= 0 && s_ed.i_selected_line >= 0 )
	{
		split_selline();
		ed2dview_refresh();
		panel_refresh();
		return TRUE;
	}
	if( event->key.keyval == 'n' )
	{
		ed_new_sector();
		ed2dview_refresh();
		panel_refresh();
		return TRUE;
	}
	if( event->key.keyval == 'e' )
	{
		ed_new_entity();
		ed2dview_refresh();
		panel_refresh();
		return TRUE;
	}
	if( event->key.keyval == 'r' && s_ed.i_selected_entity >= 0)
	{
		ed_delete_entity();
		ed2dview_refresh();
		panel_refresh();
		return TRUE;
	}
	if( event->key.keyval == 'd' && s_ed.i_selected_sector >= 0 )
	{
		ed_delete_sector();
		ed2dview_refresh();
		panel_refresh();
		return TRUE;
	}
	if( event->key.keyval == 's' )
	{
		if( snap_selected_vert( ) )
		{
			ed2dview_refresh();
			panel_refresh();
			return TRUE;
		}
		return FALSE;
	}
	if( event->key.keyval == 0x31 && s_ed.i_which_view == 1 )
	{
		s_ed.i_draw_path_queued = 1;
		ed2dview_refresh();
	}
	return FALSE;
}

G_MODULE_EXPORT void snap_change_value_cb(GtkAdjustment *adjustment, gpointer user_data)
{
	gdouble d_nsnap;
	d_nsnap = gtk_adjustment_get_value( adjustment );

	if( d_nsnap < s_ed.i_snap )
	{
		s_ed.i_snap = MAX( s_ed.i_snap >> 1, 1 );
	}
	else if( d_nsnap > s_ed.i_snap )
	{
		s_ed.i_snap = MIN( s_ed.i_snap << 1, 32 );
	}
	gtk_adjustment_set_value( adjustment, ( gdouble )s_ed.i_snap );
}

G_MODULE_EXPORT void on_ztop_change_value(GtkAdjustment *adjustment, gpointer user_data)
{
	gdouble z_top;
	z_top = gtk_adjustment_get_value( adjustment );
	printf("ztop %f %f ( %f ) %d\n", s_ed.sectors[ s_ed.i_selected_sector ].z[ 0 ], s_ed.sectors[ s_ed.i_selected_sector ].z[ 1 ], z_top, s_ed.i_selected_sector );
	if( s_ed.i_selected_sector >= 0 )
	{
		z_top = MIN( 512, MAX( s_ed.sectors[ s_ed.i_selected_sector ].z[ 0 ] + 1 , z_top ) );
		s_ed.sectors[ s_ed.i_selected_sector ].z[ 1 ] = z_top;
	}
	gtk_adjustment_set_value( adjustment, ( gdouble )z_top );
}

G_MODULE_EXPORT void on_zbot_change_value(GtkAdjustment *adjustment, gpointer user_data)
{
	gdouble z_bot;
	z_bot = gtk_adjustment_get_value( adjustment );

	if( s_ed.i_selected_sector >= 0 )
	{
		z_bot = MAX( -512, MIN( s_ed.sectors[ s_ed.i_selected_sector ].z[ 1 ] - 1 , z_bot ) );
		s_ed.sectors[ s_ed.i_selected_sector ].z[ 0 ] = z_bot;
	}
	gtk_adjustment_set_value( adjustment, ( gdouble )z_bot );
}

G_MODULE_EXPORT void on_textop_changed(GtkEditable *editable, gpointer user_data)
{
	if( s_ed.i_selected_sector >= 0 )
	{
		const gchar *p_text;

		p_text = gtk_entry_get_text( p_entry_sector_ceil );
		strcpy( s_ed.sectors[ s_ed.i_selected_sector ].rgui8_texture_ceiling, p_text );
	}
}

G_MODULE_EXPORT void on_texbottom_changed(GtkEditable *editable, gpointer user_data)
{
	if( s_ed.i_selected_sector >= 0 )
	{
		const gchar *p_text;

		p_text = gtk_entry_get_text( p_entry_sector_floor );
		strcpy( s_ed.sectors[ s_ed.i_selected_sector ].rgui8_texture_floor, p_text );
	}
}

G_MODULE_EXPORT void on_texup_changed(GtkEditable *editable, gpointer user_data)
{
	if( s_ed.i_selected_sector >= 0 && s_ed.i_selected_line >= 0 )
	{
		const gchar *p_text;

		p_text = gtk_entry_get_text( p_entry_linetex_top );
		strcpy( s_ed.sectors[ s_ed.i_selected_sector ].lines[ s_ed.i_selected_line ].rgi8_texture_upper, p_text );
	}
}

G_MODULE_EXPORT void on_texlow_changed(GtkEditable *editable, gpointer user_data)
{
	if( s_ed.i_selected_sector >= 0 && s_ed.i_selected_line >= 0 )
	{
		const gchar *p_text;

		p_text = gtk_entry_get_text( p_entry_linetex_bottom );
		strcpy( s_ed.sectors[ s_ed.i_selected_sector ].lines[ s_ed.i_selected_line ].rgi8_texture_lower, p_text );
	}
}

G_MODULE_EXPORT void on_entityclass_changed(GtkEditable *editable, gpointer user_data)
{
	if( s_ed.i_selected_entity >= 0 )
	{
		const gchar *p_text;

		p_text = gtk_entry_get_text( p_entry_entity_class );
		strcpy( s_ed.entities[ s_ed.i_selected_entity ].rgi8_class_name, p_text );
	}
}


void select_contents( int x, int y, int cycle )
{
	int i_idx, i_idx2, i_sel_line;
	float f_scale, f_dist2, f_distsel, ndist;
	vec2_t sel, v1, v2, delt, delt2, norm;

	sel[ 0 ] = x + s_ed.origin[ 0 ];
	sel[ 1 ] = y + s_ed.origin[ 1 ];

	if( s_ed.i_selected_sector >= 0 && cycle == 0 )
	{
		for( i_idx = 0; i_idx < s_ed.sectors[ s_ed.i_selected_sector ].i_numverts; i_idx++ )
		{
			v1[ 0 ] = s_ed.sectors[ s_ed.i_selected_sector ].verts[ i_idx ][ 0 ];
			v1[ 1 ] = s_ed.sectors[ s_ed.i_selected_sector ].verts[ i_idx ][ 1 ];
			v2[ 0 ] = s_ed.sectors[ s_ed.i_selected_sector ].verts[ ( i_idx + 1 ) % s_ed.sectors[ s_ed.i_selected_sector ].i_numverts ][ 0 ];
			v2[ 1 ] = s_ed.sectors[ s_ed.i_selected_sector ].verts[ ( i_idx + 1 ) % s_ed.sectors[ s_ed.i_selected_sector ].i_numverts ][ 1 ];

			if( fabs( sel[ 0 ] - v1[ 0 ] ) <= 2.0 && fabs( sel[ 1 ] - v1[ 1 ] ) <= 2.0 )
			{
				s_ed.i_selected_vert = i_idx;
				s_ed.i_selected_line = s_ed.i_selected_vert;
				return;
			}
			if( fabs( sel[ 0 ] - v2[ 0 ] ) <= 2.0 && fabs( sel[ 1 ] - v2[ 1 ] ) <= 2.0 )
			{
				s_ed.i_selected_vert = ( i_idx + 1 ) % s_ed.sectors[ s_ed.i_selected_sector ].i_numverts;
				s_ed.i_selected_line = s_ed.i_selected_vert;
				return;
			}

			delt[ 0 ] = v2[ 0 ] - v1[ 0 ];
			delt[ 1 ] = v2[ 1 ] - v1[ 1 ];
			delt2[ 0 ] = sel[ 0 ] - v1[ 0 ];
			delt2[ 1 ] = sel[ 1 ] - v1[ 1 ];

			norm[ 1 ] = delt[ 0 ];
			norm[ 0 ] = -delt[ 1 ];
			f_scale = 1.0 / sqrt( ( norm[ 0 ] * norm[ 0 ] ) + ( norm[ 1 ] * norm[ 1 ] ) );
			norm[ 0 ] = norm[ 0 ] * f_scale;
			norm[ 1 ] = norm[ 1 ] * f_scale;

			ndist = ( v1[ 0 ] * norm[ 0 ] ) + ( v1[ 1 ] * norm[ 1 ] );

			if( fabs( ( ( norm[ 0 ] * sel[ 0 ] ) + ( norm[ 1 ] * sel[ 1 ] ) - ndist ) ) <= 2.0 &&
				sqrt( delt[ 0 ] * delt[ 0 ] + delt[ 1 ] * delt[ 1 ] ) > sqrt( delt2[ 0 ] * delt2[ 0 ] + delt2[ 1 ] * delt2[ 1 ] ) )
			{
				s_ed.i_selected_vert = -1;
				s_ed.i_selected_line = i_idx;
				return;
			}
		}
	}
	else
	{
		int cycled, scount;
		for( s_ed.i_selected_entity = 0; s_ed.i_selected_entity < s_ed.i_num_entities; s_ed.i_selected_entity++ )
		{
			if( fabs( s_ed.entities[ s_ed.i_selected_entity ].origin[ 0 ] - sel[ 0 ] ) < 5 && fabs( s_ed.entities[ s_ed.i_selected_entity ].origin[ 1 ] - sel[ 1 ] ) < 5 )
			{
				return;
			}
		}
		s_ed.i_selected_entity = -1;

		if( cycle == 0 )
		{
			s_ed.i_selected_sector = 0;
		}
		else
		{
			s_ed.i_selected_sector = ( ( s_ed.i_selected_sector + 1 ) % s_ed.i_num_sectors );
		}
		for( scount = 0; scount < s_ed.i_num_sectors; s_ed.i_selected_sector = ( ( s_ed.i_selected_sector + 1 ) % s_ed.i_num_sectors ), scount++ )
		{
			for( i_idx = 0; i_idx < s_ed.sectors[ s_ed.i_selected_sector ].i_numverts; i_idx++ )
			{
				v1[ 0 ] = s_ed.sectors[ s_ed.i_selected_sector ].verts[ i_idx ][ 0 ];
				v1[ 1 ] = s_ed.sectors[ s_ed.i_selected_sector ].verts[ i_idx ][ 1 ];
				v2[ 0 ] = s_ed.sectors[ s_ed.i_selected_sector ].verts[ ( i_idx + 1 ) % s_ed.sectors[ s_ed.i_selected_sector ].i_numverts ][ 0 ];
				v2[ 1 ] = s_ed.sectors[ s_ed.i_selected_sector ].verts[ ( i_idx + 1 ) % s_ed.sectors[ s_ed.i_selected_sector ].i_numverts ][ 1 ];

				if( fabs( sel[ 0 ] - v1[ 0 ] ) <= 2.0 && fabs( sel[ 1 ] - v1[ 1 ] ) <= 2.0 )
				{
					return;
				}
				if( fabs( sel[ 0 ] - v2[ 0 ] ) <= 2.0 && fabs( sel[ 1 ] - v2[ 1 ] ) <= 2.0 )
				{
					return;
				}

				delt[ 0 ] = v2[ 0 ] - v1[ 0 ];
				delt[ 1 ] = v2[ 1 ] - v1[ 1 ];
				delt2[ 0 ] = sel[ 0 ] - v1[ 0 ];
				delt2[ 1 ] = sel[ 1 ] - v1[ 1 ];

				norm[ 1 ] = delt[ 0 ];
				norm[ 0 ] = -delt[ 1 ];
				f_scale = 1.0 / sqrt( ( norm[ 0 ] * norm[ 0 ] ) + ( norm[ 1 ] * norm[ 1 ] ) );
				norm[ 0 ] = norm[ 0 ] * f_scale;
				norm[ 1 ] = norm[ 1 ] * f_scale;

				ndist = ( v1[ 0 ] * norm[ 0 ] ) + ( v1[ 1 ] * norm[ 1 ] );

				if( fabs( ( ( norm[ 0 ] * sel[ 0 ] ) + ( norm[ 1 ] * sel[ 1 ] ) - ndist ) ) <= 2.0 ) /* on plane */
				{
					float dist1, dist2;

					ndist = sqrt( delt[ 0 ] * delt[ 0 ] + delt[ 1 ] * delt[ 1 ] );
					norm[ 0 ] = delt[ 0 ] / ndist;
					norm[ 1 ] = delt[ 1 ] / ndist;

					ndist = v1[ 0 ] * norm[ 0 ] + v1[ 1 ] * norm[ 1 ];
					dist1 = norm[ 0 ] * sel[ 0 ] + norm[ 1 ] * sel[ 1 ] - ndist;
					if( dist1 >= -2.0 )
					{
						ndist = v2[ 0 ] * norm[ 0 ] + v2[ 1 ] * norm[ 1 ];
						dist2 = norm[ 0 ] * sel[ 0 ] + norm[ 1 ] * sel[ 1 ] - ndist;
						if( dist2 <= 2.0 )
						{
							return;
						}
					}
				}
			}
		}
		s_ed.i_selected_sector = -1;
		s_ed.i_selected_vert = -1;
		s_ed.i_selected_line = -1;
	}
}

void snap_vec2( vec2_t v )
{
	v[ 0 ] = floor( ( v[ 0 ] + ( ( float )s_ed.i_snap ) / 2.0 ) / s_ed.i_snap ) * s_ed.i_snap;
	v[ 1 ] = floor( ( v[ 1 ] + ( ( float )s_ed.i_snap ) / 2.0 ) / s_ed.i_snap ) * s_ed.i_snap;
}

int snap_selected_vert( )
{
	if( s_ed.i_selected_sector >= 0 && s_ed.i_selected_vert >= 0 )
	{
		snap_vec2( s_ed.sectors[ s_ed.i_selected_sector ].verts[ s_ed.i_selected_vert ] );
		return TRUE;
	}
	return FALSE;
}

void ed_new_sector( )
{
	int i_idx;
	sector_t *ps_sector;
	vec2_t origin;

	origin[ 0 ] = s_ed.origin[ 0 ] + s_ed.e2d_width / 2;
	origin[ 1 ] = s_ed.origin[ 1 ] + s_ed.e2d_height / 2;
	snap_vec2( origin );

	ps_sector = &s_ed.sectors[ s_ed.i_num_sectors++ ];
	ps_sector->i_numverts = 4;
	ps_sector->verts[ 0 ][ 0 ] = origin[ 0 ] - 32;
	ps_sector->verts[ 0 ][ 1 ] = origin[ 1 ] - 32;
	ps_sector->verts[ 1 ][ 0 ] = origin[ 0 ] + 32;
	ps_sector->verts[ 1 ][ 1 ] = origin[ 1 ] - 32;
	ps_sector->verts[ 2 ][ 0 ] = origin[ 0 ] + 32;
	ps_sector->verts[ 2 ][ 1 ] = origin[ 1 ] + 32;
	ps_sector->verts[ 3 ][ 0 ] = origin[ 0 ] - 32;
	ps_sector->verts[ 3 ][ 1 ] = origin[ 1 ] + 32;
	ps_sector->z[ 0 ] = 0;
	ps_sector->z[ 1 ] = 128;

	for( i_idx = 0; i_idx < 4; i_idx++ )
	{
		sprintf( &ps_sector->lines[ i_idx ].rgi8_texture_lower[ 0 ], "unknown" );
		sprintf( &ps_sector->lines[ i_idx ].rgi8_texture_upper[ 0 ], "unknown" );
	}
	sprintf( &ps_sector->rgui8_texture_floor[ 0 ], "unknown" );
	sprintf( &ps_sector->rgui8_texture_ceiling[ 0 ], "unknown" );

	if( s_ed.i_selected_sector >= 0 )
	{
		strcpy( &ps_sector->rgui8_texture_floor[ 0 ], s_ed.sectors[ s_ed.i_selected_sector ].rgui8_texture_floor );
		strcpy( &ps_sector->rgui8_texture_ceiling[ 0 ], s_ed.sectors[ s_ed.i_selected_sector ].rgui8_texture_ceiling );
		for( i_idx = 0; i_idx < 4; i_idx++ )
		{
			strcpy( &ps_sector->lines[ i_idx ].rgi8_texture_lower[ 0 ], s_ed.sectors[ s_ed.i_selected_sector ].lines[ 0 ].rgi8_texture_lower );
			strcpy( &ps_sector->lines[ i_idx ].rgi8_texture_upper[ 0 ], s_ed.sectors[ s_ed.i_selected_sector ].lines[ 0 ].rgi8_texture_upper );
		}
		ps_sector->z[ 0 ] = s_ed.sectors[ s_ed.i_selected_sector ].z[ 0 ];
		ps_sector->z[ 1 ] = s_ed.sectors[ s_ed.i_selected_sector ].z[ 1 ];
	}
}

void ed_delete_sector( )
{
	int i_idx;

	s_ed.i_num_sectors--;
	memmove( &s_ed.sectors[ s_ed.i_selected_sector ], &s_ed.sectors[ s_ed.i_selected_sector + 1 ], sizeof( sector_t ) * s_ed.i_num_sectors - s_ed.i_selected_sector );
	s_ed.i_selected_sector = -1;
}

void ed_new_entity( )
{
	entity_t *ps_ent;

	ps_ent = &s_ed.entities[ s_ed.i_num_entities++ ];
	ps_ent->origin[ 0 ] = s_ed.origin[ 0 ] + s_ed.e2d_width / 2;
	ps_ent->origin[ 1 ] = s_ed.origin[ 1 ] + s_ed.e2d_height / 2;
	snap_vec2( ps_ent->origin );

	sprintf( &ps_ent->rgi8_class_name[ 0 ], "unknown_entity" );
}


void ed_delete_entity( )
{
	s_ed.i_num_entities--;
	memmove( &s_ed.entities[ s_ed.i_selected_entity ], &s_ed.entities[ s_ed.i_selected_entity + 1 ], sizeof( entity_t ) * ( s_ed.i_num_entities - s_ed.i_selected_entity ) );
	s_ed.i_selected_entity = -1;
}


void split_selline( )
{
	int i_idx;
	vec2_t v;

	memmove( &s_ed.sectors[ s_ed.i_selected_sector ].verts[ s_ed.i_selected_line + 2 ], &s_ed.sectors[ s_ed.i_selected_sector ].verts[ s_ed.i_selected_line + 1 ],
		sizeof( vec2_t ) * s_ed.sectors[ s_ed.i_selected_sector ].i_numverts - ( s_ed.i_selected_vert + 1 ) );
	memmove( &s_ed.sectors[ s_ed.i_selected_sector ].lines[ s_ed.i_selected_line + 2 ], &s_ed.sectors[ s_ed.i_selected_sector ].lines[ s_ed.i_selected_line + 1 ],
		sizeof( line_t ) * s_ed.sectors[ s_ed.i_selected_sector ].i_numverts - ( s_ed.i_selected_vert + 1 ) );

	s_ed.sectors[ s_ed.i_selected_sector ].verts[ s_ed.i_selected_line + 1 ][ 0 ] = ( s_ed.sectors[ s_ed.i_selected_sector ].verts[ s_ed.i_selected_line ][ 0 ] + s_ed.sectors[ s_ed.i_selected_sector ].verts[ ( s_ed.i_selected_line + 1 ) % s_ed.sectors[ s_ed.i_selected_sector ].i_numverts ][ 0 ] ) / 2.0;
	s_ed.sectors[ s_ed.i_selected_sector ].verts[ s_ed.i_selected_line + 1 ][ 1 ] = ( s_ed.sectors[ s_ed.i_selected_sector ].verts[ s_ed.i_selected_line ][ 1 ] + s_ed.sectors[ s_ed.i_selected_sector ].verts[ ( s_ed.i_selected_line + 1 ) % s_ed.sectors[ s_ed.i_selected_sector ].i_numverts ][ 1 ] ) / 2.0;
	s_ed.sectors[ s_ed.i_selected_sector ].lines[ s_ed.i_selected_line + 1 ] = s_ed.sectors[ s_ed.i_selected_sector ].lines[ s_ed.i_selected_line ];
	s_ed.sectors[ s_ed.i_selected_sector ].i_numverts++;
}

void delete_selvert()
{
	if(	s_ed.sectors[ s_ed.i_selected_sector ].i_numverts > 3 )
	{
		memmove( &s_ed.sectors[ s_ed.i_selected_sector ].verts[ s_ed.i_selected_vert ], &s_ed.sectors[ s_ed.i_selected_sector ].verts[ s_ed.i_selected_vert + 1 ],
			sizeof( vec2_t ) * s_ed.sectors[ s_ed.i_selected_sector ].i_numverts - ( s_ed.i_selected_vert + 1 ) );
		memmove( &s_ed.sectors[ s_ed.i_selected_sector ].lines[ s_ed.i_selected_vert ], &s_ed.sectors[ s_ed.i_selected_sector ].lines[ s_ed.i_selected_vert + 1 ],
			sizeof( line_t ) * s_ed.sectors[ s_ed.i_selected_sector ].i_numverts - ( s_ed.i_selected_vert + 1 ) );
		s_ed.sectors[ s_ed.i_selected_sector ].i_numverts--;
		s_ed.i_selected_vert = -1;
		s_ed.i_selected_line = -1;
	}
}

void load_map( char *fname )
{
	FILE *f;
	f = fopen( fname, "rb" );
	if( !f )
	{
		return;
	}
	fread( &s_ed.i_num_sectors, sizeof( int ), 1, f );
	fread( &s_ed.sectors, sizeof( sector_t ), s_ed.i_num_sectors, f );
	fread( &s_ed.i_num_entities, sizeof( int ), 1, f );
	fread( &s_ed.entities, sizeof( entity_t ), s_ed.i_num_entities, f );
	fclose( f );
}

void save_map( char *fname )
{
	FILE *f;
	f = fopen( fname, "wb" );
	if( !f )
	{
		return;
	}
	fwrite( &s_ed.i_num_sectors, sizeof( int ), 1, f );
	fwrite( &s_ed.sectors, sizeof( sector_t ), s_ed.i_num_sectors, f );
	fwrite( &s_ed.i_num_entities, sizeof( int ), 1, f );
	fwrite( &s_ed.entities, sizeof( entity_t ), s_ed.i_num_entities, f );
	fclose( f );
}


int main( int argc, char *argv[] )
{
    GtkBuilder *builder;
    GError     *error = NULL;
 
	memset( &s_ed, 0, sizeof( editor_t ) );
	s_ed.i_snap = 8;
	s_ed.i_selected_line = -1;
	s_ed.i_selected_vert = -1;
	s_ed.i_selected_sector = -1;
	s_ed.i_which_view = 0;

    /* Init GTK+ */
    gtk_init( &argc, &argv );
 
    /* Create new GtkBuilder object */
    builder = gtk_builder_new();
    /* Load UI from file. If error occurs, report it and quit application.
     * Replace "tut.glade" with your saved project. */
    if( ! gtk_builder_add_from_file( builder, "editor.glade", &error ) )
    {
        g_warning( "%s", error->message );
        g_free( error );
        return( 1 );
    }
 
    /* Get main window pointer from UI */
    window = GTK_WIDGET( gtk_builder_get_object( builder, "window1" ) );

	p_entry_linetex_top = GTK_ENTRY( gtk_builder_get_object( builder, "texup" ) );
	p_entry_linetex_bottom = GTK_ENTRY( gtk_builder_get_object( builder, "texlow" ) );
	p_entry_sector_floor = GTK_ENTRY( gtk_builder_get_object( builder, "texbottom" ) );
	p_entry_sector_ceil = GTK_ENTRY( gtk_builder_get_object( builder, "textop" ) );
	p_entry_entity_class = GTK_ENTRY( gtk_builder_get_object( builder, "entityclass" ) );


	adjustment_ztop = GTK_ADJUSTMENT( gtk_builder_get_object( builder, "adjustment2" ) );
	adjustment_zbot = GTK_ADJUSTMENT( gtk_builder_get_object( builder, "adjustment1" ) );
	adjustment_snap = GTK_ADJUSTMENT( gtk_builder_get_object( builder, "adjustment3" ) );
	gtk_adjustment_set_value( adjustment_snap, ( gdouble )s_ed.i_snap );


    /* Connect signals */
    gtk_builder_connect_signals( builder, NULL );
 
    /* Destroy builder, since we don't need it anymore */
    g_object_unref( G_OBJECT( builder ) );
 
    /* Show window. All other widgets are automatically shown by GtkBuilder */
    gtk_widget_show( window );
 
    /* Start main loop */
    gtk_main();
 
    return( 0 );
}
