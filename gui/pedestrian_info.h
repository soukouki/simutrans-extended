/*
 * This file is part of the Simutrans-Extended project under the Artistic License.
 * (see LICENSE.txt)
 */

#ifndef GUI_PEDESTRIAN_INFO_H
#define GUI_PEDESTRIAN_INFO_H


//#include "../obj/simobj.h"
#include "base_info.h"
#include "components/gui_obj_view_t.h"
#include "../vehicle/pedestrian.h"

/**
 * An adapter class to display info windows for things (objects)
 */
class pedestrian_info_t : public base_infowin_t
{
	obj_view_t view;
public:
	pedestrian_info_t(const pedestrian_t* obj);

	bool has_sticky() const OVERRIDE { return false; }

	void draw(scr_coord pos, scr_size size) OVERRIDE;
};


#endif
