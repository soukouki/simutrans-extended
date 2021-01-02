/*
 * This file is part of the Simutrans-Extended project under the Artistic License.
 * (see LICENSE.txt)
 */

#include <algorithm>

#include "../boden/grund.h"
#include "../simworld.h"
#include "../display/simimg.h"
#include "../simobj.h"
#include "../player/simplay.h"
#include "../simtool.h"

#include "../dataobj/loadsave.h"
#include "../dataobj/ribi.h"
#include "../dataobj/scenario.h"
#include "../dataobj/translator.h"
#include "../dataobj/environment.h"

#include "../descriptor/bridge_desc.h"
#include "../descriptor/tunnel_desc.h"
#include "../descriptor/way_obj_desc.h"

#include "../gui/tool_selector.h"

#include "../boden/grund.h"
#include "../boden/wege/weg.h"
#include "../boden/wege/strasse.h"

#include "../tpl/stringhashtable_tpl.h"

#include "../utils/simstring.h"

#include "bruecke.h"
#include "tunnel.h"
#include "wayobj.h"

#ifdef MULTI_THREAD
#include "../utils/simthread.h"
static pthread_mutex_t wayobj_calc_image_mutex = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
#endif

// the descriptions ...
const way_obj_desc_t *wayobj_t::default_oberleitung=NULL;

stringhashtable_tpl<way_obj_desc_t *, N_BAGS_MEDIUM> wayobj_t::table;

wayobj_t::wayobj_t(loadsave_t* const file) :
#ifdef INLINE_OBJ_TYPE
	obj_no_info_t(obj_t::wayobj)
#else
	obj_no_info_t()
#endif
	, hang(slope_t::flat)
{
	rdwr(file);
}


wayobj_t::wayobj_t(koord3d const pos, player_t* const owner, ribi_t::ribi const d, way_obj_desc_t const* const b) :
#ifdef INLINE_OBJ_TYPE
	obj_no_info_t(obj_t::wayobj, pos)
#else
	obj_no_info_t(pos)
#endif
	, hang(slope_t::flat)
{
	desc = b;
	dir = d;
	set_owner(owner);
}


wayobj_t::~wayobj_t()
{
	if(!desc) {
		return;
	}
	player_t::add_maintenance(get_owner(), -desc->get_maintenance(), get_waytype());
	if(desc->is_overhead_line() && !welt->is_destroying()) {
		grund_t *gr=welt->lookup(get_pos());
		weg_t *weg=NULL;
		if(gr) {
			const waytype_t wt = (desc->get_wtyp()==tram_wt) ? track_wt : desc->get_wtyp();
			weg = gr->get_weg(wt);
			if (wt == any_wt) {
				weg_t *weg2 = welt->lookup(get_pos())->get_weg_nr(1);
				if (weg2) {
					weg2->set_electrify(true);
					if (weg2->get_max_speed()>desc->get_topspeed()) {
						weg2->set_max_speed(desc->get_topspeed());
					}
				}
			}
			if(weg) {
				// Weg wieder freigeben, wenn das Signal nicht mehr da ist.
				weg->set_electrify(false);
				// restore old speed limit and way constraints
				weg->reset_way_constraints();
				sint32 max_speed;
				const slope_t::type hang = gr ? gr->get_weg_hang() : slope_t::flat;
				if(hang != slope_t::flat)
				{
					const uint slope_height = (hang & 7) ? 1 : 2;
					if(slope_height == 1)
					{
						max_speed = weg->get_desc()->get_topspeed_gradient_1();
					}
					else
					{
						max_speed = weg->get_desc()->get_topspeed_gradient_2();
					}
				}
				else
				{
					max_speed = weg->get_desc()->get_topspeed();
				}
				if(weg->hat_gehweg())
				{
					max_speed = 50;
				}
				if(gr->get_typ()==grund_t::tunnelboden)
				{
					tunnel_t *t = gr->find<tunnel_t>(1);
					if(t)
					{
						if(hang != slope_t::flat)
						{
							const uint slope_height = (hang & 7) ? 1 : 2;
							if(slope_height == 1)
							{
								max_speed = t->get_desc()->get_topspeed_gradient_1();
							}
							else
							{
								max_speed = t->get_desc()->get_topspeed_gradient_2();
							}
						}
						else
						{
							max_speed = t->get_desc()->get_topspeed();
						}
						weg->add_way_constraints(t->get_desc()->get_way_constraints());
					}
				}
				if(gr->get_typ()==grund_t::brueckenboden)
				{
					bruecke_t *b = gr->find<bruecke_t>(1);
					if(b)
					{
						if(hang != slope_t::flat)
						{
							const uint slope_height = (hang & 7) ? 1 : 2;
							if(slope_height == 1)
							{
								max_speed = b->get_desc()->get_topspeed_gradient_1();
							}
							else
							{
								max_speed = b->get_desc()->get_topspeed_gradient_2();
							}
						}
						else
						{
							max_speed = b->get_desc()->get_topspeed();
						}
						weg->add_way_constraints(b->get_desc()->get_way_constraints());
					}
				}
				weg->set_max_speed(max_speed);
			}
			else {
				dbg->warning("wayobj_t::~wayobj_t()","ground was not a way!");
			}
		}
	}

	mark_image_dirty( get_front_image(), 0 );
	mark_image_dirty( get_image(), 0 );
	grund_t *gr = welt->lookup( get_pos() );
	if( gr ) {
		for( uint8 i = 0; i < 4; i++ ) {
			// Remove ribis from adjacent wayobj.
			if( ribi_t::nsew[i] & get_dir() ) {
				grund_t *next_gr;
				if( gr->get_neighbour( next_gr, desc->get_wtyp(), ribi_t::nsew[i] ) ) {
					wayobj_t *wo2 = next_gr->get_wayobj( desc->get_wtyp() );
					if( wo2 ) {
						wo2->mark_image_dirty( wo2->get_front_image(), 0 );
						wo2->mark_image_dirty( wo2->get_image(), 0 );
						wo2->set_dir( wo2->get_dir() & ~ribi_t::backward(ribi_t::nsew[i]) ); // This has the effect of looking for directions in front of this way object (the ~ combined with the ribi_t::backward).
						wo2->mark_image_dirty( wo2->get_front_image(), 0 );
						wo2->mark_image_dirty( wo2->get_image(), 0 );
						wo2->set_flag(obj_t::dirty);
					}
				}
			}
		}
	}
}


