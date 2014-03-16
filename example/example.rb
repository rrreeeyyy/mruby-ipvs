s = IPVS::Service.new({
  'addr' => '10.0.0.1:80',
  'port' => 80,
  'sched_name' => 'wrr'
}).add_service
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
s.add_dest(d1)
s.add_dest(d2)

d1.weight = 3
