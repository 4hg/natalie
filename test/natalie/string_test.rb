# encoding: UTF-8

require_relative '../spec_helper'

describe 'string' do
  it 'processes backslashes properly' do
    "foo\\bar".should == "foo" + "\\" + "bar"
  end

  describe '#inspect' do
    it 'returns a code representation of the string' do
      'foo'.inspect.should == '"foo"'
      "foo\nbar".inspect.should == "\"foo\\nbar\""
      "😉🤷".inspect.should == "\"😉🤷\""
    end
  end

  describe "#<=>" do
    it "should return -1 if lhs is less than rhs" do
      ('a' <=> 'b').should == -1
      ('a' <=> 'z').should == -1
    end

    it "should return 1 if lhs is greater than rhs" do
      ('b' <=> 'a').should == 1
      ('z' <=> 'a').should == 1
    end

    it "should return 0 if both sides are equal" do
      ('a' <=> 'a').should == 0
      ('z' <=> 'z').should == 0
    end
  end

  describe '#bytes' do
    it 'returns an array of byte values' do
      'foo'.bytes.should == [102, 111, 111]
    end
  end

  describe '#ord' do
    it 'returns the character code for the first character of the string' do
      ' '.ord.should == 32
      'a'.ord.should == 97
      'abc'.ord.should == 97
      'ă'.ord.should == 259
      '”'.ord.should == 8221
      '😉'.ord.should == 128521
      '😉😉😉'.ord.should == 128521
    end

    it 'raises an error if the string is empty' do
      -> { ''.ord }.should raise_error(ArgumentError, 'empty string')
    end
  end

  describe '#encode' do
    it 'changes the encoding while reinterpreting the characters' do
      s = 'abc123'
      s.encoding.should == Encoding::UTF_8
      s2 = s.encode 'ascii-8bit'
      s2.encoding.should == Encoding::ASCII_8BIT
      s2.should == 'abc123'
      s3 = s2.encode 'utf-8'
      s3.encoding.should == Encoding::UTF_8
      s3.should == 'abc123'
    end

    it 'raises an error if a character cannot be converted to the new encoding' do
      s = 'abc 😢'
      s.encoding.should == Encoding::UTF_8
      -> { s.encode 'ascii-8bit' }.should raise_error(Encoding::UndefinedConversionError, 'U+1F622 from UTF-8 to ASCII-8BIT')
      s = 'xyz 🥺'
      s.encoding.should == Encoding::UTF_8
      -> { s.encode 'ascii-8bit' }.should raise_error(Encoding::UndefinedConversionError, 'U+1F97A from UTF-8 to ASCII-8BIT')
    end

    it 'raises an error if the encoding converter does not exist' do
      s = 'abc 😢'
      -> { s.encode 'bogus-fake-encoding' }.should raise_error(StandardError) # TODO: not actually the right error ;-)
    end
  end

  describe '#force_encoding' do
    it 'changes the encoding without reinterpreting the characters' do
      s = ''
      s.encoding.should == Encoding::UTF_8
      s.force_encoding 'ascii-8bit'
      s.encoding.should == Encoding::ASCII_8BIT
    end
  end

  describe '#each_char' do
    it 'yields to the block each character' do
      result = []
      'foo'.each_char do |char|
        result << char
      end
      result.should == ['f', 'o', 'o']
    end
  end

  describe '#chars' do
    it 'returns an array of characters' do
      'foo'.chars.should == ['f', 'o', 'o']
      s = "😉”ăa"
      s.chars.should == ["😉", "”", "ă", "a"]
      s.force_encoding 'ascii-8bit'
      s.chars.map { |c| c.ord }.should == [240, 159, 152, 137, 226, 128, 157, 196, 131, 97]
    end
  end

  describe '[]' do
    it 'returns the character at the given index' do
      s = "😉”ăa"
      s[0].should == "😉"
      s[1].should == "”"
      s[2].should == "ă"
      s[3].should == "a"
    end

    it 'returns nil if the index is past the end' do
      s = "😉”ăa"
      s[4].should == nil
    end

    it 'returns the character from the end given a negative index' do
      s = "😉”ăa"
      s[-1].should == "a"
      s[-2].should == "ă"
      s[-3].should == "”"
      s[-4].should == "😉"
    end

    it 'returns nil if the negative index is too small' do
      s = "😉”ăa"
      s[-5].should == nil
    end

    context 'given a range' do
      it 'returns a substring' do
        s = "😉”ăa"
        s[1..-1].should == "”ăa"
      end

      it 'returns nil for a range out of bounds' do
        s = "hello"
        s[-2..0].should == ""
        s[2..100].should == "llo"
      end

      it 'returns nil for a range that starts beyond the end of the string' do
        s = "hello"
        s[90..100].should == nil
      end
    end
  end

  describe '#succ' do
    context 'given a single character' do
      it 'returns the next character' do
        'a'.succ.should == 'b'
        'm'.succ.should == 'n'
        'A'.succ.should == 'B'
        'M'.succ.should == 'N'
        '0'.succ.should == '1'
      end

      it 'loops on z' do
        'z'.succ.should == 'aa'
      end

      it 'loops on Z' do
        'Z'.succ.should == 'AA'
      end

      it 'loops on 9' do
        '9'.succ.should == '10'
      end
    end

    context 'given multiple characters' do
      it 'loops on z' do
        'az'.succ.should == 'ba'
        'aaz'.succ.should == 'aba'
        'zzz'.succ.should == 'aaaa'
      end
    end

    # TODO: handle mixed case, e.g. 'Az' and 'Zz'

    context 'given a character outside alphanumeric range' do
      it 'returns the next character' do
        '👍'.succ.should == '👎'
      end
    end
  end

  describe '#index' do
    it 'returns the index of the substring' do
      s = 'tim is ok'
      s.index('tim').should == 0
      s.index('is').should == 4
      s.index('ok').should == 7
    end

    it 'returns nil if the substring cannot be found' do
      s = 'tim is ok'
      s.index('rocks').should == nil
    end
  end

  describe '#start_with?' do
    it 'returns true if the string starts with the given substring' do
      s = 'tim morgan'
      s.start_with?('tim').should be_true
      s.start_with?('t').should be_true
      s.start_with?('').should be_true
      s.start_with?('x').should be_false
      s.start_with?('xxxxxxxxxxxxxxx').should be_false
    end
  end

  describe '#sub' do
    it 'returns a duped string if no substitution was made' do
      s = 'tim is ok'
      s.sub('is cool', '').object_id.should != s.object_id
      s.sub(/is cool/, '').object_id.should != s.object_id
    end

    it 'replaces the matching string' do
      s = 'tim is ok'
      s.sub('is ok', 'rocks').should == 'tim rocks'
      s.should == 'tim is ok'
      s.sub(' is ok', '').should == 'tim'
      s.sub('bogus', '').should == 'tim is ok'
    end

    it 'replaces the matching regex' do
      s = 'tim is ok'
      s.sub(/is ok/, 'rocks').should == 'tim rocks'
      s.should == 'tim is ok'
      s.sub(/ is ok/, '').should == 'tim'
      s.sub(/is.*/, 'rocks').should == 'tim rocks'
      s.sub(/bogus/, '').should == 'tim is ok'
    end

    it 'substitues back references' do
      '0b1101011'.sub(/0b([01]+)/, 'the binary number is \1').should == 'the binary number is 1101011'
      'abc'.sub(/([a-z]+)/, '\0def').should == 'abcdef'
    end

    it 'raises an error if the arguments are of the wrong type' do
      -> { 'foo'.sub(1, 'bar') }.should raise_error(TypeError, 'wrong argument type Integer (expected Regexp)')
      -> { 'foo'.sub(:foo, 'bar') }.should raise_error(TypeError, 'wrong argument type Symbol (expected Regexp)')
      -> { 'foo'.sub('foo', :bar) }.should raise_error(TypeError, 'no implicit conversion of Symbol into String')
    end
  end

  describe '#to_i' do
    it 'returns an Integer by recognizing digits in the string' do
      '12345'.to_i.should == 12345
      ' 12345'.to_i.should == 12345
      ' 123 45'.to_i.should == 123
      '99 red balloons'.to_i.should == 99
      '0a'.to_i.should == 0
      '0a'.to_i(16).should == 10
      '0A'.to_i(16).should == 10
      'hello'.to_i.should == 0
      '1100101'.to_i(2).should == 101
      '1100101'.to_i(8).should == 294977
      '1100101'.to_i(10).should == 1100101
      '1100101'.to_i(16).should == 17826049
    end
  end

  describe '#split' do
    it 'splits a string into an array of smaller strings using a string match' do
      ''.split(',').should == []
      ' '.split(',').should == [' ']
      'tim'.split(',').should == ['tim']
      'tim,morgan,rocks'.split(',').should == ['tim', 'morgan', 'rocks']
      'tim morgan rocks'.split(' morgan ').should == ['tim', 'rocks']
    end

    it 'splits a string into an array of smaller strings using a regexp match' do
      ''.split(/,/).should == []
      ' '.split(/,/).should == [' ']
      'tim'.split(/,/).should == ['tim']
      'tim,morgan,rocks'.split(/,/).should == ['tim', 'morgan', 'rocks']
      'tim     morgan rocks'.split(/\s+/).should == ['tim', 'morgan', 'rocks']
      'tim morgan rocks'.split(/ mo[a-z]+ /).should == ['tim', 'rocks']
    end
  end

  describe '#ljust' do
    it 'returns a padded copy using spaces to reach the desired length' do
      s = 'tim'
      s.ljust(10).should == 'tim       '
    end

    it 'returns a padded copy using the given padstr to reach the desired length' do
      s = 'tim'
      s.ljust(10, 'x').should == 'timxxxxxxx'
      s.ljust(10, 'xy').should == 'timxyxyxyx'
    end

    it 'returns an unmodified copy if the string length is already the desired length' do
      s = 'tim morgan'
      s.ljust(10).should == 'tim morgan'
      s.ljust(5).should == 'tim morgan'
      s.ljust(5, 'x').should == 'tim morgan'
    end
  end

  describe 'subclass' do
    class NegativeString < String
      def initialize(s)
        super('not ' + s)
        @bar = 'bar'
      end
      attr_reader :bar
    end

    it 'works' do
      s = NegativeString.new('foo')
      s.should == 'not foo'
      s.bar.should == 'bar'
    end
  end
end
