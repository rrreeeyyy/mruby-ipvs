#ifndef MRB_IPVS_DEST_H
#define MRB_IPVS_DEST_H

#define DEST_NONE 0x0000
#define DEST_ADDR 0x0001
#define DEST_PORT 0x0002

#define DEF_WEIGHT 1
#define DEF_CONN_FLAGS "NAT"

extern const struct mrb_data_type mrb_ipvs_dest_type;

struct mrb_ipvs_dest {
  ipvs_dest_t dest;
  ipvs_dest_entry_t ent;
};

#endif
