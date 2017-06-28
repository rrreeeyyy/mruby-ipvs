class IPVS
  module Formater
    def to_h
      attributes.each_with_object({}) do |a,r|
        v = self.send(a)
        v = v.map {|vv| vv.to_h } if v.is_a?(Array)
        r[a] = v
      end
    end
    alias_method :inspect, :to_h
  end
end
