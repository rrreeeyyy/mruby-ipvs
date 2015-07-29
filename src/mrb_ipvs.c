#include "mrb_ipvs.h"

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
  mrb_ipvs_sync_daemon_class_init(mrb, _class_ipvs);
}

void mrb_mruby_ipvs_gem_final(mrb_state *mrb) { ipvs_close(); }
