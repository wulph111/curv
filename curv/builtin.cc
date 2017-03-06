// Copyright Doug Moen 2016-2017.
// Distributed under The MIT License.
// See accompanying file LICENSE.md or https://opensource.org/licenses/MIT

#include <cassert>
#include <cmath>
#include <cstdlib>
#include <string>

#include <boost/math/constants/constants.hpp>

#include <curv/arg.h>
#include <curv/builtin.h>
#include <curv/eval.h>
#include <curv/exception.h>
#include <curv/file.h>
#include <curv/function.h>
#include <curv/shape.h>
#include <curv/system.h>
#include <curv/gl_context.h>
#include <curv/array_op.h>
#include <curv/analyzer.h>

using namespace std;
using namespace boost::math::double_constants;

namespace curv {

Shared<Meaning>
Builtin_Value::to_meaning(const Identifier& id) const
{
    return make<Constant>(share(id), value_);
}

struct Is_List_Function : public Polyadic_Function
{
    Is_List_Function() : Polyadic_Function(1) {}
    Value call(Frame& args) override
    {
        return {args[0].dycast<List>() != nullptr};
    }
};

struct Bit_Function : public Polyadic_Function
{
    Bit_Function() : Polyadic_Function(1) {}
    Value call(Frame& args) override
    {
        return {double(args[0].to_bool(At_Arg(0, args)))};
    }
    GL_Value gl_call(GL_Frame& f) const override
    {
        auto arg = f[0];
        if (arg.type != GL_Type::Bool)
            throw Exception(At_GL_Arg(0, f),
                "bit: argument is not a bool");
        auto result = f.gl.newvalue(GL_Type::Num);
        f.gl.out << "  float "<<result<<" = float("<<arg<<");\n";
        return result;
    }
};

struct Sqrt_Function : public Polyadic_Function
{
    Sqrt_Function() : Polyadic_Function(1) {}
    struct Scalar_Op {
        static double f(double x) { return sqrt(x); }
        static Shared<const String> callstr(Value x) {
            return stringify("sqrt(",x,")");
        }
    };
    static Unary_Numeric_Array_Op<Scalar_Op> array_op;
    Value call(Frame& args) override
    {
        return array_op.op(args[0], At_Arg(0, args));
    }
    GL_Value gl_call(GL_Frame& f) const override
    {
        return gl_call_unary_numeric(f, "sqrt");
    }
};
// log(x) is the natural logarithm of x
struct Log_Function : public Polyadic_Function
{
    Log_Function() : Polyadic_Function(1) {}
    struct Scalar_Op {
        static double f(double x) { return log(x); }
        static Shared<const String> callstr(Value x) {
            return stringify("log(",x,")");
        }
    };
    static Unary_Numeric_Array_Op<Scalar_Op> array_op;
    Value call(Frame& args) override
    {
        return array_op.op(args[0], At_Arg(0, args));
    }
    GL_Value gl_call(GL_Frame& f) const override
    {
        return gl_call_unary_numeric(f, "log");
    }
};
struct Abs_Function : public Polyadic_Function
{
    Abs_Function() : Polyadic_Function(1) {}
    struct Scalar_Op {
        static double f(double x) { return abs(x); }
        static Shared<const String> callstr(Value x) {
            return stringify("abs(",x,")");
        }
    };
    static Unary_Numeric_Array_Op<Scalar_Op> array_op;
    Value call(Frame& args) override
    {
        return array_op.op(args[0], At_Arg(0, args));
    }
    GL_Value gl_call(GL_Frame& f) const override
    {
        return gl_call_unary_numeric(f, "abs");
    }
};
struct Floor_Function : public Polyadic_Function
{
    Floor_Function() : Polyadic_Function(1) {}
    struct Scalar_Op {
        static double f(double x) { return floor(x); }
        static Shared<const String> callstr(Value x) {
            return stringify("floor(",x,")");
        }
    };
    static Unary_Numeric_Array_Op<Scalar_Op> array_op;
    Value call(Frame& args) override
    {
        return array_op.op(args[0], At_Arg(0, args));
    }
    GL_Value gl_call(GL_Frame& f) const override
    {
        return gl_call_unary_numeric(f, "floor");
    }
};
struct Sin_Function : public Polyadic_Function
{
    Sin_Function() : Polyadic_Function(1) {}
    struct Scalar_Op {
        static double f(double x) { return sin(x); }
        static Shared<const String> callstr(Value x) {
            return stringify("sin(",x,")");
        }
    };
    static Unary_Numeric_Array_Op<Scalar_Op> array_op;
    Value call(Frame& args) override
    {
        return array_op.op(args[0], At_Arg(0, args));
    }
    GL_Value gl_call(GL_Frame& f) const override
    {
        return gl_call_unary_numeric(f, "sin");
    }
};
struct Cos_Function : public Polyadic_Function
{
    Cos_Function() : Polyadic_Function(1) {}
    struct Scalar_Op {
        static double f(double x) { return cos(x); }
        static Shared<const String> callstr(Value x) {
            return stringify("cos(",x,")");
        }
    };
    static Unary_Numeric_Array_Op<Scalar_Op> array_op;
    Value call(Frame& args) override
    {
        return array_op.op(args[0], At_Arg(0, args));
    }
    GL_Value gl_call(GL_Frame& f) const override
    {
        return gl_call_unary_numeric(f, "cos");
    }
};
struct Atan2_Function : public Polyadic_Function
{
    Atan2_Function() : Polyadic_Function(2) {}

