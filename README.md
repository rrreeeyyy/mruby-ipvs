# mruby-ipvs

[![Gitter](https://badges.gitter.im/Join Chat.svg)](https://gitter.im/rrreeeyyy/mruby-ipvs?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge)

mruby-ipvs is an interface to the [IP Virtual Server(IPVS)](http://www.linuxvirtualserver.org/software/ipvs.html) for [mruby](https://github.com/mruby/mruby/).

## Installation

* Install dependencies.

```
apt-get -y install libnl-dev # or yum -y install libnl-devel, and so on...
```

* clone mruby

```
git clone https://github.com/mruby/mruby/ /usr/local/src/mruby
```

* add `conf.gem` line to `build_config.rb` (`/usr/local/src/mruby/build_config.rb`).

```
MRuby::Build.new do |conf|
  # ... (snip) ...
  conf.gem :git => 'https://github.com/rrreeeyyy/mruby-ipvs'
end
```

* build mruby

```
cd /usr/local/src/mruby && ruby ./minirake
```

* check the instllation

```
/usr/local/src/mruby/build/host/bin/mruby -e 'p IPVS'
IPVS
```

## Example

* add service.

```
# Create IPVS::Service instance.
s = IPVS::Service.new({
  'addr' => '10.0.0.1:80',
  'port' => 80,
  'sched_name' => 'wrr'
})

# apply to IPVS.
s.add_service
```

* add destination.

```
# Create IPVS::Dest instance.
d1 = IPVS::Dest.new({
  'addr' => '192.168.0.1',
  'port' => 80,
  'weight' => 1
})

d2 = IPVS::Dest.new({
  'addr' => '192.168.0.2',
  'port' => 80,
  'weight' => 1
})

# Add destination to IPVS::Service instance.
s.add_dest(d1)
s.add_dest(d2)

# Change the destination weight.
d1.weight = 3
```

* check the results

```
$ cat /proc/net/ip_vs
IP Virtual Server version 1.2.1 (size=4096)
Prot LocalAddress:Port Scheduler Flags
  -> RemoteAddress:Port Forward Weight ActiveConn InActConn
TCP  0A000001:0050 wrr
  -> C0A80002:0050      Masq    1      0          0
  -> C0A80001:0050      Masq    3      0          0
```

* more examples in `examples/`.
    * `examples/keepalived.rb`: [keepalived](https://github.com/acassen/keepalived/commits/master) like DSL.

## Development

Local Source Testing (Requirements: [Vagrant](https://www.vagrantup.com/))

```
$ git clone https://github.com/rrreeeyyy/mruby-ipvs/
$ cd mruby-ipvs
$ vagrant up
$ vagrant ssh
```

```
$ sudo su -
# cp -a /vagrant/.vagrant_default.gembox /usr/local/src/mruby/mrbgems/
# cd /usr/local/src/mruby/ && rake test
```

# More Information

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
