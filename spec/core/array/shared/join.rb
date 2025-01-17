require_relative '../fixtures/classes'
require_relative '../fixtures/encoded_strings'

describe :array_join_with_default_separator, shared: true do
  before :each do
    @separator = $,
  end

  after :each do
    $, = @separator
  end

  it "returns an empty string if the Array is empty" do
    [].send(@method).should == ''
  end

  it "returns a US-ASCII string for an empty Array" do
    [].send(@method).encoding.should == Encoding::US_ASCII
  end

  it "returns a string formed by concatenating each String element separated by $," do
    suppress_warning {
      $, = " | "
      ["1", "2", "3"].send(@method).should == "1 | 2 | 3"
    }
  end

  it "attempts coercion via #to_str first" do
    obj = mock('foo')
    obj.should_receive(:to_str).any_number_of_times.and_return("foo")
    [obj].send(@method).should == "foo"
  end

  it "attempts coercion via #to_ary second" do
    obj = mock('foo')
    obj.should_receive(:to_str).any_number_of_times.and_return(nil)
    obj.should_receive(:to_ary).any_number_of_times.and_return(["foo"])
    [obj].send(@method).should == "foo"
  end

  it "attempts coercion via #to_s third" do
    obj = mock('foo')
    obj.should_receive(:to_str).any_number_of_times.and_return(nil)
    obj.should_receive(:to_ary).any_number_of_times.and_return(nil)
    obj.should_receive(:to_s).any_number_of_times.and_return("foo")
    [obj].send(@method).should == "foo"
  end

  it "raises a NoMethodError if an element does not respond to #to_str, #to_ary, or #to_s" do
    obj = mock('o')
    class << obj; undef :to_s; end
    -> { [1, obj].send(@method) }.should raise_error(NoMethodError)
  end

  it "raises an ArgumentError when the Array is recursive" do
    -> { ArraySpecs.recursive_array.send(@method) }.should raise_error(ArgumentError)
    -> { ArraySpecs.head_recursive_array.send(@method) }.should raise_error(ArgumentError)
    -> { ArraySpecs.empty_recursive_array.send(@method) }.should raise_error(ArgumentError)
  end

  # NATFIXME : Revisit when Encoding.compatiblity? is implemented
  xit "uses the first encoding when other strings are compatible" do
    ary1 = ArraySpecs.array_with_7bit_utf8_and_usascii_strings
    ary2 = ArraySpecs.array_with_usascii_and_7bit_utf8_strings
    ary3 = ArraySpecs.array_with_utf8_and_7bit_binary_strings
    ary4 = ArraySpecs.array_with_usascii_and_7bit_binary_strings

    ary1.send(@method).encoding.should == Encoding::UTF_8
    ary2.send(@method).encoding.should == Encoding::US_ASCII
    ary3.send(@method).encoding.should == Encoding::UTF_8
    ary4.send(@method).encoding.should == Encoding::US_ASCII
  end
  
  it "uses the widest common encoding when other strings are incompatible" do
    ary1 = ArraySpecs.array_with_utf8_and_usascii_strings
    ary2 = ArraySpecs.array_with_usascii_and_utf8_strings

    ary1.send(@method).encoding.should == Encoding::UTF_8
    ary2.send(@method).encoding.should == Encoding::UTF_8
  end

  # NATFIXME : Revisit when Encoding.compatiblity? is implemented
  xit "fails for arrays with incompatibly-encoded strings" do
    ary_utf8_bad_binary = ArraySpecs.array_with_utf8_and_binary_strings

    -> { ary_utf8_bad_binary.send(@method) }.should raise_error(EncodingError)
  end

  # NATFIXME : Pending proper handling of deprecation warnings
  xcontext "when $, is not nil" do
    before do
      suppress_warning do
        $, = '*'
      end
    end

    it "warns" do
      -> { [].join }.should complain(/warning: \$, is set to non-nil value/)
      -> { [].join(nil) }.should complain(/warning: \$, is set to non-nil value/)
    end
  end
end

describe :array_join_with_string_separator, shared: true do
  it "returns a string formed by concatenating each element.to_str separated by separator" do
    obj = mock('foo')
    obj.should_receive(:to_str).and_return("foo")
    [1, 2, 3, 4, obj].send(@method, ' | ').should == '1 | 2 | 3 | 4 | foo'
  end

  it "uses the same separator with nested arrays" do
    [1, [2, [3, 4], 5], 6].send(@method, ":").should == "1:2:3:4:5:6"
    [1, [2, ArraySpecs::MyArray[3, 4], 5], 6].send(@method, ":").should == "1:2:3:4:5:6"
  end
end
