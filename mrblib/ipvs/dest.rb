module IPVS::Formater; end
class IPVS
  class Dest
    include IPVS::Formater
    def equal?(b)
      attributes.all? do |n|
        self.send(n) == b.send(n)
      end
    end

    private
    def attributes
      %w(
        addr
        port
        weight
        conn
      )
    end
  end
end
