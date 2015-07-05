#include "mrb_ipvs.h"
#include "mrb_ipvs_sync_daemon.h"

const struct mrb_data_type mrb_ipvs_sync_daemon_type = {"Daemon", mrb_free};

static int parse_daemon_state(const char *conn) {
  if (strcmp(conn, "master") == 0 || strcmp(conn, "MASTER") == 0)
    return IP_VS_STATE_MASTER;
  else if (strcmp(conn, "backup") == 0 || strcmp(conn, "BACKUP") == 0)
    return IP_VS_STATE_BACKUP;
  else
    return parse_daemon_state(DEF_DAEMON_STATE);
}

static mrb_value mrb_ipvs_sync_daemon_init(mrb_state *mrb, mrb_value self) {
  mrb_value arg_opt = mrb_nil_value(), ifname = mrb_nil_value(),
            state = mrb_nil_value(), obj = mrb_nil_value();
  mrb_int syncid;
  struct mrb_ipvs_sync_daemon *ie;

  ie = (struct mrb_ipvs_sync_daemon *)mrb_malloc(mrb, sizeof(*ie));
  memset(ie, 0, sizeof(struct mrb_ipvs_sync_daemon));

  mrb_get_args(mrb, "H", &arg_opt);

  if (mrb_nil_p(arg_opt)) {
    mrb_raise(mrb, E_ARGUMENT_ERROR, "invalid argument");
  }

  state = mrb_hash_get(mrb, arg_opt, mrb_str_new_cstr(mrb, "state"));
  if (mrb_nil_p(state)) {
    mrb_raise(mrb, E_ARGUMENT_ERROR, "invalid daemon statement");
  }

  ifname = mrb_hash_get(mrb, arg_opt, mrb_str_new_cstr(mrb, "ifname"));
  if (mrb_nil_p(ifname)) {
    ifname = mrb_str_new_cstr(mrb, DEF_MCAST_IFN);
  }

  obj = mrb_hash_get(mrb, arg_opt, mrb_str_new_cstr(mrb, "syncid"));
  syncid = mrb_nil_p(obj) ? 0 : mrb_fixnum(obj);
  if (syncid < 0 || syncid > 256) {
    mrb_raise(mrb, E_ARGUMENT_ERROR, "invalid syncid value specified");
  }

  mrb_iv_set(mrb, self, mrb_intern_lit(mrb, "@daemon"), mrb_nil_value());

  ie->daemon.syncid = syncid;
  strncpy(ie->daemon.mcast_ifn, (char *)RSTRING_PTR(ifname),
          IP_VS_IFNAME_MAXLEN);
  ie->daemon.state = parse_daemon_state((char *)RSTRING_PTR(state));

  mrb_data_init(self, ie, &mrb_ipvs_sync_daemon_type);

  return self;
}

static mrb_value mrb_ipvs_sync_daemon_start(mrb_state *mrb, mrb_value self) {
  errno = 0;
  ipvs_start_daemon(DATA_PTR(self));
  if (errno) {
    mrb_raise(mrb, E_RUNTIME_ERROR, ipvs_strerror(errno));
  }
  return self;
}

static mrb_value mrb_ipvs_sync_daemon_stop(mrb_state *mrb, mrb_value self) {
  errno = 0;
  ipvs_stop_daemon(DATA_PTR(self));
  if (errno) {
    mrb_raise(mrb, E_RUNTIME_ERROR, ipvs_strerror(errno));
  }
  return self;
}

void mrb_ipvs_sync_daemon_class_init(mrb_state *mrb,
                                     struct RClass *_class_ipvs) {
  struct RClass *_class_ipvs_sync_daemon;
  _class_ipvs_sync_daemon = mrb_define_class(mrb, "Daemon", mrb->object_class);
  MRB_SET_INSTANCE_TT(_class_ipvs_sync_daemon, MRB_TT_DATA);
  mrb_define_method(mrb, _class_ipvs_sync_daemon, "initialize",
                    mrb_ipvs_sync_daemon_init, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, _class_ipvs_sync_daemon, "start",
                    mrb_ipvs_sync_daemon_start, MRB_ARGS_NONE());
  mrb_define_method(mrb, _class_ipvs_sync_daemon, "stop",
                    mrb_ipvs_sync_daemon_stop, MRB_ARGS_NONE());
}
