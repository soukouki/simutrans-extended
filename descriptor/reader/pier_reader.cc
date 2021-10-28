/*
 * This file is part of the Simutrans-Extended project under the Artistic License.
 * (see LICENSE.txt)
 */

#include "pier_reader.h"
#include "../../simtypes.h"
#include "../pier_desc.h"
#include "../../bauer/pier_builder.h"
#include "../../network/pakset_info.h"

void pier_reader_t::register_obj(obj_desc_t *&data){
    pier_desc_t *desc = static_cast<pier_desc_t *>(data);
    pier_builder_t::register_desc(desc);

    checksum_t *chk = new checksum_t();
    desc->calc_checksum(chk);
    pakset_info_t::append(desc->get_name(), get_type(),chk);
}

obj_desc_t * pier_reader_t::read_node(FILE *fp, obj_node_info_t &node){
    ALLOCA(char, desc_buf, node.size);

    pier_desc_t *desc = new pier_desc_t();

    fread(desc_buf, node.size, 1, fp);

    char * p = desc_buf;

    //read version (not used yet, only one version)
    uint16 sub_version = decode_uint16(p);
    sub_version &=  0x3FFF;
    sub_version >>= 8;
    desc->price = decode_uint32(p);
    desc->maintenance = decode_uint32(p);
    desc->intro_date = decode_uint16(p);
    desc->retire_date = decode_uint16(p);
    desc->max_weight = decode_uint32(p);
    desc->max_altitude = decode_sint8(p);
    desc->above_way_ribi = decode_uint8(p);
    desc->below_way_ribi = decode_uint8(p);
    desc->base_mask = decode_uint32(p);
    desc->support_mask = decode_uint32(p);
    desc->sub_obj_mask = decode_uint32(p);
    desc->deck_obj_mask = decode_uint32(p);
    desc->auto_group = decode_uint8(p);
    desc->auto_height = decode_uint8(p);
    desc->number_of_seasons=decode_uint8(p);
    desc->above_slope=decode_uint8(p);
    desc->rotational_symmetry=decode_uint8(p);
    desc->middle_mask=decode_uint32(p);
    desc->pier_weight=decode_uint32(p);
    desc->drag_ribi=decode_uint8(p);
    desc->above_way_supplement=decode_uint8(p);

    desc->topspeed = -1;
    desc->axle_load = -1;

    if(sub_version>=2){
        //not used yet
        decode_uint32(p);
        decode_uint32(p);
        decode_uint32(p);

        desc->topspeed = decode_uint16(p);
        desc->axle_load = decode_uint16(p);
    }

    return desc;
}
