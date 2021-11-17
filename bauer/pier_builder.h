/*
 * This file is part of the Simutrans-Extended project under the Artistic License.
 * (see LICENSE.txt)
 */

#ifndef BAUER_PIER_BUILDER_H
#define BAUER_PIER_BUILDER_H


#include "../tpl/stringhashtable_tpl.h"
#include "../dataobj/koord3d.h"

class pier_desc_t;
class grund_t;
class karte_ptr_t;
class player_t;
class tool_selector_t;

class pier_builder_t {
private:
    pier_builder_t() {}

    static karte_ptr_t welt;

    static stringhashtable_tpl<pier_desc_t *, N_BAGS_MEDIUM> desc_table;



    static const char * check_below_ways(player_t *player, koord3d pos, const pier_desc_t *desc, const uint8 rotation, bool halfheight);

    static const char * check_for_buildings(const grund_t *gr, const pier_desc_t *desc, const uint8 rotation);

    struct pier_finder_params{
        pier_finder_params(){
            above_way_ribi=0;
            below_way_ribi=0;
            above_slope=0;
            ground_slope=0;
            support_needed=0;
            support_avail=0;
            middle_mask_taken=0;
            deck_obj_present=0;
            sub_obj_present=-1;
            need_clearence=false;
            allow_low_waydeck=false;
            is_wet=false;
            on_deck=false;
        }

        uint64 support_needed;
        uint64 support_avail;
        uint64 middle_mask_taken;
        uint32 deck_obj_present;
        uint32 sub_obj_present;
        ribi_t::ribi above_way_ribi;
        ribi_t::ribi below_way_ribi;
        slope_t::type above_slope;
        slope_t::type ground_slope;
        bool need_clearence;
        bool allow_low_waydeck;
        bool is_wet;
        bool on_deck;

    };

public:

    /**
     * @brief lookup_deck_pos find the position of the deck on a given ground
     * @param gr ground to check
     * @return position of pier deck
     */
    static koord3d lookup_deck_pos(const grund_t *gr);

	/**
	 * Registers a new pier type and adds it to the list of build tools.
	 *
	 * @param desc Description of the bridge to register.
	 */
	static void register_desc(pier_desc_t *desc);

	/**
	 * Method to retrieve pier descriptor
	 * @param name name of the pier
	 * @return bridge descriptor or NULL if not found
	 */
	static const pier_desc_t *get_desc(const char *name);

	/**
	 * @brief get_desc_bad_load obtain pier requirements from context after a bad load
	 * @param pos position of pier
	 * @param owner owner of pier
	 * @return the best matching pier
	 */
	static const pier_desc_t *get_desc_bad_load(koord3d pos,player_t *owner,uint8 &rotation);

	static bool get_desc_context(pier_desc_t const *& descriptor, uint8& rotation, pier_finder_params params, bool allow_inexact=false);


	/**
	 * build a single pier
	 * @param player The player wanting to build the pier
	 * @param pos the position of the pier
	 * @param desc Descriptor of the pier
	 * @param rotation The orientation of the pier
	 * @return NULL on Sucess or error message
	 */
	static const char *build( player_t *player, koord3d pos, const pier_desc_t *desc, const uint8 rotation);

	/**
	 * Remove a pier
	 * @param player player removing the pier
	 * @param pos position of the pier
	 * @return NULL on sucess or error message
	 */
	static const char *remove( player_t *player, koord3d pos);

	/**
	 * fill_menu fill menu of all piers
	 * @param tool_selector gui object of toolbar
	 */
	static void fill_menu(tool_selector_t *tool_selector);
};

#endif // PIER_BUILDER_H
