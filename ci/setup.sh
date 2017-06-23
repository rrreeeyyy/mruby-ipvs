#!/bin/bash
ipvsadm -C
ipvsadm -A -t 127.0.0.1:80 -s rr
ipvsadm -a -t 127.0.0.1:80 -r 127.0.0.1 -w 256
