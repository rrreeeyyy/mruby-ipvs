module IPVS::Formater; end
class IPVS
  class Service
    include IPVS::Formater
    attr_accessor :dests
    def equal?(b)
      attributes.select {|a| a != 'dests' }.all? do |n|
        self.send(n) == b.send(n)
      end
    end

    def deep_equal?(b)
      equal?(b) &&
      dests.all? {|d| b.dests.any? {|bd| bd.equal?(d) } } &&
      b.dests.all? {|bd| self.dests.any? {|d| d.equal?(bd) } }
    end

    private
    def attributes
      %w(
        proto
        addr
        port
        sched_name
        dests
      )
    end
  end
end
