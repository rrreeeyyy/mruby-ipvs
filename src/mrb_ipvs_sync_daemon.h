#ifndef MRB_IPVS_SYNC_DAEMON_H
#define MRB_IPVS_SYNC_DAEMON_H

#define DEF_MCAST_IFN "eth0"
#define DEF_DAEMON_STATE "backup"

extern const struct mrb_data_type mrb_ipvs_sync_daemon_type;

struct mrb_ipvs_sync_daemon {
  ipvs_daemon_t daemon;
};

#endif