void wayobj_t::rdwr(loadsave_t *file)
{
	xml_tag_t t( file, "wayobj_t" );
	obj_t::rdwr(file);
	if(file->get_version_int()>=89000) {
		uint8 ddir = dir;
		file->rdwr_byte(ddir);
		dir = ddir;
		if(file->is_saving()) {
			const char *s = desc->get_name();
			file->rdwr_str(s);
		}
		else {
			char bname[128];
			file->rdwr_str(bname, lengthof(bname));

			desc = wayobj_t::table.get(bname);
			if(desc==NULL) {
				desc = wayobj_t::table.get(translator::compatibility_name(bname));
				if(desc==NULL) {
					if(strstr(bname,"atenary")  ||  strstr(bname,"electri")  ||  strstr(bname,"power")  ) {
						desc = default_oberleitung;
					}
				}
				if(desc==NULL) {
					dbg->warning("wayobj_t::rwdr", "description %s for wayobj_t at %d,%d not found, will be removed!", bname, get_pos().x, get_pos().y );
					welt->add_missing_paks( bname, karte_t::MISSING_WAYOBJ );
				}
				else {
					dbg->warning("wayobj_t::rwdr", "wayobj %s at %d,%d replaced by %s", bname, get_pos().x, get_pos().y, desc->get_name() );
				}
			}
		}
	}
	else {
		desc = default_oberleitung;
		dir = dir_unknown;
	}
}


void wayobj_t::cleanup(player_t *player)
{
	if(desc) {
		player_t::book_construction_costs(player, -desc->get_value(), get_pos().get_2d(), desc->get_wtyp());
	}
}


// returns NULL, if removal is allowed
// players can remove public owned wayobjs
const char *wayobj_t:: is_deletable(const player_t *player)
{
	if(  get_player_nr()==1  ) {
		return NULL;
	}
	return obj_t:: is_deletable(player);
}


