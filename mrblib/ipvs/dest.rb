class IPVS
  class Dest
    def equal?(b)
      %w(
        addr
        port
        weight
        conn
      ).all? do |n|
        self.send(n) == b.send(n)
      end
    end
  end
end
