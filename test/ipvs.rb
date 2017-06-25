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

assert('IPVS::Service.inpect') do
  expect = [
    {
      "protocol"=>"TCP",
      "addr"=>"127.0.0.1",
      "port"=>80,
      "sched_name"=>"rr",
      "dests"=> [{
          "addr"=>"127.0.0.1",
          "port"=>80,
          "weight"=>256,
          "conn"=>"DR"
      }]
    }
  ]
  s = IPVS::Service.new({"addr" => "127.0.0.1", "port" => 80, "sched_name" => "rr"})
  d = IPVS::Dest.new({"addr" => "127.0.0.1", "weight" => 256, "port" => 80, "conn" => "dr"})

  begin
    s.add_service
    s.add_dest(d)
    assert_equal(expect, IPVS::Service.inspect)
  ensure
    s.del_dest(d)
    s.del_service
  end
end
