module Comparable
  def ==(other)
    result = self <=> other
    raise ArgumentError, "comparison of #{result.class} with 0 failed" if !result.is_a?(Numeric) && !result.nil?
    result == 0
  end

  def <(other)
    result = (self <=> other)
    raise ArgumentError, "comparison of #{self.class} with #{other.class} failed" if result.nil?
    result < 0
  end

  def <=(other)
    result = (self <=> other)
    raise ArgumentError, "comparison of #{self.class} with #{other.class} failed" if result.nil?
    result <= 0
  end

  def >(other)
    !(self <= other)
  end

  def >=(other)
    !(self < other)
  end

  def between?(min, max)
    self >= min && self <= max
  end

  def clamp(*args)
    if args.length == 2
      min, max = args

      compared = min <=> max
      raise ArgumentError, 'min argument must be smaller than max argument' if compared.nil? || compared > 0

      if self < min
        min
      elsif self > max
        max
      else
        self
      end
    elsif args.first.is_a?(Range)
      range = args.first
      raise ArgumentError, 'cannot clamp with an exclusive range' if range.exclude_end?
      clamp(range.begin, range.end)
    elsif args.length == 1
      raise TypeError, "wrong argument type #{args.first.inspect} (expected Range)"
    else
      raise ArgumentError, "wrong number of arguments (given #{args.length.size}, expected 1..2)"
    end
  end
end