    struct Scalar_Op {
        static double f(double x, double y) { return atan2(x, y); }
        static const char* name() { return "atan2"; }
        static Shared<const String> callstr(Value x, Value y) {
            return stringify("atan2(",x,",",y,")");
        }
    };
    static Binary_Numeric_Array_Op<Scalar_Op> array_op;
    Value call(Frame& args) override
    {
        return array_op.op(args[0], args[1],
            At_Phrase(*args.call_phrase, &args));
    }
    GL_Value gl_call(GL_Frame& f) const override
    {
        auto x = f[0];
        auto y = f[1];

        GL_Type rtype = GL_Type::Bool;
        if (x.type == y.type)
            rtype = x.type;
        else if (x.type == GL_Type::Num)
            rtype = y.type;
        else if (y.type == GL_Type::Num)
            rtype = x.type;
        if (rtype == GL_Type::Bool)
            throw Exception(At_GL_Phrase(*f.call_phrase, &f),
                "GL domain error");

        GL_Value result = f.gl.newvalue(rtype);
        f.gl.out <<"  "<<rtype<<" "<<result<<" = atan(";
        gl_put_as(f, x, f.call_phrase->at(0), rtype);
        f.gl.out << ",";
        gl_put_as(f, y, f.call_phrase->at(1), rtype);
        f.gl.out << ");\n";
        return result;
    }
};

struct Max_Function : public Polyadic_Function
{
    Max_Function() : Polyadic_Function(1) {}

    struct Scalar_Op {
        static double f(double x, double y) { return x > y ? x : y; }
        static const char* name() { return "max"; }
        static Shared<const String> callstr(Value x, Value y) {
            return stringify("max[",x,",",y,"]");
        }
    };
    static Binary_Numeric_Array_Op<Scalar_Op> array_op;
    Value call(Frame& args) override
    {
        return array_op.reduce(-INFINITY, args[0], At_Arg(0, args));
    }
    GL_Value gl_call(GL_Frame& f) const override
    {
        auto arg = f[0];
        if (arg.type != GL_Type::Vec2)
            throw Exception(At_GL_Arg(0, f),
                "max: argument is not a vec2");
        auto result = f.gl.newvalue(GL_Type::Num);
        f.gl.out << "  float "<<result<<" = max("<<arg<<".x,"<<arg<<".y);\n";
        return result;
    }
};

struct Min_Function : public Polyadic_Function
{
    Min_Function() : Polyadic_Function(1) {}