void wayobj_t::finish_rd()
{
	// (re)set dir
	if(dir==dir_unknown) {
		const waytype_t wt = (desc->get_wtyp()==tram_wt) ? track_wt : desc->get_wtyp();
		weg_t *w=welt->lookup(get_pos())->get_weg(wt);
		if(w) {
			dir = w->get_ribi_unmasked();
		}
		else {
			dir = ribi_t::all;
		}
	}

	const waytype_t wt = (desc->get_wtyp()==tram_wt) ? track_wt : desc->get_wtyp();
	const grund_t* gr = welt->lookup(get_pos());
	weg_t *weg = gr->get_weg(wt);

	// electrify a way if we are a catenary
	if (desc->is_overhead_line())
	{
		if(weg)
		{
			// Weg wieder freigeben, wenn das Signal nicht mehr da ist.
			weg->set_electrify(true);
			sint32 way_top_speed;
			if(hang != slope_t::flat)
			{
				const uint slope_height = (hang & 7) ? 1 : 2;
				if(slope_height == 1)
				{
					way_top_speed = desc->get_topspeed_gradient_1();
				}
				else
				{
					way_top_speed = desc->get_topspeed_gradient_2();
				}
			}
			else
			{
				way_top_speed = desc->get_topspeed();
			}

			if(weg->get_max_speed() > way_top_speed)
			{
				weg->set_max_speed(way_top_speed);
			}
			// Add the way constraints together.
			weg->add_way_constraints(desc->get_way_constraints());
		}
		else
		{
			dbg->warning("wayobj_t::finish_rd()","ground was not a way!");
		}
	}

	player_t::add_maintenance(get_owner(), desc->get_maintenance(), desc->get_wtyp());
}


void wayobj_t::rotate90()
{
	obj_t::rotate90();
	dir = ribi_t::rotate90( dir);
	hang = slope_t::rotate90( hang );
}


// helper function: gets the ribi on next tile
ribi_t::ribi wayobj_t::find_next_ribi(const grund_t *start, ribi_t::ribi const dir, const waytype_t wt) const
{
	grund_t *to;
	ribi_t::ribi r1 = ribi_t::none;
	if(start->get_neighbour(to,wt,dir)) {
		const wayobj_t* wo = to->get_wayobj( wt );
		if(wo) {
			r1 = wo->get_dir();
		}
	}
	return r1;
}


void wayobj_t::calc_image()
{
#ifdef MULTI_THREAD
	pthread_mutex_lock( &wayobj_calc_image_mutex );
#endif
	grund_t *gr = welt->lookup(get_pos());
	diagonal = false;
	if(gr) {
		const waytype_t wt = (desc->get_wtyp()==tram_wt) ? track_wt : desc->get_wtyp();
		weg_t *w=gr->get_weg(wt);
		if(!w) {
			dbg->error("wayobj_t::calc_image()","without way at (%s)", get_pos().get_str() );
			// well, we are not on a way anymore? => delete us
			cleanup(get_owner());
			delete this;
			gr->set_flag(grund_t::dirty);
#ifdef MULTI_THREAD
			pthread_mutex_unlock( &wayobj_calc_image_mutex );
#endif
			return;
		}

		ribi_t::ribi sec_way_ribi_unmasked = 0;
		if(wt == any_wt) {
			if (weg_t *sec_w = gr->get_weg_nr(1)) {
				sec_way_ribi_unmasked = sec_w->get_ribi_unmasked();
			}
		}

		set_yoff( -gr->get_weg_yoff() );
		set_xoff( 0 );
		if (get_desc()->is_noise_barrier()) {
			dir |= (w->get_ribi_unmasked() | sec_way_ribi_unmasked);
		} else {
			dir &= (w->get_ribi_unmasked() | sec_way_ribi_unmasked);
		}

		// if there is a slope, we are finished, only four choices here (so far)
		hang = gr->get_weg_hang();
		if(hang!=slope_t::flat) {
#ifdef MULTI_THREAD
			pthread_mutex_unlock( &wayobj_calc_image_mutex );
#endif
			return;
		}

		// find out whether using diagonals or straight lines
		if(ribi_t::is_bend(dir)  &&  desc->has_diagonal_image()) {
			ribi_t::ribi r1 = ribi_t::none, r2 = ribi_t::none;

			// get the ribis of the ways that connect to us
			// r1 will be 45 degree clockwise ribi (eg nordost->east), r2 will be anticlockwise ribi (eg nordost->north)
			r1 = find_next_ribi( gr, ribi_t::rotate45(dir), wt );
			r2 = find_next_ribi( gr, ribi_t::rotate45l(dir), wt );

			// diagonal if r1 or r2 are our reverse and neither one is 90 degree rotation of us
			diagonal = (r1 == ribi_t::backward(dir) || r2 == ribi_t::backward(dir)) && r1 != ribi_t::rotate90l(dir) && r2 != ribi_t::rotate90(dir);

			if(diagonal) {
				// with this, we avoid calling us endlessly
				// HACK (originally by hajo?)
				static int rekursion = 0;

				if(rekursion == 0) {
					grund_t *to;
					rekursion++;
					for(int r = 0; r < 4; r++) {
						if(gr->get_neighbour(to, wt, ribi_t::nsew[r])) {
							wayobj_t* wo = to->get_wayobj( wt );
							if(wo) {
								wo->calc_image();
							}
						}
					}
					rekursion--;
				}

				image_id after = desc->get_front_diagonal_image_id(dir);
				image_id image = desc->get_back_diagonal_image_id(dir);
				if(image==IMG_EMPTY  &&  after==IMG_EMPTY) {
					// no diagonals available
					diagonal = false;
				}
			}

			if(  ribi_t::is_threeway(dir)  ) {
			}
		}
	}
#ifdef MULTI_THREAD
	pthread_mutex_unlock( &wayobj_calc_image_mutex );
#endif
}



