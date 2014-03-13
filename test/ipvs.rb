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

assert('IPVS::Dest.new({"addr" => "10.0.0.1", "weight" => 5}') do
  dest = IPVS::Dest.new({"addr" => "10.0.0.1", "weight" => 5})
  dest.weight = 3 and dest.weight == 3
end