    struct Scalar_Op {
        static double f(double x, double y) { return x < y ? x : y; }
        static const char* name() { return "min"; }
        static Shared<const String> callstr(Value x, Value y) {
            return stringify("min[",x,",",y,"]");
        }
    };
    static Binary_Numeric_Array_Op<Scalar_Op> array_op;
    Value call(Frame& args) override
    {
        return array_op.reduce(INFINITY, args[0], At_Arg(0, args));
    }
    GL_Value gl_call(GL_Frame& f) const override
    {
        auto arg = f[0];
        if (arg.type != GL_Type::Vec2)
            throw Exception(At_GL_Arg(0, f),
                "min: argument is not a vec2");
        auto result = f.gl.newvalue(GL_Type::Num);
        f.gl.out << "  float "<<result<<" = min("<<arg<<".x,"<<arg<<".y);\n";
        return result;
    }
};

struct Mag_Function : public Polyadic_Function
{
    Mag_Function() : Polyadic_Function(1) {}
    Value call(Frame& args) override
    {
        // TODO: use hypot() or BLAS DNRM2 or Eigen stableNorm/blueNorm?
        // Avoids overflow/underflow due to squaring of large/small values.
        // Slower.  https://forum.kde.org/viewtopic.php?f=74&t=62402
        auto& list = arg_to_list(args[0], At_Arg(0, args));
        double sum = 0.0;
        for (auto val : list) {
            double x = val.get_num_or_nan();
            sum += x * x;
        }
        if (sum == sum)
            return {sqrt(sum)};
        throw Exception(At_Arg(0, args), "mag: domain error");
    }
    GL_Value gl_call(GL_Frame& f) const override
    {
        auto arg = f[0];
        if (arg.type != GL_Type::Vec2)
            throw Exception(At_GL_Arg(0, f),
                "mag: argument is not a vec2");
        auto result = f.gl.newvalue(GL_Type::Num);
        f.gl.out << "  float "<<result<<" = length("<<arg<<");\n";
        return result;
    }
};

struct Len_Function : public Polyadic_Function
{
    Len_Function() : Polyadic_Function(1) {}
    Value call(Frame& args) override
    {
        auto& list {arg_to_list(args[0], At_Arg(0, args))};
        return {double(list.size())};
    }
};

struct File_Function : public Polyadic_Function
{
    File_Function() : Polyadic_Function(1) {}
    Value call(Frame& f) override
    {
        At_Arg ctx0(0, f);
        String& path {arg_to_string(f[0], ctx0)};
        auto file = make<File_Script>(share(path), ctx0);
        return {eval_script(*file, f.system, &f)};
    }
};

struct Shape2d_Function : public Polyadic_Function
{
    Shape2d_Function() : Polyadic_Function(1) {}
    Value call(Frame& f) override
    {
        auto& record {arg_to_record(f[0], At_Arg(0, f))};
        return {make<Shape2D>(share(record))};
    }
};

/// The meaning of a call to `echo`, such as `echo("foo")`.
struct Echo_Action : public Just_Action
{
    std::vector<Shared<Operation>> argv_;
    Echo_Action(
        Shared<const Phrase> source,
        std::vector<Shared<Operation>> argv)
    :
        Just_Action(std::move(source)),
        argv_(std::move(argv))
    {}
    virtual void exec(Frame& f) const override
    {
        std::ostream& out = f.system.console();
        out << "ECHO: ";
        bool first = true;
        for (auto a : argv_) {
            if (!first) out << ",";
            out << a->eval(f);
            first = false;
        }
        out << "\n";
    }
};
/// The meaning of the phrase `echo` in isolation.
struct Echo_Metafunction : public Metafunction
{
    using Metafunction::Metafunction;
    virtual Shared<Meaning> call(const Call_Phrase& ph, Environ& env) override
    {
        return make<Echo_Action>(share(ph), ph.analyze_args(env));
    }
};

struct Assert_Action : public Just_Action
{
    Shared<Operation> arg_;
    Assert_Action(
        Shared<const Phrase> source,
        Shared<Operation> arg)
    :
        Just_Action(std::move(source)),
        arg_(std::move(arg))
    {}
    virtual void exec(Frame& f) const override
    {
        Value a = arg_->eval(f);
        if (!a.is_bool())
            throw Exception(At_Phrase(*source_, &f), "domain error");
        bool b = a.get_bool_unsafe();
        if (!b)
            throw Exception(At_Phrase(*source_, &f), "assertion failed");
    }
};
struct Assert_Metafunction : public Metafunction
{
    using Metafunction::Metafunction;
    virtual Shared<Meaning> call(const Call_Phrase& ph, Environ& env) override
    {
        auto args = ph.analyze_args(env);
        if (args.size() != 1)
            throw Exception(At_Phrase(ph, env), "assert: expecting 1 argument");
        return make<Assert_Action>(share(ph), args.front());
    }
};

struct Defined_Expression : public Just_Expression
{
    Shared<const Operation> expr_;
    Atom id_;