/******************** static stuff from here **********************/


/* better use this constrcutor for new wayobj; it will extend a matching obj or make an new one
 */
const char *wayobj_t::extend_wayobj_t(koord3d pos, player_t *owner, ribi_t::ribi dir, const way_obj_desc_t *desc)
{
	grund_t *gr=welt->lookup(pos);
	if(gr)
	{
		wayobj_t *existing_wayobj = gr->get_wayobj( desc->get_wtyp() );
		if( existing_wayobj ) {
			if(  existing_wayobj->get_desc() != desc  &&  player_t::check_owner(owner, existing_wayobj->get_owner())  ) {
				// replace slower by faster
				dir = dir | existing_wayobj->get_dir();
				gr->set_flag(grund_t::dirty);
				delete existing_wayobj;
			}
			else {
				// extend this one instead
				existing_wayobj->set_dir(dir|existing_wayobj->get_dir());
				existing_wayobj->mark_image_dirty( existing_wayobj->get_front_image(), 0 );
				existing_wayobj->mark_image_dirty( existing_wayobj->get_image(), 0 );
				existing_wayobj->set_flag(obj_t::dirty);
				return NULL;
			}
		}

		if(desc->is_noise_barrier()) {
			if (gr->removing_way_would_disrupt_public_right_of_way(road_wt)) {
				return "Cannot remove a public right of way without providing an adequate diversionary route";
			}
			if (gr->removing_road_would_disconnect_city_building() ||
			    gr->removing_road_would_break_monument_loop()) {
				return "Cannot delete a road where to do so would leave a city building unconnected by road.";
			}

			if (gr->get_halt().is_bound()) {
				return "Cannot combine way object and halt.";
			}

			strasse_t *str = static_cast<strasse_t *>(gr->get_weg(road_wt));
			if (str == NULL) {
				return "";
			}
			str->set_gehweg(false);
			gr->get_weg(road_wt)->set_public_right_of_way(false);
		}

		// nothing found => make a new one
		wayobj_t *wo = new wayobj_t(pos,owner,dir,desc);
		gr->obj_add(wo);
		wo->finish_rd();
		wo->calc_image();
		wo->mark_image_dirty( wo->get_front_image(), 0 );
		wo->set_flag(obj_t::dirty);
		gr->calc_image();
		for(int r = 0; r < 4; r++) {
			grund_t *to;
			if(gr->get_neighbour(to, invalid_wt, ribi_t::nsew[r])) {
				to->calc_image();
			}
		}
		player_t::book_construction_costs( owner,  -desc->get_value(), pos.get_2d(), desc->get_wtyp());

		for( uint8 i = 0; i < 4; i++ ) {
		// Extend wayobjects around the new one, that aren't already connected.
			if( ribi_t::nsew[i] & ~wo->get_dir() ) {
				grund_t *next_gr;
				if( gr->get_neighbour( next_gr, desc->get_wtyp(), ribi_t::nsew[i] ) ) {
					wayobj_t *wo2 = next_gr->get_wayobj( desc->get_wtyp() );
					if( wo2 ) {
						wo2->set_dir( wo2->get_dir() | ribi_t::backward(ribi_t::nsew[i]) );
						wo2->mark_image_dirty( wo2->get_front_image(), 0 );
						wo->set_dir( wo->get_dir() | ribi_t::nsew[i] );
						wo->mark_image_dirty( wo->get_front_image(), 0 );
					}
				}
			}
		}
	}

	return NULL;
}



