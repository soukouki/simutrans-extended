/*
 * This file is part of the Simutrans-Extended project under the Artistic License.
 * (see LICENSE.txt)
 */

#ifndef UTILS_CHECKLIST_H
#define UTILS_CHECKLIST_H

#include "../simtypes.h"


class memory_rw_t;


#define CHK_RANDS 32
#define CHK_DEBUG_SUMS 10

struct checklist_t
{
	uint32 ss;
	uint32 st;
	uint8 nfc;
	uint32 random_seed;
	uint16 halt_entry;
	uint16 line_entry;
	uint16 convoy_entry;

	uint32 rand[CHK_RANDS];
	uint32 debug_sum[CHK_DEBUG_SUMS];


	checklist_t(uint32 _ss, uint32 _st, uint8 _nfc, uint32 _random_seed, uint16 _halt_entry, uint16 _line_entry, uint16 _convoy_entry, uint32 *_rands, uint32 *_debug_sums);
	checklist_t() : ss(0), st(0), nfc(0), random_seed(0), halt_entry(0), line_entry(0), convoy_entry(0)
	{
		for(  uint8 i = 0;  i < CHK_RANDS;  i++  ) {
			rand[i] = 0;
		}
		for(  uint8 i = 0;  i < CHK_DEBUG_SUMS;  i++  ) {
			debug_sum[i] = 0;
		}
	}

	bool operator == (const checklist_t &other) const
	{
		bool rands_equal = true;
		for(  uint8 i = 0;  i < CHK_RANDS  &&  rands_equal;  i++  ) {
			rands_equal = rands_equal  &&  rand[i] == other.rand[i];
		}
		bool debugs_equal = true;
		for(  uint8 i = 0;  i < CHK_DEBUG_SUMS  &&  debugs_equal;  i++  ) {
			// If debug sums are too expensive, then this test below would allow them to be switched off independently at either end:
			// debugs_equal = debugs_equal  &&  (debug_sum[i] == 0  ||  other.debug_sum[i] == 0  ||  debug_sum[i] == other.debug_sum[i]);
			debugs_equal = debugs_equal  &&  debug_sum[i] == other.debug_sum[i];
		}
		return ( rands_equal &&
			debugs_equal &&
			ss == other.ss &&
			st == other.st &&
			nfc == other.nfc &&
			random_seed == other.random_seed &&
			halt_entry == other.halt_entry &&
			line_entry == other.line_entry &&
			convoy_entry == other.convoy_entry
			);
	}
	bool operator != (const checklist_t &other) const { return !( (*this)==other ); }

	void rdwr(memory_rw_t *buffer);
	int print(char *buffer, const char *entity) const;
};


#endif
