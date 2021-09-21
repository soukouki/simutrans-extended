/*
 * This file is part of the Simutrans-Extended project under the Artistic License.
 * (see LICENSE.txt)
 */

#include "checklist.h"

#include "../network/memory_rw.h"


checklist_t::checklist_t(uint32 _ss, uint32 _st, uint8 _nfc, uint32 _random_seed, uint16 _halt_entry, uint16 _line_entry, uint16 _convoy_entry, uint32 *_rands, uint32 *_debug_sums)
	: ss(_ss), st(_st), nfc(_nfc), random_seed(_random_seed), halt_entry(_halt_entry), line_entry(_line_entry), convoy_entry(_convoy_entry)
{
	for(  uint8 i = 0;  i < CHK_RANDS; i++  ) {
		rand[i]	 = _rands[i];
	}
	for(  uint8 i = 0;  i < CHK_DEBUG_SUMS; i++  ) {
		debug_sum[i]	 = _debug_sums[i];
	}
}


void checklist_t::rdwr(memory_rw_t *buffer)
{
	buffer->rdwr_long(ss);
	buffer->rdwr_long(st);
	buffer->rdwr_byte(nfc);
	buffer->rdwr_long(random_seed);
	buffer->rdwr_short(halt_entry);
	buffer->rdwr_short(line_entry);
	buffer->rdwr_short(convoy_entry);

	// desync debug
	for(  uint8 i = 0;  i < CHK_RANDS;  i++  ) {
		buffer->rdwr_long(rand[i]);
	}
	// More desync debug - should catch desyncs earlier with little computational penalty
	for(  uint8 i = 0;  i < CHK_DEBUG_SUMS;  i++  ) {
		buffer->rdwr_long(debug_sum[i]);
	}
}


int checklist_t::print(char *buffer, const char *entity) const
{
	return sprintf(buffer, "%s=[ss=%u st=%u nfc=%u rand=%u halt=%u line=%u cnvy=%u\n\tssr=%u,%u,%u,%u,%u,%u,%u,%u\n\tstr=%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u\n\texr=%u,%u,%u,%u,%u,%u,%u,%u\n\tsums=%u,%u,%u,%u,%u,%u,%u,%u,%u,%u]\n",
		entity, ss, st, nfc, random_seed, halt_entry, line_entry, convoy_entry,
		rand[0], rand[1], rand[2], rand[3], rand[4], rand[5], rand[6], rand[7],
		rand[8], rand[9], rand[10], rand[11], rand[12], rand[13], rand[14], rand[15], rand[16], rand[17], rand[18], rand[19], rand[20], rand[21], rand[22], rand[23],
		rand[24], rand[25], rand[26], rand[27], rand[28], rand[29], rand[30], rand[31],
		debug_sum[0], debug_sum[1], debug_sum[2], debug_sum[3], debug_sum[4], debug_sum[5], debug_sum[6], debug_sum[7], debug_sum[8], debug_sum[9]
	);
}
