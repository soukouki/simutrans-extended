/*
 * This file is part of the Simutrans-Extended project under the Artistic License.
 * (see LICENSE.txt)
 */


#include "gui_waytype_image_box.h"

#include "../../simskin.h"
#include "../../simworld.h"

#include "../../descriptor/skin_desc.h"


gui_waytype_image_box_t::gui_waytype_image_box_t(waytype_t wt) :
	gui_image_t(skinverwaltung_t::get_waytype_skin(wt)->get_image_id(0), 0, 0, true)
{
	set_waytype(wt);
}


void gui_waytype_image_box_t::set_waytype(waytype_t wt)
{
	set_image(skinverwaltung_t::get_waytype_skin(wt)->get_image_id(0), true);
	bgcol = world()->get_settings().get_waytype_color(wt);
	set_size(scr_size( gui_image_t::get_size().w, gui_image_t::get_size().h) );
}

void gui_waytype_image_box_t::draw(scr_coord offset)
{
	if( bgcol!=0 ) {
		display_filled_roundbox_clip(pos.x+offset.x,   pos.y+offset.y,   size.w,   size.h, SYSCOL_SHADOW, false);
		display_filled_roundbox_clip(pos.x+offset.x+1, pos.y+offset.y+1, size.w-2, size.h-2, bgcol, false);
	}
	gui_image_t::draw(offset);
}


void gui_waytype_image_box_t::set_size( scr_size size_par )
{
	if( id  !=  IMG_EMPTY ) {

		scr_coord_val x,y,w,h;
		display_get_base_image_offset( id, &x, &y, &w, &h );

		remove_offset = scr_coord(-x,-y);

		size_par = scr_size( max(x+w+remove_offset.x+6, size_par.w), max(y+h+remove_offset.y+4,size_par.h) );
		remove_offset = remove_offset+scr_size((size_par.w-w)/2,(size_par.h-h)/2);
	}

	gui_component_t::set_size(size_par);
}
