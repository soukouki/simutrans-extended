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

    static koord3d lookup_deck_pos(const grund_t *gr, koord3d pos);

    static const char * check_below_ways(player_t *player, koord3d pos, const pier_desc_t *desc, const uint8 rotation, bool halfheight);

    static const char * check_for_buildings(const grund_t *gr, const pier_desc_t *desc, const uint8 rotation);

public:

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
