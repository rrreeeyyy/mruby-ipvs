# mruby-ipvs

[![wercker status](https://app.wercker.com/status/ebc274270b5be552c38d235d307e85aa/s "wercker status")](https://app.wercker.com/project/bykey/ebc274270b5be552c38d235d307e85aa)

mruby-ipvs is an interface to the [IP Virtual Server(IPVS)](http://www.linuxvirtualserver.org/software/ipvs.html) for [mruby](https://github.com/mruby/mruby/).

## Installation

* Install dependencies.

```bash
apt-get -y install libnl-dev # or yum -y install libnl-devel, and so on...
```

* Clone mruby

```bash
git clone https://github.com/mruby/mruby/ /usr/local/src/mruby
```

* Set up `build_config.rb`

Use [`mgem`](https://github.com/mruby/mgem-list) (recommended)

```
mgem add mruby-ipvs

# and add some mgems if you want
# mgem add ...

mgem config default > /usr/local/src/mruby/build_config.rb # generate build_config.rb
```

or add `conf.gem` line to `build_config.rb` (`/usr/local/src/mruby/build_config.rb`).

```ruby
MRuby::Build.new do |conf|
  # ... (snip) ...
  conf.gem :git => 'https://github.com/rrreeeyyy/mruby-ipvs', :options => '--recursive'
end
```

* Build mruby

```bash
cd /usr/local/src/mruby && ruby ./minirake
```

* Check the instllation

```bash
/usr/local/src/mruby/build/host/bin/mruby -e 'p IPVS'
IPVS
```

## Examples

* Add service.

```ruby
# Create IPVS::Service instance.
s = IPVS::Service.new({
  'addr' => '10.0.0.1',
  'port' => 80,
  'sched_name' => 'wrr'
})

# apply to IPVS.
s.add_service
```

* Add destination.

```ruby
# Create IPVS::Dest instance.
d1 = IPVS::Dest.new({
  'addr' => '192.168.0.1',
  'port' => 80,
  'weight' => 1
})

d2 = IPVS::Dest.new({
  'addr' => '192.168.0.2',
  'port' => 80,
  'weight' => 1,
  'conn' => 'DR'
})

# Add destination to IPVS::Service instance.
s.add_dest(d1)
s.add_dest(d2)

# Change the destination weight, connection flag.
d1.weight = 3
d2.conn = 'TUN'
```

* Sync daemon start/stop

```ruby
# Create IPVS::Daemon instance.
daemon = IPVS::Daemon.new({
  'state' => 'master',            # "master" or "backup"
  'ifname' => 'eth0',
  'syncid' => 1
})
daemon.start()                    # you can daemon.stop() after start.
```

* Check the results

```bash
$ cat /proc/net/ip_vs
IP Virtual Server version 1.2.1 (size=4096)
Prot LocalAddress:Port Scheduler Flags
  -> RemoteAddress:Port Forward Weight ActiveConn InActConn
TCP  0A000001:0050 wrr
  -> C0A80002:0050      Tunnel  1      0          0
  -> C0A80001:0050      Masq    3      0          0
```

* Check the sync daemon multicast group

```
# netstat -anu | grep 224.0.0.81:8848
udp        0      0 10.0.2.15:50758         224.0.0.81:8848         ESTABLISHED
```

* more examples in `examples/`.
    * [examples/keepalived.rb](./examples/keepalived.rb): DSL like [keepalived](https://github.com/acassen/keepalived/commits/master).

## Docker

You can run mruby-ipvs with docker.

- Build docker image:

```
docker build -t mruby-ipvs .
```

- In docker host machine, load `ip_vs` kernel module.

```
modprobe ip_vs
```

or if you use `docker-machine`, just type:

```
docker-machine ssh ${DOCKER_MACHINE_NAME:-default} sudo modprobe ip_vs
```

- Then just docker run:

```
docker run -d mruby-ipvs ${MRUBY_FILE}
```

## Development

Local Source Testing (Requirements: [Vagrant](https://www.vagrantup.com/))

```bash
$ git clone https://github.com/rrreeeyyy/mruby-ipvs/
$ cd mruby-ipvs
$ git submodule update --init
$ vagrant up
$ vagrant ssh -c 'cd /usr/local/src/mruby && rake test'
```

## More Information

* [in Japanese] http://rrreeeyyy.com/blog/2014/03/17/mruby-ipvs/ (blog)
* [in Japanese] https://speakerdeck.com/rrreeeyyy/mruby-ipvs (presentation)
* [in Japanese] http://rrreeeyyy.com/thesis/thesis.pdf (graduation thesis)

## Contributing

1. Fork it ( https://github.com/rrreeeyyy/mruby-ipvs/fork )
2. Create your feature branch (`git checkout -b my-new-feature`)
3. Commit your changes (`git commit -am 'Add some feature'`)
4. Push to the branch (`git push origin my-new-feature`)
5. Create a new Pull Request

## License

GNU General Public License Version 2
