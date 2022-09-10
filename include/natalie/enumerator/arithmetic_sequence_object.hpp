#pragma once

#include "natalie/forward.hpp"
#include "natalie/object.hpp"

namespace Natalie {
class Enumerator::ArithmeticSequenceObject : public Object {
public:
    ArithmeticSequenceObject(ClassObject *klass)
        : Object { Object::Type::EnumeratorArithmeticSequence, klass } { }

    ArithmeticSequenceObject()
        : ArithmeticSequenceObject { GlobalEnv::the()->Object()->const_fetch("Enumerator"_s)->const_fetch("ArithmeticSequence"_s)->as_class() } { }

    ArithmeticSequenceObject(Value begin, Value end, Value step, bool exclude_end)
        : ArithmeticSequenceObject {} {
        m_begin = begin;
        m_end = end;
        m_step = step;
        m_exclude_end = exclude_end;
    }

    Value begin() { return m_begin; }
    Value end() { return m_end; }

    virtual void gc_inspect(char *buf, size_t len) const override {
        snprintf(buf, len, "<Enumerator::ArithmeticSequence %p>", this);
    }

    virtual void visit_children(Visitor &visitor) override {
        Object::visit_children(visitor);
        visitor.visit(m_begin);
        visitor.visit(m_end);
        visitor.visit(m_step);
    }

private:
    Value m_begin { nullptr };
    Value m_end { nullptr };
    Value m_step { nullptr };
    bool m_exclude_end { false };
};
};
