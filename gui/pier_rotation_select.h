/*
 * This file is part of the Simutrans-Extended project under the Artistic License.
 * (see LICENSE.txt)
 */

#ifndef GUI_PIER_ROTATION_SELECT_H
#define GUI_PIER_ROTATION_SELECT_H


#include "components/action_listener.h"
#include "gui_frame.h"
#include "components/gui_button.h"

class pier_desc_t;
class tool_build_pier_t;

class pier_rotation_select_t : public gui_frame_t, action_listener_t{
    const pier_desc_t *desc;

    button_t actionbutton[4];

    static tool_build_pier_t tool;
public:
    pier_rotation_select_t(const pier_desc_t *desc);

    bool action_triggered(gui_action_creator_t *comp, value_t extra) override;
};

#endif // PIER_ROTATION_SELECT_H
