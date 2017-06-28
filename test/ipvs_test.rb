##
# mruby-ipvs test

require '../test/helper'
assert('define module IPVS') do
  Object.const_defined?(:IPVS)
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

