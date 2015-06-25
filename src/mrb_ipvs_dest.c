#include "mrb_ipvs.h"
#include "mrb_ipvs_dest.h"
#include "mrb_ipvs_service.h"

const struct mrb_data_type mrb_ipvs_dest_type = {"Dest", mrb_free};

static int parse_dest(char *buf, ipvs_dest_t *dest) {
  char *portp = NULL;
  long portn;
  int result = DEST_NONE;
  struct in_addr inaddr;
  struct in6_addr inaddr6;

  if (buf == NULL || str_is_digit(buf))
    return DEST_NONE;
  if (buf[0] == '[') {
    buf++;
    portp = strchr(buf, ']');
    if (portp == NULL)
      return DEST_NONE;
    *portp = '\0';
    portp++;
    if (*portp == ':')
      *portp = '\0';
    else
      return DEST_NONE;
  }
  if (inet_pton(AF_INET6, buf, &inaddr6) > 0) {
    dest->addr.in6 = inaddr6;
    dest->af = AF_INET6;
  } else {
    portp = strrchr(buf, ':');
    if (portp != NULL)
      *portp = '\0';

    if (inet_aton(buf, &inaddr) != 0) {
      dest->addr.ip = inaddr.s_addr;
      dest->af = AF_INET;
    } else if (host_to_addr(buf, &inaddr) != -1) {
      dest->addr.ip = inaddr.s_addr;
      dest->af = AF_INET;
    } else
      return DEST_NONE;
  }

  result |= DEST_ADDR;

  if (portp != NULL) {
    result |= DEST_PORT;

    if ((portn = string_to_number(portp + 1, 0, 65535)) != -1)
      dest->port = htons(portn);
    else
      return DEST_NONE;
  }

  return result;
}

static int parse_conn_flags(const char *conn) {
  if (strcmp(conn, "nat") == 0 || strcmp(conn, "NAT") == 0)
    return IP_VS_CONN_F_MASQ;
  else if (strcmp(conn, "dr") == 0 || strcmp(conn, "DR") == 0)
    return IP_VS_CONN_F_DROUTE;
  else if (strcmp(conn, "tun") == 0 || strcmp(conn, "TUN") == 0)
    return IP_VS_CONN_F_TUNNEL;
  else
    return parse_conn_flags(DEF_CONN_FLAGS);
}

static mrb_value mrb_ipvs_dest_init(mrb_state *mrb, mrb_value self) {
  int parse;
  mrb_value arg_opt = mrb_nil_value(), addr = mrb_nil_value(),
            conn = mrb_nil_value(), obj = mrb_nil_value();
  mrb_int port, weight;
  struct mrb_ipvs_dest *ie;

  ie = (struct mrb_ipvs_dest *)mrb_malloc(mrb, sizeof(*ie));
  memset(ie, 0, sizeof(struct mrb_ipvs_dest));

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

  obj = mrb_hash_get(mrb, arg_opt, mrb_str_new_cstr(mrb, "weight"));
  weight = mrb_nil_p(obj) ? DEF_WEIGHT : mrb_fixnum(obj);
  if (weight < 0 || weight > INT_MAX) {
    mrb_raise(mrb, E_ARGUMENT_ERROR, "invalid weight value specified");
  }

  conn = mrb_hash_get(mrb, arg_opt, mrb_str_new_cstr(mrb, "conn"));
  if (mrb_nil_p(conn)) {
    conn = mrb_str_new_cstr(mrb, DEF_CONN_FLAGS);
  }

  parse = parse_dest((char *)RSTRING_PTR(addr), &ie->dest);
  if (strrchr((char *)RSTRING_PTR(addr), ':') == NULL) {
    ie->dest.port = htons(port);
  }

  if (!(parse & DEST_ADDR)) {
    mrb_raise(mrb, E_ARGUMENT_ERROR, "invalid argument");
  }

  mrb_iv_set(mrb, self, mrb_intern_lit(mrb, "@service"), mrb_nil_value());

  ie->dest.weight = weight;
  ie->dest.conn_flags = parse_conn_flags((char *)RSTRING_PTR(conn));

  mrb_data_init(self, ie, &mrb_ipvs_dest_type);

  return self;
}

static mrb_value mrb_ipvs_dest_init_copy(mrb_state *mrb, mrb_value copy) {
  mrb_value src;

  mrb_get_args(mrb, "o", &src);
  if (mrb_obj_equal(mrb, copy, src)) {
    return copy;
  }
  if (!mrb_obj_is_instance_of(mrb, src, mrb_obj_class(mrb, copy))) {
    mrb_raise(mrb, E_TYPE_ERROR, "wrong argument class");
  }
  if (!DATA_PTR(copy)) {
    DATA_PTR(copy) =
        (struct mrb_ipvs_dest *)mrb_malloc(mrb, sizeof(struct mrb_ipvs_dest));
    DATA_TYPE(copy) = &mrb_ipvs_dest_type;
  }
  *(struct mrb_ipvs_dest *)DATA_PTR(copy) =
      *(struct mrb_ipvs_dest *)DATA_PTR(src);
  return copy;
}

