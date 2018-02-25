#include "mrb_ipvs.h"
#include "mrb_ipvs_dest.h"
#include "mrb_ipvs_service.h"

const struct mrb_data_type mrb_ipvs_service_type = {"Service", mrb_free};

static int parse_service(char *buf, ipvs_service_t *svc) {
  char *strp = NULL;
  int result = SERVICE_NONE;
  struct in_addr inaddr;
  struct in6_addr inaddr6;

  if (buf == NULL || str_is_digit(buf))
    return SERVICE_NONE;
  if (buf[0] == '[') {
    buf++;
    strp = strchr(buf, ']');
    if (strp == NULL)
      return SERVICE_NONE;
    *strp = '\0';
  }
  if (inet_pton(AF_INET6, buf, &inaddr6) > 0) {
    svc->addr.in6 = inaddr6;
    svc->af = AF_INET6;
    svc->netmask = 128;
  } else if (inet_aton(buf, &inaddr) != 0) {
    svc->addr.ip = inaddr.s_addr;
    svc->af = AF_INET;
  } else if (host_to_addr(buf, &inaddr) != -1) {
    svc->addr.ip = inaddr.s_addr;
    svc->af = AF_INET;
  } else
    return SERVICE_NONE;

  result |= SERVICE_ADDR;
  return result;
}

static int parse_proto(const char *proto) {
  if (strcmp(proto, "udp") == 0 || strcmp(proto, "UDP") == 0)
    return IPPROTO_UDP;
  else if (strcmp(proto, "tcp") == 0 || strcmp(proto, "TCP") == 0)
    return IPPROTO_TCP;
  else
    return parse_proto(DEF_PROTO);
}

static mrb_value mrb_ipvs_service_init(mrb_state *mrb, mrb_value self) {
  int parse;
  mrb_value arg_opt = mrb_nil_value(), addr = mrb_nil_value(),
            proto = mrb_nil_value(), sched_name = mrb_nil_value(),
            obj = mrb_nil_value();
  // mrb_value arg_opt = mrb_nil_value(), addr = mrb_nil_value(),
  //           proto = mrb_nil_value(), sched_name = mrb_nil_value(),
  //           ops = mrb_nil_value(), obj = mrb_nil_value();
  mrb_int port;
  // mrb_int port, timeout, netmask;
  struct mrb_ipvs_service *ie;

  ie = (struct mrb_ipvs_service *)mrb_malloc(mrb, sizeof(*ie));
  memset(ie, 0, sizeof(struct mrb_ipvs_service));

  mrb_get_args(mrb, "H", &arg_opt);

  if (mrb_nil_p(arg_opt)) {
    mrb_raise(mrb, E_ARGUMENT_ERROR, "invalid argument");
  }

  addr = mrb_hash_get(mrb, arg_opt, mrb_str_new_cstr(mrb, "addr"));
  if (mrb_nil_p(addr)) {
    mrb_raise(mrb, E_ARGUMENT_ERROR, "invalid argument");
  }

  obj = mrb_hash_get(mrb, arg_opt, mrb_str_new_cstr(mrb, "port"));
  port = mrb_nil_p(obj) ? 0 : mrb_fixnum(obj);
  if (port < 0 || port > 65535) {
    mrb_raise(mrb, E_ARGUMENT_ERROR, "invalid port value specified");
  }

  proto = mrb_hash_get(mrb, arg_opt, mrb_str_new_cstr(mrb, "protocol"));
  if (mrb_nil_p(proto)) {
    proto = mrb_str_new_cstr(mrb, DEF_PROTO);
  }

  sched_name = mrb_hash_get(mrb, arg_opt, mrb_str_new_cstr(mrb, "sched_name"));
  if (mrb_nil_p(sched_name)) {
    sched_name = mrb_str_new_cstr(mrb, DEF_SCHED);
  }

  // ops = mrb_hash_get(mrb, arg_opt, mrb_str_new_cstr(mrb, "ops"));
  // timeout =
  //     mrb_fixnum(mrb_hash_get(mrb, arg_opt, mrb_str_new_cstr(mrb,
  //     "timeout")));
  // netmask =
  //     mrb_fixnum(mrb_hash_get(mrb, arg_opt, mrb_str_new_cstr(mrb,
  //     "netmask")));

  parse = parse_service((char *)RSTRING_PTR(addr), &ie->svc);

  ie->svc.protocol = parse_proto((char *)RSTRING_PTR(proto));
  ie->svc.port = htons(port);

  if (!(parse & SERVICE_ADDR)) {
    mrb_raise(mrb, E_ARGUMENT_ERROR, "invalid addr value specified");
  }

  strncpy(ie->svc.sched_name, (char *)RSTRING_PTR(sched_name),
          IP_VS_SCHEDNAME_MAXLEN);

  mrb_data_init(self, ie, &mrb_ipvs_service_type);
  mrb_update_service_dests(mrb, self);

  return self;
}

