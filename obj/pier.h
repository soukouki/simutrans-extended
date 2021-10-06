/*
 * This file is part of the Simutrans-Extended project under the Artistic License.
 * (see LICENSE.txt)
 */

#ifndef PIER_H
#define PIER_H

class karte_t;

#include "../descriptor/pier_desc.h"
#include "simobj.h"

class pier_t : public obj_no_info_t
{
private:
    const pier_desc_t *desc;

    image_id back_image;
    image_id front_image;

    uint8_t rotation;

protected:
    void rdwr(loadsave_t *file) override;

public:
    pier_t(loadsave_t *file);
    pier_t(koord3d pos, player_t *player, const pier_desc_t *desc, uint8_t rot);

    ~pier_t(){
        dbg->message("pier_t","pier destroyed");
        return;
    }

    const pier_desc_t *get_desc() const {return desc; }

    ribi_t::ribi get_above_ribi() const {return desc->get_above_way_ribi(rotation);}
    ribi_t::ribi get_below_ribi() const {return desc->get_below_way_ribi(rotation);}

    uint32 get_base_mask() const {return desc->get_base_mask(rotation);}
    uint32 get_middle_mask() const {return desc->get_middle_mask(rotation);}
    uint32 get_support_mask() const {return desc->get_support_mask(rotation);}

    //get the total for entire tile
    static ribi_t::ribi get_above_ribi_total(const grund_t* gr);
    static ribi_t::ribi get_below_ribi_total(const grund_t* gr);

    static uint32 get_base_mask_total(const grund_t* gr);
    static uint32 get_middle_mask_total(const grund_t* gr);
    static uint32 get_support_mask_total(const grund_t* gr);

    image_id get_image() const override;
    image_id get_front_image() const override;

    const char *get_name() const OVERRIDE {return "Pier";}
#ifdef INLINE_OBJ_TYPE
#else
    typ get_typ() const OVERRIDE { return pier; }
#endif

	void calc_image() override;

	bool check_season(const bool calc_only_season_change) OVERRIDE
	{
		if(  !calc_only_season_change  ) {
			calc_image();
		}
		return true;
	}

	void finish_rd() override;

	void cleanup(player_t *player) override;

	void rotate90() override;

	const char *is_deletable(const player_t *player) override;
};

#endif // PIER_H
