#ifndef MRB_IPVS_SERVICE_H
#define MRB_IPVS_SERVICE_H

#define SERVICE_NONE 0x0000
#define SERVICE_ADDR 0x0001

#define DEF_PROTO "TCP"
#define DEF_SCHED "wlc"

extern const struct mrb_data_type mrb_ipvs_service_type;
static mrb_value mrb_update_service_dests(mrb_state*, mrb_value);

struct mrb_ipvs_service {
  ipvs_service_t svc;
  ipvs_service_entry_t ent;
};

#endif