static mrb_value mrb_ipvs_service_init_copy(mrb_state *mrb, mrb_value copy) {
  mrb_value src;

  mrb_get_args(mrb, "o", &src);
  if (mrb_obj_equal(mrb, copy, src)) {
    return copy;
  }
  if (!mrb_obj_is_instance_of(mrb, src, mrb_obj_class(mrb, copy))) {
    mrb_raise(mrb, E_TYPE_ERROR, "wrong argument class");
  }
  if (!DATA_PTR(copy)) {
    DATA_PTR(copy) = (struct mrb_ipvs_service *)mrb_malloc(
        mrb, sizeof(struct mrb_ipvs_service));
    DATA_TYPE(copy) = &mrb_ipvs_service_type;
  }
  *(struct mrb_ipvs_service *)DATA_PTR(copy) =
      *(struct mrb_ipvs_service *)DATA_PTR(src);
  return copy;
}

static mrb_value mrb_ipvs_service_get_addr(mrb_state *mrb, mrb_value self) {
  struct mrb_ipvs_service *ie;
  char pbuf[INET6_ADDRSTRLEN];
  ie = DATA_PTR(self);
  inet_ntop(ie->svc.af, &ie->svc.addr.ip, pbuf, sizeof(pbuf));
  return mrb_str_new_cstr(mrb, pbuf);
}

static mrb_value mrb_ipvs_service_get_port(mrb_state *mrb, mrb_value self) {
  struct mrb_ipvs_service *ie;
  ie = DATA_PTR(self);
  return mrb_fixnum_value(ntohs(ie->svc.port));
}

static mrb_value mrb_ipvs_service_get_proto(mrb_state *mrb, mrb_value self) {
  struct mrb_ipvs_service *ie;
  ie = DATA_PTR(self);
  return mrb_str_new_cstr(mrb, ie->svc.protocol == IPPROTO_TCP ? "TCP" : "UDP");
}

static mrb_value mrb_ipvs_service_get_sched_name(mrb_state *mrb,
                                                 mrb_value self) {
  struct mrb_ipvs_service *ie;
  ie = DATA_PTR(self);
  return mrb_str_new_cstr(mrb, ie->svc.sched_name);
}

static mrb_value mrb_ipvs_service_add(mrb_state *mrb, mrb_value self) {
  errno = 0;
  ipvs_add_service(DATA_PTR(self));
  if (errno) {
    mrb_raise(mrb, E_RUNTIME_ERROR, ipvs_strerror(errno));
  }
  return self;
}

static mrb_value mrb_ipvs_service_del(mrb_state *mrb, mrb_value self) {
  ipvs_del_service(DATA_PTR(self));
  return mrb_nil_value();
}

static mrb_value mrb_ipvs_service_add_dest(mrb_state *mrb, mrb_value self) {
  struct mrb_ipvs_dest *ie;
  mrb_value arg;
  mrb_get_args(mrb, "o", &arg);
  if (!(DATA_TYPE(arg) == &mrb_ipvs_dest_type)) {
    mrb_raise(mrb, E_ARGUMENT_ERROR, "invalid argument");
  }
  ie = DATA_PTR(arg);
  mrb_iv_set(mrb, arg, mrb_intern_lit(mrb, "@service"), self);
  ipvs_add_dest(DATA_PTR(self), &ie->dest);
  return mrb_update_service_dests(mrb, self);
}

static mrb_value mrb_ipvs_service_del_dest(mrb_state *mrb, mrb_value self) {
  struct mrb_ipvs_dest *ie;
  mrb_value arg;
  mrb_get_args(mrb, "o", &arg);
  if (!(DATA_TYPE(arg) == &mrb_ipvs_dest_type)) {
    mrb_raise(mrb, E_ARGUMENT_ERROR, "invalid argument");
  }
  ie = DATA_PTR(arg);
  ipvs_del_dest(DATA_PTR(self), &ie->dest);
  return mrb_update_service_dests(mrb, self);
}

static inline const char *fwd_name(unsigned flags)
{
  const char *fwd = NULL;

  switch (flags & IP_VS_CONN_F_FWD_MASK) {
  case IP_VS_CONN_F_MASQ:
    fwd = "NAT";
    break;
  case IP_VS_CONN_F_LOCALNODE:
    fwd = "LOCAL";
    break;
  case IP_VS_CONN_F_TUNNEL:
    fwd = "TUN";
    break;
  case IP_VS_CONN_F_DROUTE:
    fwd = "DR";
    break;
  }
  return fwd;
}

