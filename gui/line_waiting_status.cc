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
#include "../simworld.h"


gui_halt_waiting_catg_t::gui_halt_waiting_catg_t(halthandle_t h, uint8 catg)
{
	halt = h;
	catg_index = catg;
	set_size(scr_size(LINESPACE<<2, D_LABEL_HEIGHT));
}

void gui_halt_waiting_catg_t::draw(scr_coord offset)
{
	offset += pos;
	scr_coord_val xoff = D_H_SPACE;
	uint8 g_class = goods_manager_t::get_classes_catg_index(catg_index) - 1;
	haltestelle_t::connexions_map *connexions = halt->get_connexions(catg_index, g_class);
	if (connexions->empty()) {
		// no connection
		xoff += display_proportional_clip_rgb(offset.x + xoff, offset.y, "-", ALIGN_LEFT, SYSCOL_TEXT_WEAK, true);
	}
	else {
		bool got_one = false;
		bool overcrowded = (halt->get_status_color(catg_index==goods_manager_t::INDEX_PAS ? 0 : catg_index == goods_manager_t::INDEX_MAIL ? 1 : 2)==SYSCOL_OVERCROWDED);

		for (uint8 j = 0; j < goods_manager_t::get_count(); j++) {
			const goods_desc_t *wtyp = goods_manager_t::get_info(j);
			if (wtyp->get_catg_index() != catg_index) {
				continue;
			}
			const uint32 sum = halt->get_ware_summe(wtyp);
			if (sum > 0) {
				buf.clear();
				display_colorbox_with_tooltip(offset.x + xoff, offset.y, GOODS_COLOR_BOX_HEIGHT, GOODS_COLOR_BOX_HEIGHT, wtyp->get_color(), false, NULL);
				xoff += GOODS_COLOR_BOX_HEIGHT+2;

				buf.printf("%s ", translator::translate(wtyp->get_name()));
				xoff += display_proportional_clip_rgb(offset.x + xoff, offset.y, buf, ALIGN_LEFT, SYSCOL_TEXT, true);
				buf.clear();
				buf.printf("%d ", sum);
				xoff += display_proportional_clip_rgb(offset.x + xoff, offset.y, buf, ALIGN_LEFT, overcrowded ? SYSCOL_OVERCROWDED : SYSCOL_TEXT, true);
				xoff += D_H_SPACE;
				got_one = true;
			}
		}
		if (!got_one) {
			xoff += display_proportional_clip_rgb(offset.x + xoff, offset.y, "0", ALIGN_LEFT, SYSCOL_TEXT_WEAK, true);
		}
	}

	set_size( scr_size(xoff+D_H_SPACE*2, D_LABEL_HEIGHT) );
}


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
