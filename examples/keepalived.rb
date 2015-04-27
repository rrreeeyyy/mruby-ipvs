class IPVS
  module Keepalived
    @lb_algo = 'wrr', @lb_kind = 'NAT', @protocol = 'TCP'
    @weight = 1, @real_servers = [], @virtual_server = nil,
      @healthcheck = []

    def self.lb_algo(algo)
      @lb_algo = algo
    end

    def self.lb_kind(kind)
      @lb_kind = kind
    end

    def self.protocol(proto)
      @protocol = proto
    end

    def self.virtual_server(addr, opts, &block)
      block.call if block_given?
      @virtual_server = IPVS::Service.new({
        'addr' => addr,
        'sched_name' => @lb_algo,
        'proto' => @protocol
      })
      @virtual_server.add_service
      @real_servers.each do |real_server|
        @virtual_server.add_dest(real_server)
      end
    end

    def self.weight(weight)
      @weight = weight
    end

    def self.real_server(addr, opts, &block)
      block.call if block_given?
      @real_servers << real_server = IPVS::Dest.new({
        'addr' => addr,
        'weight' => @weight,
        'fwd' => @lb_kind
      })
      real_server
    end

    def self.healthcheck(&block)
      @healthcheck << block
    end

    def self.start
      @healthcheck.each do |h|
        h.call(@virtual_server, @real_servers)
      end
      # sleep @wait_time
    end
  end
end

module Kernel
  def virtual_server(addr, opts={}, &block)
    IPVS::Keepalived.virtual_server addr, opts, &block
  end
  def lb_algo(algo)
    IPVS::Keepalived.lb_algo algo
  end
  def lb_kind(kind)
    IPVS::Keepalived.lb_kind kind
  end
  def protocol(proto)
    IPVS::Keepalived.protocol proto
  end
  def real_server(addr, opts={}, &block)
    IPVS::Keepalived.real_server addr, opts, &block
  end
  def healthcheck(&block)
    IPVS::Keepalived.healthcheck(&block)
  end
  def weight(weight)
    IPVS::Keepalived.weight weight
  end
end

# # For example
# virtual_server('10.0.0.1:80'){
#   lb_algo 'wrr'
#   lb_kind 'NAT'
#   protocol 'TCP'
#
#   web1 = real_server('192.168.0.1:80'){
#     weight 1
#   }
#
#   web2 = real_server('192.168.0.2:80'){
#     weight 2
#   }
#
#   web3 = real_server('192.168.0.3:80')
# }
#
# IPVS::Keepalived.start