mrb_value mrb_update_service_dests(mrb_state *mrb, mrb_value self)
{
  struct mrb_ipvs_service *ie;
  ipvs_service_entry_t *se;
  struct ip_vs_get_dests *d;
  mrb_value dest, dests, h;
  ipvs_dest_entry_t *e;
  char pbuf[INET6_ADDRSTRLEN];
  int i;

  ie = DATA_PTR(self);
  dests = mrb_ary_new(mrb);

  se = ipvs_get_service(ie->svc.fwmark, ie->svc.af, ie->svc.protocol, ie->svc.addr, ie->svc.port);
  if(se && se->num_dests > 0) {

    if (!(d = ipvs_get_dests(se))) {
      mrb_raisef(mrb, E_RUNTIME_ERROR, "%s", ipvs_strerror(errno));
    }
    ipvs_sort_dests(d, ipvs_cmp_dests);

    for (i = 0; i < d->num_dests; i++) {
      h = mrb_hash_new(mrb);
      e = &d->entrytable[i];

      inet_ntop(e->af, &(e->addr), pbuf, sizeof(pbuf));
      mrb_hash_set(mrb, h, mrb_str_new_cstr(mrb, "addr"), mrb_str_new_cstr(mrb, pbuf));
      mrb_hash_set(mrb, h, mrb_str_new_cstr(mrb, "port"), mrb_fixnum_value(ntohs(e->port)));
      mrb_hash_set(mrb, h, mrb_str_new_cstr(mrb, "weight"), mrb_fixnum_value(e->weight));
      mrb_hash_set(mrb, h, mrb_str_new_cstr(mrb, "conn"), mrb_str_new_cstr(mrb, fwd_name(e->conn_flags)));
      dest = mrb_obj_new(mrb, mrb_class_get_under(mrb, mrb_class_get(mrb, "IPVS"), "Dest"), 1, &h);
      mrb_ary_push(mrb, dests, dest);
    }
    mrb_free(mrb, d);
  }
  mrb_iv_set(mrb, self, mrb_intern_lit(mrb, "@dests"), dests);

  return mrb_nil_value();
}

void mrb_ipvs_service_class_init(mrb_state *mrb, struct RClass *_class_ipvs) {
  struct RClass *_class_ipvs_service;

  _class_ipvs_service =
      mrb_define_class_under(mrb, _class_ipvs, "Service", mrb->object_class);
  MRB_SET_INSTANCE_TT(_class_ipvs_service, MRB_TT_DATA);
  mrb_define_method(mrb, _class_ipvs_service, "initialize",
                    mrb_ipvs_service_init, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, _class_ipvs_service, "initialize_copy",
                    mrb_ipvs_service_init_copy,
                    MRB_ARGS_REQ(1) | MRB_ARGS_OPT(6));
  mrb_define_method(mrb, _class_ipvs_service, "add_service",
                    mrb_ipvs_service_add, MRB_ARGS_NONE());
  mrb_define_method(mrb, _class_ipvs_service, "del_service",
                    mrb_ipvs_service_del, MRB_ARGS_NONE());
  mrb_define_method(mrb, _class_ipvs_service, "add_dest",
                    mrb_ipvs_service_add_dest, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, _class_ipvs_service, "del_dest",
                    mrb_ipvs_service_del_dest, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, _class_ipvs_service, "addr", mrb_ipvs_service_get_addr,
                    MRB_ARGS_NONE());
  mrb_define_method(mrb, _class_ipvs_service, "port", mrb_ipvs_service_get_port,
                    MRB_ARGS_NONE());
  mrb_define_method(mrb, _class_ipvs_service, "proto",
                    mrb_ipvs_service_get_proto, MRB_ARGS_NONE());
  mrb_define_method(mrb, _class_ipvs_service, "sched_name",
                    mrb_ipvs_service_get_sched_name, MRB_ARGS_NONE());
  //  mrb_define_method(mrb, _class_ipvs_service, "timeout",
  //  mrb_ipvs_service_get_timeout, MRB_ARGS_NONE());
  //  mrb_define_method(mrb, _class_ipvs_service, "netmask",
  //  mrb_ipvs_service_get_netmask, MRB_ARGS_NONE());
  //  mrb_define_method(mrb, _class_ipvs_service, "ops",
  //  mrb_ipvs_service_get_ops, MRB_ARGS_NONE());
}
