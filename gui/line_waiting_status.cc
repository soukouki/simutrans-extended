/*
 * This file is part of the Simutrans-Extended project under the Artistic License.
 * (see LICENSE.txt)
 */

#include "line_waiting_status.h"
#include "halt_info.h"
#include "components/gui_divider.h"
#include "components/gui_image.h"
#include "components/gui_schedule_item.h"
#include "../player/simplay.h"
#include "../simhalt.h"
#include "../simworld.h"

gui_line_waiting_status_t::gui_line_waiting_status_t(linehandle_t line_)
{
	line = line_;

	set_table_layout(1, 0);
	set_spacing(scr_size(1, 0));
	init();
}

void gui_line_waiting_status_t::init()
{
	remove_all();
	if (line.is_bound()) {
		schedule = line->get_schedule();
		if( !schedule->get_count() ) return; // nothing to show

		uint8 cols; // table cols
		cols = line->get_goods_catg_index().get_count()+show_name+1;

		add_table(cols, 0);
		{
			// header
			new_component<gui_empty_t>();
			if (show_name) {
				new_component<gui_label_t>("stations");
			}
			for (uint8 catg_index = 0; catg_index < goods_manager_t::get_max_catg_index(); catg_index++) {
				if (line->get_goods_catg_index().is_contained(catg_index) ) {
					add_table(2,1);
					{
						new_component<gui_image_t>(goods_manager_t::get_info_catg_index(catg_index)->get_catg_symbol(), 0, ALIGN_NONE, true);
						new_component<gui_label_t>(goods_manager_t::get_info_catg_index(catg_index)->get_catg_name());
					}
					end_table();
				}
			}

			// border
			new_component<gui_empty_t>();
			for (uint8 i = 1; i < cols; ++i) {
				new_component<gui_border_t>();
			}

			uint8 entry_idx = 0;
			FORX(minivec_tpl<schedule_entry_t>, const& i, schedule->entries, ++entry_idx) {
				halthandle_t const halt = haltestelle_t::get_halt(i.pos, line->get_owner());
				if( !halt.is_bound() ) { continue; }

				const bool is_interchange = (halt->registered_lines.get_count() + halt->registered_convoys.get_count()) > 1;
				new_component<gui_schedule_entry_number_t>(entry_idx, halt->get_owner()->get_player_color1(),
					is_interchange ? gui_schedule_entry_number_t::number_style::interchange : gui_schedule_entry_number_t::number_style::halt,
					scr_size(D_ENTRY_NO_WIDTH, max(D_POS_BUTTON_HEIGHT, D_ENTRY_NO_HEIGHT)),
					halt->get_basis_pos3d()
				);

				if (show_name) {
					const bool can_serve = halt->can_serve(line);
					gui_label_buf_t *lb = new_component<gui_label_buf_t>(can_serve ? SYSCOL_TEXT : COL_INACTIVE);
					if (!can_serve) {
						lb->buf().printf("(%s)", halt->get_name());
					}
					else {
						lb->buf().append(halt->get_name());
					}
					lb->update();
				}

				for (uint8 catg_index = 0; catg_index < goods_manager_t::get_max_catg_index(); catg_index++) {
					if (line->get_goods_catg_index().is_contained(catg_index)) {
						new_component<gui_halt_waiting_catg_t>(halt, catg_index);
					}
				}
			}
		}
		end_table();
	}
}

void gui_line_waiting_status_t::draw(scr_coord offset)
{
	if (line.is_bound()) {
		if (!line->get_schedule()->matches(world(), schedule)) {
			init();
		}
	}
	set_size(get_size());
	gui_aligned_container_t::draw(offset);
}
