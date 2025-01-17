class Set
  def initialize(items = nil)
    @data = Hash.new
    
    return if items.nil?

    items.each do |item|
      add(item)
    end
  end

  def self.[](*items)
    new(items)
  end

  def length
    @data.length
  end
  alias size length

  def add(obj)
    @data[obj] = true
    self
  end
  alias << add

  def add?(obj)
    if include?(obj)
      nil
    else
      @data[obj] = true
      self
    end
  end

  def include?(obj)
    @data.key?(obj)
  end

  def clear
    @data.clear
    self
  end

  def join(sep = nil)
    to_a.join(sep)
  end

  def to_a
    @data.keys
  end
end
