/*
 * This file is part of the Simutrans-Extended project under the Artistic License.
 * (see LICENSE.txt)
 */

#include "gui_line_lettercode.h"
#include "../../utils/simstring.h"
//#include "../../display/simgraph.h"


gui_line_lettercode_t::gui_line_lettercode_t(PIXVAL color_val, uint8 line_lettercode_style)
{
	style = line_lettercode_style;
	line_color = color_val;

	size = scr_size(LINESPACE, LINEASCENT+6);
};

void gui_line_lettercode_t::set_linecode_l(const char *new_name)
{
	tstrncpy(linecode_l, new_name, lengthof(linecode_l));
}

void gui_line_lettercode_t::set_linecode_r(const char *new_name)
{
	tstrncpy(linecode_r, new_name, lengthof(linecode_r));
}

void gui_line_lettercode_t::draw(scr_coord offset)
{
	if (style || (style==0 && line_color>0) || linecode_l[0]!='\0' || linecode_r[0]!='\0' ) {
		offset += pos;
		size.w = max(display_line_lettercode_rgb(offset.x, offset.y, line_color, style, linecode_l, linecode_r), LINESPACE);
	}

	if (size != get_size()) {
		set_size(size);
	}
};
