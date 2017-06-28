##
# mruby-ipvs test

assert('define module IPVS') do
  Object.const_defined?(:IPVS)
end

assert('IPVS::Service.new({"addr" => "10.0.0.1"})') do
  ipvs = IPVS::Service.new({"addr" => "10.0.0.1"})
  ipvs.addr == "10.0.0.1"
end

assert('IPVS::Service.new({"addr" => "10.0.0.1", "port" => 80})') do
  ipvs = IPVS::Service.new({"addr" => "10.0.0.1", "port" => 80})
  ipvs.port == 80
end

assert('IPVS::Service.new({"addr" => "10.0.0.1", "proto" => "TCP"})') do
  ipvs = IPVS::Service.new({"addr" => "10.0.0.1", "proto" => "TCP"})
  ipvs.proto == "TCP"
end

assert('IPVS::Service.new({"addr" => "10.0.0.1", "sched_name" => "rr"}') do
  ipvs = IPVS::Service.new({"addr" => "10.0.0.1", "sched_name" => "rr"})
  ipvs.sched_name == "rr"
end

assert('IPVS::Dest.new({"addr" => "10.0.0.1"})') do
  dest = IPVS::Dest.new({"addr" => "10.0.0.1"})
  dest.addr == "10.0.0.1"
end

assert('IPVS::Dest.new({"addr" => "10.0.0.1", "port" => 80})') do
  dest = IPVS::Dest.new({"addr" => "10.0.0.1", "port" => 80})
  dest.port == 80
end

assert('IPVS::Dest.new({"addr" => "10.0.0.1", "weight" => 5}') do
  dest = IPVS::Dest.new({"addr" => "10.0.0.1", "weight" => 5})
  dest.weight == 5
end

assert('d = IPVS::Dest.new({"addr" => "10.0.0.1", "weight" => 5}; d.weight = 3') do
  dest = IPVS::Dest.new({"addr" => "10.0.0.1", "weight" => 5})
  dest.weight = 3 and dest.weight == 3
end

assert('IPVS::Dest.new({"addr" => "10.0.0.1", "weight" => 5, "conn" => "nat"}') do
  dest = IPVS::Dest.new({"addr" => "10.0.0.1", "weight" => 5, "conn" => "nat"})
  dest.conn == "NAT"
end

assert('IPVS::Dest.new({"addr" => "10.0.0.1", "weight" => 5, "conn" => "dr"}') do
  dest = IPVS::Dest.new({"addr" => "10.0.0.1", "weight" => 5, "conn" => "dr"})
  dest.conn == "DR"
end

assert('IPVS::Dest.new({"addr" => "10.0.0.1", "weight" => 5, "conn" => "tun"}') do
  dest = IPVS::Dest.new({"addr" => "10.0.0.1", "weight" => 5, "conn" => "tun"})
  dest.conn == "TUN"
end

assert('d = IPVS::Dest.new({"addr" => "10.0.0.1", "weight" => 5, "conn" => "tun"}; d.conn="NAT"') do
  dest = IPVS::Dest.new({"addr" => "10.0.0.1", "weight" => 5, "conn" => "tun"})
  dest.conn = "NAT" and dest.conn == "NAT"
end

def add_service_with(&block)
  s1 = IPVS::Service.new({"addr" => "127.0.0.1", "port" => 80, "sched_name" => "rr"})
  d1 = IPVS::Dest.new({"addr" => "192.168.0.1", "weight" => 256, "port" => 80, "conn" => "dr"})
  d2 = IPVS::Dest.new({"addr" => "192.168.0.2", "weight" => 256, "port" => 80, "conn" => "dr"})

  s2 = IPVS::Service.new({"addr" => "127.0.0.1", "port" => 443, "sched_name" => "rr"})

  begin
    s1.add_service
    s2.add_service
    s1.add_dest(d1)
    s1.add_dest(d2)
    block.call([s1, s2], [d1, d2])
  ensure
    s1.del_service
    s2.del_service
  end
end

assert('IPVS::Service.inspect') do
  expect = [
    {
      "protocol"=>"TCP",
      "addr"=>"127.0.0.1",
      "port"=>80,
      "sched_name"=>"rr",
      "dests"=> [{
        "addr"=>"192.168.0.2",
          "port"=>80,
          "weight"=>256,
          "conn"=>"DR"
      }]
    }
  ]

  add_service_with do |s,d|
    assert_equal(expect, IPVS::Service.inspect)
  end
end

assert('IPVS.services') do
  add_service_with do |s,d|
    current = IPVS.services
    service = current.first

    assert_equal(2, current.length)
    assert_equal("127.0.0.1", service.addr)
    assert_equal(80, service.port)
    assert_equal("rr", service.sched_name)

    service = current.last

    assert_equal("127.0.0.1", service.addr)
    assert_equal(443, service.port)
    assert_equal("rr", service.sched_name)
  end
end

assert('IPVS::Service#dests') do
  add_service_with do |s,d|
    dests = s[0].dests
    dest = dests.first

    assert_equal(2, dests.length)
    assert_equal("192.168.0.1", dest.addr)
    assert_equal(80, dest.port)
    assert_equal(256, dest.weight)
    assert_equal("DR", dest.conn)

    dest = dests.last
    assert_equal("192.168.0.2", dest.addr)
    assert_equal(80, dest.port)
    assert_equal(256, dest.weight)
    assert_equal("DR", dest.conn)

    dests = s[1].dests
    assert_equal(0, dests.length)
  end
end

assert('IPVS::Service#equal?') do
  add_service_with do |s,d|
    expect_service = IPVS::Service.new({"addr" => "127.0.0.1", "port" => 80, "sched_name" => "rr"})
    ng_service     = IPVS::Service.new({"addr" => "127.0.0.2", "port" => 80, "sched_name" => "rr"})

    assert_true(s[0].equal?(expect_service))
    assert_false(s[0].equal?(ng_service))
  end
end

assert('IPVS::Service#deep_equal?') do
  add_service_with do |s,d|
    expect_service = IPVS::Service.new({"addr" => "127.0.0.1", "port" => 80, "sched_name" => "rr"})
    expect_service.dests = [
      IPVS::Dest.new({"addr" => "192.168.0.1", "weight" => 256, "port" => 80, "conn" => "dr"}),
      IPVS::Dest.new({"addr" => "192.168.0.2", "weight" => 256, "port" => 80, "conn" => "dr"})
    ]

    assert_true(s[0].deep_equal?(expect_service))

    expect_service.dests = [IPVS::Dest.new({"addr" => "192.168.0.3", "weight" => 256, "port" => 80, "conn" => "dr"})]
    assert_false(s[0].deep_equal?(expect_service))
  end
end

assert('IPVS::Dest#equal?') do
  add_service_with do |s,d|
    expect_dest = IPVS::Dest.new({"addr" => "192.168.0.1", "weight" => 256, "port" => 80, "conn" => "dr"})
    ng_dest     = IPVS::Dest.new({"addr" => "192.168.0.3", "weight" => 256, "port" => 80, "conn" => "dr"})

    assert_true(d[0].equal?(expect_dest))
    assert_false(d[0].equal?(ng_dest))
  end
end
