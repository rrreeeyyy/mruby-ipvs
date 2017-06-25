#ifndef MRB_IPVS_H
#define MRB_IPVS_H

#include <mruby.h>
#include <mruby/variable.h>
#include <mruby/string.h>
#include <mruby/hash.h>
#include <mruby/array.h>
#include <mruby/data.h>
#include <mruby/class.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <ip_vs.h>
#include <libipvs.h>
#include <string.h>
#include <malloc.h>
#include <netdb.h>
#include <errno.h>
#include <ctype.h>
#include <limits.h>

int str_is_digit(const char *str);
int host_to_addr(const char *name, struct in_addr *addr);

#define FMT_NUMERIC		0x0001
#define RARRAY_LEN(a) (RARRAY(a)->len)
#endif
