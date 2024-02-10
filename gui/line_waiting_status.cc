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
	set_table_layout(1,0);
	update();
}

void gui_halt_waiting_catg_t::update()
{
	uint8 g_class = goods_manager_t::get_classes_catg_index(catg_index) - 1;
	haltestelle_t::connexions_map *connexions = halt->get_connexions(catg_index, g_class);

	remove_all();
	add_table(0,1);
	{
		if (connexions->empty()) {
			new_component<gui_label_t>("-", COL_INACTIVE);
		}
		else if (!update_seed) {
			new_component<gui_margin_t>(GOODS_COLOR_BOX_HEIGHT);
			new_component<gui_label_t>("0", SYSCOL_TEXT_WEAK);
		}
		else {
			bool got_one = false;
			bool overcrowded = (halt->get_status_color(catg_index == goods_manager_t::INDEX_PAS ? 0 : catg_index == goods_manager_t::INDEX_MAIL ? 1 : 2) == SYSCOL_OVERCROWDED);
			for (uint8 j = 0; j < goods_manager_t::get_count(); j++) {
				const goods_desc_t *wtyp = goods_manager_t::get_info(j);
				if (wtyp->get_catg_index() != catg_index) {
					continue;
				}
				const uint32 sum = halt->get_ware_summe(wtyp);
				if (sum > 0) {
					if (got_one) {
						new_component<gui_label_t>(", ", SYSCOL_TEXT);
					}

					PIXVAL goods_color = wtyp->get_color();
					new_component<gui_colorbox_t>(goods_color)->set_size(GOODS_COLOR_BOX_SIZE);
					gui_label_buf_t *lb = new_component<gui_label_buf_t>(overcrowded ? SYSCOL_OVERCROWDED : SYSCOL_TEXT);
					lb->buf().printf("%s %d", translator::translate(wtyp->get_name()), sum);
					lb->update();

					got_one = true;
				}
			}
		}
	}
	end_table();
	set_size(get_min_size());
}

void gui_halt_waiting_catg_t::draw(scr_coord offset)
{
	// update seed
	uint32 temp;
	if (catg_index == goods_manager_t::INDEX_PAS) {
		temp = halt->get_ware_summe(goods_manager_t::get_info(goods_manager_t::INDEX_PAS));
	}
	else if (catg_index == goods_manager_t::INDEX_MAIL) {
		temp = halt->get_ware_summe(goods_manager_t::get_info(goods_manager_t::INDEX_MAIL));
	}
	else {
		// freight
		temp = halt->get_finance_history(0, HALT_WAITING) - halt->get_ware_summe(goods_manager_t::get_info(goods_manager_t::INDEX_PAS)) - halt->get_ware_summe(goods_manager_t::get_info(goods_manager_t::INDEX_MAIL));
	}

	if(temp!= update_seed) {
		update_seed = temp;
		update();
	}
	gui_aligned_container_t::draw(offset);
}


gui_line_waiting_status_t::gui_line_waiting_status_t(linehandle_t line_)
{
	line = line_;

	set_table_layout(1, 0);
	set_table_frame(true,true);
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

		add_table(1,1); // main table
		{
			add_table(cols, 0)->set_margin(scr_size(D_MARGIN_LEFT, D_MARGIN_TOP), scr_size(D_MARGIN_LEFT, D_MARGIN_TOP));
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

				new_component<gui_empty_t>();
				// border
				new_component<gui_border_t>();
				for (uint8 i = 1; i < cols-1; ++i) {
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
		end_table();
	}
}

void gui_line_waiting_status_t::draw(scr_coord offset)
{
	if (line.is_bound()) {
		if (!line->get_schedule()->matches(world(), schedule)) {
			init();
		}
		// need to recheck child components size
		set_size(gui_aligned_container_t::get_size());
	}
	gui_aligned_container_t::draw(offset);
}
