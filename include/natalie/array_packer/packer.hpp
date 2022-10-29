#pragma once

#include "natalie/array_object.hpp"
#include "natalie/array_packer/float_handler.hpp"
#include "natalie/array_packer/integer_handler.hpp"
#include "natalie/array_packer/string_handler.hpp"
#include "natalie/array_packer/tokenizer.hpp"
#include "natalie/env.hpp"
#include "natalie/string_object.hpp"
#include "natalie/symbol_object.hpp"

namespace Natalie {

namespace ArrayPacker {

    class Packer : public Cell {
    public:
        Packer(ArrayObject *source, String directives)
            : m_source { source }
            , m_directives { Tokenizer { directives }.tokenize() }
            , m_encoding { EncodingObject::get(Encoding::ASCII_8BIT) } { }

        ~Packer() { delete m_directives; }

        StringObject *pack(Env *env) {
            for (auto token : *m_directives) {
                if (token.error)
                    env->raise("ArgumentError", *token.error);

                char d = token.directive;
                switch (d) {
                case 'A':
                case 'a':
                case 'B':
                case 'b':
                case 'H':
                case 'h':
                case 'M':
                case 'm':
                case 'P':
                case 'p':
                case 'u':
                case 'Z': {
                    if (at_end())
                        env->raise("ArgumentError", "too few arguments");

                    String string;
                    StringObject *string_object = nullptr;
                    auto item = m_source->at(m_index);
                    if (m_source->is_string()) {
                        string_object = item->as_string();
                        string = item->as_string()->string();
                    } else if (item->is_nil()) {
                        if (d == 'u' || d == 'm')
                            env->raise("TypeError", "no implicit conversion of nil into String");
                        string = "";
                    } else if (item->respond_to(env, "to_str"_s)) {
                        auto str = item->send(env, "to_str"_s);
                        str->assert_type(env, Object::Type::String, "String");
                        string_object = str->as_string();
                        string = str->as_string()->string();
                    } else if (d == 'M' && (item->respond_to(env, "to_s"_s))) {
                        auto str = item->send(env, "to_s"_s);
                        if (str->is_string()) {
                            string_object = str->as_string();
                            string = str->as_string()->string();
                        } else {
                            string_object = str->to_s(env)->as_string();
                            string = str->to_s(env)->string();
                        }
                    } else {
                        env->raise("TypeError", "no implicit conversion of {} into String", item->klass()->inspect_str());
                        NAT_UNREACHABLE();
                    }

                    auto packer = StringHandler { string, string_object, token };
                    m_packed.append(packer.pack(env));

                    if (d == 'm' || d == 'M')
                        m_encoding = EncodingObject::get(Encoding::US_ASCII);

                    m_index++;
                    break;
                }
                case 'C':
                case 'c':
                case 'I':
                case 'i':
                case 'J':
                case 'j':
                case 'L':
                case 'l':
                case 'N':
                case 'n':
                case 'S':
                case 's':
                case 'U':
                case 'V':
                case 'v': {
                    pack_with_loop(env, token, [&]() {
                        auto integer = m_source->at(m_index)->to_int(env);
                        auto packer = IntegerHandler { integer, token };
                        auto result = packer.pack(env);
                        m_packed.append(result);
                    });

                    if (d == 'U')
                        m_encoding = EncodingObject::get(Encoding::UTF_8);
                    else
                        m_encoding = EncodingObject::get(Encoding::ASCII_8BIT);

                    break;
                }
                case 'D':
                case 'd':
                case 'E':
                case 'e':
                case 'F':
                case 'f':
                case 'G':
                case 'g': {
                    pack_with_loop(env, token, [&]() {
                        auto value = m_source->at(m_index);
                        if (value->is_integer())
                            value = value->as_integer()->to_f();
                        auto packer = FloatHandler { value->as_float_or_raise(env), token };
                        m_packed.append(packer.pack(env));
                    });
                    break;
                }
                case 'x': {
                    // Asterisks have no effect on this directive.
                    if (token.star)
                        break;

                    auto null_byte_count = (token.count < 0) ? 1 : token.count;
                    for (int count = 0; count < null_byte_count; count++) {
                        m_packed.append_char('\0');
                    }
                    break;
                }
                case 'X': {
                    // Asterisks have no effect on this directive.
                    if (token.star)
                        break;

                    auto amount_of_truncated_bytes = (token.count < 0) ? 1 : token.count;

                    // If the packed string is empty or if the amount of bytes to be truncated
                    // is greater than the packed string's current length,
                    // then we can't truncate it. Raise ArgumentError.
                    if (m_packed.length() == 0 || (int)m_packed.length() < amount_of_truncated_bytes) {
                        env->raise("ArgumentError", "X outside of the string");
                    }
                    m_packed.truncate(m_packed.length() - amount_of_truncated_bytes);
                    break;
                }
                case '@': {
                    auto count = (token.count < 0) ? 1 : token.count;
                    auto missing_chars = static_cast<nat_int_t>(count) - static_cast<nat_int_t>(m_packed.size());
                    if (missing_chars > 0) {
                        for (nat_int_t i = 0; i < missing_chars; ++i)
                            m_packed.append_char('\0');
                        break;
                    }
                    m_packed.truncate(count);
                    break;
                }
                default: {
                    env->raise("ArgumentError", "{} is not supported", d);
                }
                }
            }
            return new StringObject { m_packed, m_encoding };
        }

        virtual void visit_children(Visitor &visitor) override {
            visitor.visit(m_source);
            visitor.visit(m_encoding);
        }

    private:
        template <typename Fn>
        void pack_with_loop(Env *env, Token token, Fn handler) {
            auto starting_index = m_index;

            auto is_complete = [&]() {
                if (token.star)
                    return at_end();

                size_t count = token.count != -1 ? token.count : 1;
                bool complete = m_index - starting_index >= count;

                if (!complete && at_end())
                    env->raise("ArgumentError", "too few Arguments");

                return complete;
            };

            while (!is_complete()) {
                handler();
                m_index++;
            }
        }

        bool at_end() { return m_index >= m_source->size(); }

        ArrayObject *m_source;
        TM::Vector<Token> *m_directives;
        String m_packed {};
        EncodingObject *m_encoding;
        size_t m_index { 0 };
    };

}

}