static mrb_value mrb_ipvs_dest_get_addr(mrb_state *mrb, mrb_value self) {
  struct mrb_ipvs_dest *ie;
  char pbuf[INET6_ADDRSTRLEN];
  ie = DATA_PTR(self);
  inet_ntop(ie->dest.af, &ie->dest.addr.ip, pbuf, sizeof(pbuf));
  return mrb_str_new_cstr(mrb, pbuf);
}

static mrb_value mrb_ipvs_dest_get_port(mrb_state *mrb, mrb_value self) {
  struct mrb_ipvs_dest *ie;
  ie = DATA_PTR(self);
  return mrb_fixnum_value(ntohs(ie->dest.port));
}

static mrb_value mrb_ipvs_dest_get_weight(mrb_state *mrb, mrb_value self) {
  struct mrb_ipvs_dest *ie;
  ie = DATA_PTR(self);
  return mrb_fixnum_value(ie->dest.weight);
}

static mrb_value mrb_ipvs_dest_set_weight(mrb_state *mrb, mrb_value self) {
  struct mrb_ipvs_service *svc;
  struct mrb_ipvs_dest *dest;
  mrb_int weight;
  dest = DATA_PTR(self);
  mrb_get_args(mrb, "i", &weight);
  dest->dest.weight = weight;
  if (!mrb_nil_p(mrb_iv_get(mrb, self, mrb_intern_lit(mrb, "@service")))) {
    svc = DATA_PTR(mrb_iv_get(mrb, self, mrb_intern_lit(mrb, "@service")));
    ipvs_update_dest(&svc->svc, &dest->dest);
  }
  return mrb_nil_value();
}

static mrb_value mrb_ipvs_dest_get_conn(mrb_state *mrb, mrb_value self) {
  struct mrb_ipvs_dest *ie;
  ie = DATA_PTR(self);
  switch (ie->dest.conn_flags & IP_VS_CONN_F_FWD_MASK) {
  case IP_VS_CONN_F_MASQ:
    return mrb_str_new_cstr(mrb, "NAT");
    break;
  case IP_VS_CONN_F_LOCALNODE:
    return mrb_str_new_cstr(mrb, "LOCAL");
    break;
  case IP_VS_CONN_F_TUNNEL:
    return mrb_str_new_cstr(mrb, "TUN");
    break;
  case IP_VS_CONN_F_DROUTE:
    return mrb_str_new_cstr(mrb, "DR");
    break;
  }
  return mrb_nil_value();
}

static mrb_value mrb_ipvs_dest_set_conn(mrb_state *mrb, mrb_value self) {
  struct mrb_ipvs_service *svc;
  struct mrb_ipvs_dest *dest;
  mrb_value conn;
  dest = DATA_PTR(self);
  mrb_get_args(mrb, "S", &conn);
  dest->dest.conn_flags = parse_conn_flags((char *)RSTRING_PTR(conn));
  if (!mrb_nil_p(mrb_iv_get(mrb, self, mrb_intern_lit(mrb, "@service")))) {
    svc = DATA_PTR(mrb_iv_get(mrb, self, mrb_intern_lit(mrb, "@service")));
    ipvs_update_dest(&svc->svc, &dest->dest);
  }
  return mrb_nil_value();
}

void mrb_ipvs_dest_class_init(mrb_state *mrb, struct RClass *_class_ipvs) {
  struct RClass *_class_ipvs_dest;

  _class_ipvs_dest =
      mrb_define_class_under(mrb, _class_ipvs, "Dest", mrb->object_class);
  MRB_SET_INSTANCE_TT(_class_ipvs_dest, MRB_TT_DATA);
  mrb_define_method(mrb, _class_ipvs_dest, "initialize", mrb_ipvs_dest_init,
                    MRB_ARGS_REQ(1));
  mrb_define_method(mrb, _class_ipvs_dest, "initialize_copy",
                    mrb_ipvs_dest_init_copy, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, _class_ipvs_dest, "addr", mrb_ipvs_dest_get_addr,
                    MRB_ARGS_NONE());
  mrb_define_method(mrb, _class_ipvs_dest, "port", mrb_ipvs_dest_get_port,
                    MRB_ARGS_NONE());
  mrb_define_method(mrb, _class_ipvs_dest, "weight", mrb_ipvs_dest_get_weight,
                    MRB_ARGS_NONE());
  mrb_define_method(mrb, _class_ipvs_dest, "weight=", mrb_ipvs_dest_set_weight,
                    MRB_ARGS_NONE());
  mrb_define_method(mrb, _class_ipvs_dest, "conn", mrb_ipvs_dest_get_conn,
                    MRB_ARGS_NONE());
  mrb_define_method(mrb, _class_ipvs_dest, "conn=", mrb_ipvs_dest_set_conn,
                    MRB_ARGS_NONE());
}
