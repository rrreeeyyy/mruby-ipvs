#include "mrb_ipvs.h"
#include "mrb_ipvs_dest.h"
#include "mrb_ipvs_service.h"

const struct mrb_data_type mrb_ipvs_service_type = {"Service", mrb_free};

static int service_to_port(const char *name, unsigned short proto) {
  struct servent *service;

  if (proto == IPPROTO_TCP && (service = getservbyname(name, "tcp")) != NULL)
    return ntohs((unsigned short)service->s_port);
  else if (proto == IPPROTO_UDP &&
           (service = getservbyname(name, "udp")) != NULL)
    return ntohs((unsigned short)service->s_port);
  else
    return -1;
}

static int parse_service(char *buf, ipvs_service_t *svc) {
  char *portp = NULL;
  long portn;
  int result = SERVICE_NONE;
  struct in_addr inaddr;
  struct in6_addr inaddr6;

  if (buf == NULL || str_is_digit(buf))
    return SERVICE_NONE;
  if (buf[0] == '[') {
    buf++;
    portp = strchr(buf, ']');
    if (portp == NULL)
      return SERVICE_NONE;
    *portp = '\0';
    portp++;
    if (*portp == ':')
      *portp = '\0';
    else
      return SERVICE_NONE;
  }
  if (inet_pton(AF_INET6, buf, &inaddr6) > 0) {
    svc->addr.in6 = inaddr6;
    svc->af = AF_INET6;
    svc->netmask = 128;
  } else {
    portp = strrchr(buf, ':');
    if (portp != NULL)
      *portp = '\0';

    if (inet_aton(buf, &inaddr) != 0) {
      svc->addr.ip = inaddr.s_addr;
      svc->af = AF_INET;
    } else if (host_to_addr(buf, &inaddr) != -1) {
      svc->addr.ip = inaddr.s_addr;
      svc->af = AF_INET;
    } else
      return SERVICE_NONE;
  }

  result |= SERVICE_ADDR;

  if (portp != NULL) {
    result |= SERVICE_PORT;

    if ((portn = string_to_number(portp + 1, 0, 65535)) != -1)
      svc->port = htons(portn);
    else if ((portn = service_to_port(portp + 1, svc->protocol)) != -1)
      svc->port = htons(portn);
    else
      return SERVICE_NONE;
  }

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

  ie->svc.protocol = parse_proto((char *)RSTRING_PTR(proto));

  parse = parse_service((char *)RSTRING_PTR(addr), &ie->svc);
  if (strrchr((char *)RSTRING_PTR(addr), ':') == NULL) {
    ie->svc.port = htons(port);
  }

  if (!(parse & SERVICE_ADDR)) {
    mrb_raise(mrb, E_ARGUMENT_ERROR, "invalid argument");
  }

  strncpy(ie->svc.sched_name, (char *)RSTRING_PTR(sched_name),
          IP_VS_SCHEDNAME_MAXLEN);

  mrb_data_init(self, ie, &mrb_ipvs_service_type);

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
  return mrb_nil_value();
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
  return mrb_nil_value();
}

void mrb_ipvs_service_class_init(mrb_state *mrb, struct RClass *_class_ipvs) {
  struct RClass *_class_ipvs_service;

  _class_ipvs_service =
      mrb_define_class_under(mrb, _class_ipvs, "Service", mrb->object_class);
  MRB_SET_INSTANCE_TT(_class_ipvs_service, MRB_TT_DATA);
  mrb_define_method(mrb, _class_ipvs_service, "initialize",
                    mrb_ipvs_service_init, ARGS_REQ(1));
  mrb_define_method(mrb, _class_ipvs_service, "initialize_copy",
                    mrb_ipvs_service_init_copy, ARGS_REQ(1) | ARGS_OPT(6));
  mrb_define_method(mrb, _class_ipvs_service, "add_service",
                    mrb_ipvs_service_add, ARGS_NONE());
  mrb_define_method(mrb, _class_ipvs_service, "del_service",
                    mrb_ipvs_service_del, ARGS_NONE());
  mrb_define_method(mrb, _class_ipvs_service, "add_dest",
                    mrb_ipvs_service_add_dest, ARGS_REQ(1));
  mrb_define_method(mrb, _class_ipvs_service, "del_dest",
                    mrb_ipvs_service_del_dest, ARGS_REQ(1));
  mrb_define_method(mrb, _class_ipvs_service, "addr", mrb_ipvs_service_get_addr,
                    ARGS_NONE());
  mrb_define_method(mrb, _class_ipvs_service, "port", mrb_ipvs_service_get_port,
                    ARGS_NONE());
  mrb_define_method(mrb, _class_ipvs_service, "proto",
                    mrb_ipvs_service_get_proto, ARGS_NONE());
  mrb_define_method(mrb, _class_ipvs_service, "sched_name",
                    mrb_ipvs_service_get_sched_name, ARGS_NONE());
  //  mrb_define_method(mrb, _class_ipvs_service, "timeout",
  //  mrb_ipvs_service_get_timeout, ARGS_NONE());
  //  mrb_define_method(mrb, _class_ipvs_service, "netmask",
  //  mrb_ipvs_service_get_netmask, ARGS_NONE());
  //  mrb_define_method(mrb, _class_ipvs_service, "ops",
  //  mrb_ipvs_service_get_ops, ARGS_NONE());
}
