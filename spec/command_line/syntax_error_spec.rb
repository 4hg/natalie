require_relative '../spec_helper'

describe "The interpreter" do
  it "prints an error when given a file with invalid syntax" do
    out = ruby_exe(fixture(__FILE__, "bad_syntax.rb"), args: "2>&1", exit_status: 1)
    out.should include "syntax error"
  end

  # NATFIXME: It looks like we only get a partial output here
  xit "prints an error when given code via -e with invalid syntax" do
    out = ruby_exe(nil, args: "-e 'a{' 2>&1", exit_status: 1)
    out.should include "syntax error"
  end
end