// to sort wayobj for always the same menu order
static bool compare_wayobj_desc(const way_obj_desc_t* a, const way_obj_desc_t* b)
{
	int diff = a->get_wtyp() - b->get_wtyp();
	if (diff == 0) {
		diff = a->get_topspeed() - b->get_topspeed();
	}
	if (diff == 0) {
		/* Some speed: sort by name */
		diff = strcmp(a->get_name(), b->get_name());
	}
	return diff < 0;
}


bool wayobj_t::successfully_loaded()
{
	if(table.empty()) {
		dbg->warning("wayobj_t::successfully_loaded()", "No obj found - may crash when loading catenary.");
	}

	way_obj_desc_t const* def = 0;
	for(auto const& i : table) {
		way_obj_desc_t const& b = *i.value;
		if (b.is_overhead_line())							continue;
		if (b.get_wtyp()     != track_wt)                   continue;
		if (def && def->get_topspeed() >= b.get_topspeed()) continue;
		def = &b;
	}
	default_oberleitung = def;

	return true;
}


bool wayobj_t::register_desc(way_obj_desc_t *desc)
{
	// avoid duplicates with same name
	if(  way_obj_desc_t *old_desc = table.remove(desc->get_name())  ) {
		dbg->doubled( "wayobj", desc->get_name() );
		tool_t::general_tool.remove( old_desc->get_builder() );
		delete old_desc->get_builder();
		delete old_desc;
	}

	if(  desc->get_cursor()->get_image_id(1)!=IMG_EMPTY  ) {
		// only add images for wayobjexts with cursor ...
		tool_build_wayobj_t *tool = new tool_build_wayobj_t();
		tool->set_icon( desc->get_cursor()->get_image_id(1) );
		tool->cursor = desc->get_cursor()->get_image_id(0);
		tool->set_default_param(desc->get_name());
		tool_t::general_tool.append( tool );
		desc->set_builder( tool );
	}
	else {
		desc->set_builder( NULL );
	}

	table.put(desc->get_name(), desc);
DBG_DEBUG( "wayobj_t::register_desc()","%s", desc->get_name() );
	return true;
}


/**
 * Fill menu with icons of given wayobjects from the list
 */
void wayobj_t::fill_menu(tool_selector_t *tool_selector, waytype_t wtyp, sint16 /*sound_ok*/)
{
	// check if scenario forbids this
	if (!welt->get_scenario()->is_tool_allowed(welt->get_active_player(), TOOL_BUILD_WAYOBJ | GENERAL_TOOL, wtyp)) {
		return;
	}

	const uint16 time=welt->get_timeline_year_month();

	vector_tpl<const way_obj_desc_t *>matching;

	for(auto const& i : table) {
		way_obj_desc_t const* const desc = i.value;
		if(  desc->is_available(time)  ) {

			DBG_DEBUG("wayobj_t::fill_menu()", "try to add %s(%p)", desc->get_name(), desc);
			if(  desc->get_builder()  &&  wtyp==desc->get_wtyp()  ) {
				// only add items with a cursor
				matching.append(desc);
			}
		}
	}
	// sort the tools before adding to menu
	std::sort(matching.begin(), matching.end(), compare_wayobj_desc);
	FOR(vector_tpl<way_obj_desc_t const*>, const i, matching) {
		tool_selector->add_tool_selector(i->get_builder());
	}
}


const way_obj_desc_t *wayobj_t::get_overhead_line(waytype_t wt, uint16 time)
{
	for(auto const& i : table) {
		way_obj_desc_t const* const desc = i.value;
		if(  desc->is_available(time)  &&  desc->get_wtyp()==wt  &&  desc->is_overhead_line()) {
			return desc;
		}
	}
	return NULL;
}


const way_obj_desc_t* wayobj_t::find_desc(const char *str)
{
	return wayobj_t::table.get(str);
}
