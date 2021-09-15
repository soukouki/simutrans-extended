/*
 * This file is part of the Simutrans-Extended project under the Artistic License.
 * (see LICENSE.txt)
 */

#include "pedestrian_info.h"
#include "components/gui_label.h"

#include "../simworld.h"
#include "../display/simgraph.h"
#include "../display/viewport.h"
#include "../descriptor/pedestrian_desc.h"


pedestrian_info_t::pedestrian_info_t(const pedestrian_t* obj) :
	gui_frame_t(translator::translate(obj->get_name()), NULL),
	view(obj, scr_size(max(64, get_base_tile_raster_width()), max(56, (get_base_tile_raster_width() * 7) / 8)))
{
	set_table_layout(1, 0);
	add_component(&view);
	if (char const* const maker = obj->get_desc()->get_copyright()) {
		add_table(2, 2)->set_spacing(scr_size(0, 0)); {
			new_component_span<gui_label_t>("Constructed by", 2);
			new_component<gui_margin_t>(LINESPACE);
			new_component<gui_label_t>(maker);
		} end_table();
	}
	reset_min_windowsize();
	set_windowsize(get_min_windowsize());
}

void pedestrian_info_t::draw(scr_coord pos, scr_size size)
{
	gui_frame_t::draw(pos, size);
	pos += scr_size(D_V_SPACE + D_MARGIN_LEFT + 2, D_TITLEBAR_HEIGHT + D_MARGIN_TOP + 2);
	display_proportional_clip_rgb(pos.x, pos.y, view.get_obj()->get_pos().get_2d().get_fullstr(), ALIGN_LEFT, color_idx_to_rgb(COL_WHITE), true);
}


