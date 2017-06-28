require File.expand_path('../helper', __FILE__)

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

assert('IPVS::Dest#equal?') do
  add_service_with do |s,d|
    expect_dest = IPVS::Dest.new({"addr" => "192.168.0.1", "weight" => 256, "port" => 80, "conn" => "dr"})
    ng_dest     = IPVS::Dest.new({"addr" => "192.168.0.3", "weight" => 256, "port" => 80, "conn" => "dr"})

    assert_true(d[0].equal?(expect_dest))
    assert_false(d[0].equal?(ng_dest))
  end
end

assert('IPVS::Dest.to_h') do
  expect = {
   "addr"=>"192.168.0.1",
    "port"=>80,
    "weight"=>256,
    "conn"=>"DR"
  }

  add_service_with do |s,d|
    assert_equal(expect, d.first.to_h)
  end
end

assert('IPVS.Dest.inspect') do
  add_service_with do |_,d|
    i = d.first.inspect
    h = d.first.to_h
    assert_equal(i, h)
  end
end
