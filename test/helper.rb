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
