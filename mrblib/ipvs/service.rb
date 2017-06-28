class IPVS
  class Service
    attr_accessor :dests
    def equal?(b)
      %w(
        proto
        addr
        port
        sched_name
      ).all? do |n|
        self.send(n) == b.send(n)
      end
    end

    def deep_equal?(b)
      equal?(b) &&
      dests.all? {|d| b.dests.any? {|bd| bd.equal?(d) } } &&
      b.dests.all? {|bd| self.dests.any? {|d| d.equal?(bd) } }
    end
  end
end
