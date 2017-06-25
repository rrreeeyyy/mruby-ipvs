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
  s = IPVS::Service.new({"addr" => "127.0.0.1", "port" => 80, "sched_name" => "rr"})
  d = IPVS::Dest.new({"addr" => "192.168.0.2", "weight" => 256, "port" => 80, "conn" => "dr"})

  begin
    s.add_service
    s.add_dest(d)
    block.call(s, d)
  ensure
    s.del_dest(d)
    s.del_service
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

assert('IPVS::Service.get') do
  add_service_with do |s,d|
    current = IPVS::Service.get
    service = current.first

    assert_equal(1, current.length)
    assert_equal("127.0.0.1", service.addr)
    assert_equal(80, service.port)
    assert_equal("rr", service.sched_name)
  end
end

assert('IPVS::Service#dests') do
  add_service_with do |s,d|
    dests = s.dests
    dest= dests.first

    assert_equal(1, dests.length)
    assert_equal("192.168.0.2", dest.addr)
    assert_equal(80, dest.port)
    assert_equal(256, dest.weight)
    assert_equal("DR", dest.conn)
  end
end

assert('IPVS::Service#equal?') do
  add_service_with do |s,d|
    expect_service = IPVS::Service.new({"addr" => "127.0.0.1", "port" => 80, "sched_name" => "rr"})
    ng_service     = IPVS::Service.new({"addr" => "127.0.0.2", "port" => 80, "sched_name" => "rr"})

    assert_true(s.equal?(expect_service))
    assert_false(s.equal?(ng_service))
  end
end

assert('IPVS::Service#deep_equal?') do
  add_service_with do |s,d|
    Dummy = ::Struct.new(:addr, :port, :proto, :sched_name, :dests)
    ng_dest     = IPVS::Dest.new({"addr" => "192.168.0.3", "weight" => 256, "port" => 80, "conn" => "dr"})

    dummy = Dummy.new("127.0.0.1", 80, "TCP", "rr", [d])
    assert_true(s.deep_equal?(dummy))

    dummy = Dummy.new("127.0.0.1", 80, "TCP", "rr", [ng_dest])
    assert_false(s.deep_equal?(dummy))
  end
end

assert('IPVS::Dest#equal?') do
  add_service_with do |s,d|
    expect_dest = IPVS::Dest.new({"addr" => "192.168.0.2", "weight" => 256, "port" => 80, "conn" => "dr"})
    ng_dest     = IPVS::Dest.new({"addr" => "192.168.0.3", "weight" => 256, "port" => 80, "conn" => "dr"})

    assert_true(d.equal?(expect_dest))
    assert_false(d.equal?(ng_dest))
  end
end
