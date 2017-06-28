require File.expand_path('../helper', __FILE__)

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

assert('IPVS::Service.to_h') do
  expect = {
    "proto"=>"TCP",
    "addr"=>"127.0.0.1",
    "port"=>80,
    "sched_name"=>"rr",
    "dests"=> [
      {
       "addr"=>"192.168.0.1",
        "port"=>80,
        "weight"=>256,
        "conn"=>"DR"
      },
      {
       "addr"=>"192.168.0.2",
        "port"=>80,
        "weight"=>256,
        "conn"=>"DR"
      }
    ]
  }

  add_service_with do |s,d|
    assert_equal(expect, s.first.to_h)
  end
end

assert('IPVS.Service.inspect') do
  add_service_with do |s,_|
    i = s.first.inspect
    h = s.first.to_h
    assert_equal(i, h)
  end
end
