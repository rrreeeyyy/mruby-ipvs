#include "mrb_ipvs.h"
#include "mrb_ipvs_service.h"

void mrb_ipvs_dest_class_init(mrb_state *mrb, struct RClass *_class_ipvs);
void mrb_ipvs_service_class_init(mrb_state *mrb, struct RClass *_class_ipvs);
void mrb_ipvs_sync_daemon_class_init(mrb_state *mrb,
                                     struct RClass *_class_ipvs);

int str_is_digit(const char *str) {
  size_t offset;
  size_t top;

  top = strlen(str);
  for (offset = 0; offset < top; offset++) {
    if (!isdigit((int)*(str + offset))) {
      break;
    }
  }

  return (offset < top) ? 0 : 1;
}

int host_to_addr(const char *name, struct in_addr *addr) {
  struct hostent *host;

  if ((host = gethostbyname(name)) != NULL) {
    if (host->h_addrtype != AF_INET || host->h_length != sizeof(struct in_addr))
      return -1;
    /* warning: we just handle h_addr_list[0] here */
    memcpy(addr, host->h_addr_list[0], sizeof(struct in_addr));
    return 0;
  }
  return -1;
}

static int _modprobe_ipvs(void) {
  char const *argv[] = {"/sbin/modprobe", "--", "ip_vs", NULL};
  int child;
  int status;
  int rc;

  if (!(child = fork())) {
    execv(argv[0], argv);
    exit(1);
  }

  rc = waitpid(child, &status, 0);

  if (rc == -1 || !WIFEXITED(status) || WEXITSTATUS(status)) {
    return 1;
  }
  return 0;
}

struct ip_vs_get_services* mrb_ipvs_get_services(mrb_state *mrb) {
  struct ip_vs_get_services *get;

  if (ipvs_getinfo() == -1) {
    mrb_raise(mrb, E_RUNTIME_ERROR, "Can't update ipvsinfo.");
  }

  if (!(get = ipvs_get_services())) {
    mrb_raisef(mrb, E_RUNTIME_ERROR, "%s", ipvs_strerror(errno));
  }
  return get;
}

static mrb_value mrb_ipvs_services(mrb_state *mrb, mrb_value self)
{
  struct ip_vs_get_services *get;
  char pbuf[INET6_ADDRSTRLEN];
  int i;
  ipvs_service_entry_t *se;
  mrb_value services, service, h;

  if (ipvs_getinfo() == -1) {
    mrb_raise(mrb, E_RUNTIME_ERROR, "Can't update ipvsinfo.");
  }

  get = mrb_ipvs_get_services(mrb);
  ipvs_sort_services(get, ipvs_cmp_services);
  services = mrb_ary_new(mrb);

  for (i = 0; i < get->num_services; i++) {
    h = mrb_hash_new(mrb);

    se = &get->entrytable[i];
    inet_ntop(se->af, &(se->addr), pbuf, sizeof(pbuf));

    mrb_hash_set(mrb, h, mrb_str_new_cstr(mrb, "protocol"),
                 mrb_str_new_cstr(mrb, se->protocol == IPPROTO_TCP ? "TCP" : "UDP"));
    mrb_hash_set(mrb, h, mrb_str_new_cstr(mrb, "addr"), mrb_str_new_cstr(mrb, pbuf));
    mrb_hash_set(mrb, h, mrb_str_new_cstr(mrb, "port"), mrb_fixnum_value(ntohs(se->port)));
    mrb_hash_set(mrb, h, mrb_str_new_cstr(mrb, "sched_name"),
                 mrb_str_new_cstr(mrb, se->sched_name));
    service = mrb_obj_new(mrb, mrb_class_get_under(mrb, mrb_class_get(mrb, "IPVS"), "Service"), 1, &h);
    mrb_update_service_dests(mrb, service, get);
    mrb_ary_push(mrb, services, service);
  }

  mrb_free(mrb, get);
  return services;
}


void mrb_mruby_ipvs_gem_init(mrb_state *mrb) {
  struct RClass *_class_ipvs;

  /* Initialize IPVS module */
  if (ipvs_init()) {
    if (_modprobe_ipvs() || ipvs_init()) {
      mrb_raise(mrb, E_RUNTIME_ERROR,
                "Can't initialize ipvs.\n"
                "Are you sure that IP Virtual Server is \n"
                "built in the kernel or as module?\n");
    }
  }

  _class_ipvs = mrb_define_class(mrb, "IPVS", mrb->object_class);
  mrb_ipvs_service_class_init(mrb, _class_ipvs);
  mrb_ipvs_dest_class_init(mrb, _class_ipvs);
  mrb_define_class_method(mrb, _class_ipvs, "services",
                    mrb_ipvs_services, MRB_ARGS_NONE());
  mrb_ipvs_sync_daemon_class_init(mrb, _class_ipvs);
}

void mrb_mruby_ipvs_gem_final(mrb_state *mrb) { ipvs_close(); }