    Defined_Expression(
        Shared<const Phrase> source,
        Shared<const Operation> expr,
        Atom id)
    :
        Just_Expression(std::move(source)),
        expr_(std::move(expr)),
        id_(std::move(id))
    {
    }

    virtual Value eval(Frame& f) const override
    {
        auto val = expr_->eval(f);
        if (val.is_ref()) {
            auto& ref = val.get_ref_unsafe();
            return {ref.getfield(id_) != missing};
        } else {
            return {false};
        }
    }
};
struct Defined_Metafunction : public Metafunction
{
    using Metafunction::Metafunction;
    virtual Shared<Meaning> call(const Call_Phrase& ph, Environ& env) override
    {
        auto arg = analyze_op(*ph.args_, env);
        auto dot = dynamic_shared_cast<Dot_Expr>(arg);
        if (dot != nullptr)
            return make<Defined_Expression>(share(ph), dot->base_, dot->id_);
        throw Exception(At_Phrase(*ph.args_, env),
            "defined: argument must be `expression.identifier`");
    }
};

const Namespace&
builtin_namespace()
{
    static const Namespace names = {
    {"pi", make<Builtin_Value>(pi)},
    {"tau", make<Builtin_Value>(two_pi)},
    {"inf", make<Builtin_Value>(INFINITY)},
    {"null", make<Builtin_Value>(Value())},
    {"false", make<Builtin_Value>(Value(false))},
    {"true", make<Builtin_Value>(Value(true))},
    {"is_list", make<Builtin_Value>(Value{make<Is_List_Function>()})},
    {"bit", make<Builtin_Value>(Value{make<Bit_Function>()})},
    {"sqrt", make<Builtin_Value>(Value{make<Sqrt_Function>()})},
    {"log", make<Builtin_Value>(Value{make<Log_Function>()})},
    {"abs", make<Builtin_Value>(Value{make<Abs_Function>()})},
    {"floor", make<Builtin_Value>(Value{make<Floor_Function>()})},
    {"sin", make<Builtin_Value>(Value{make<Sin_Function>()})},
    {"cos", make<Builtin_Value>(Value{make<Cos_Function>()})},
    {"atan2", make<Builtin_Value>(Value{make<Atan2_Function>()})},
    {"max", make<Builtin_Value>(Value{make<Max_Function>()})},
    {"min", make<Builtin_Value>(Value{make<Min_Function>()})},
    {"mag", make<Builtin_Value>(Value{make<Mag_Function>()})},
    {"len", make<Builtin_Value>(Value{make<Len_Function>()})},
    {"file", make<Builtin_Value>(Value{make<File_Function>()})},
    {"shape2d", make<Builtin_Value>(Value{make<Shape2d_Function>()})},
    {"echo", make<Builtin_Meaning<Echo_Metafunction>>()},
    {"assert", make<Builtin_Meaning<Assert_Metafunction>>()},
    {"defined", make<Builtin_Meaning<Defined_Metafunction>>()},
    };
    return names;
}

} // namespace curv
