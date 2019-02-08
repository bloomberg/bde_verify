namespace bsl {
struct allocator_arg_t { constexpr allocator_arg_t() { } };
const allocator_arg_t allocator_arg;
}

namespace BloombergLP { namespace bslma { class Allocator { }; } }

using namespace BloombergLP::bslma;
using namespace bsl;
// BDE_VERIFY pragma: -AL01
// BDE_VERIFY pragma: -AT02
// BDE_VERIFY pragma: -AP02
struct Last {
    Last(Allocator * = 0);
    Last(const Last &, Allocator * = 0);
};

struct First {
    First(allocator_arg_t, Allocator&);
    First(allocator_arg_t, Allocator&, const First &);
    First();
    First(const First &);
};

namespace N1 {
struct LastC {
    LastC(Allocator * = 0);
    LastC(const LastC &, Allocator * = 0);
    Last d_last;
    First d_first;
};

struct FirstC {
    FirstC(allocator_arg_t, Allocator&);
    FirstC(allocator_arg_t, Allocator&, const FirstC &);
    Last d_last;
    First d_first;
};

LastC::LastC(Allocator *alloc)
: d_last(alloc)
, d_first(allocator_arg, *alloc)
{
}

LastC::LastC(const LastC &other, Allocator *alloc)
: d_last(other.d_last, alloc)
, d_first(allocator_arg, *alloc, other.d_first)
{
}

FirstC::FirstC(allocator_arg_t, Allocator& alloc)
: d_last(&alloc)
, d_first(allocator_arg, alloc)
{
}

FirstC::FirstC(allocator_arg_t, Allocator& alloc, const FirstC &other)
: d_last(other.d_last, &alloc)
, d_first(allocator_arg, alloc, other.d_first)
{
}
}

namespace N2 {
struct LastC {
    LastC(Allocator * = 0);
    LastC(const LastC &, Allocator * = 0);
    Last d_last;
    First d_first;
};

struct FirstC {
    FirstC(allocator_arg_t, Allocator&);
    FirstC(allocator_arg_t, Allocator&, const FirstC &);
    Last d_last;
    First d_first;
};

LastC::LastC(Allocator *alloc)
: d_last()
, d_first()
{
}

LastC::LastC(const LastC &other, Allocator *)
: d_last(other.d_last)
, d_first(other.d_first)
{
}

FirstC::FirstC(allocator_arg_t, Allocator&)
: d_last()
, d_first()
{
}

FirstC::FirstC(allocator_arg_t, Allocator&, const FirstC &other)
: d_last(other.d_last)
, d_first(other.d_first)
{
}
}
