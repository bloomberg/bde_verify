// joexx_traits.h                                                     -*-C++-*-
#ifndef INCLUDED_JOEXX_TRAITS
#define INCLUDED_JOEXX_TRAITS

#ifndef INCLUDED_BSLMF_ISTRIVIALLYCOPYABLE
#include <bslmf_istriviallycopyable.h>
#endif

namespace BloombergLP {
namespace joexx {
class Traits { int a[7]; };
}
}

namespace bsl {
template <>
struct is_trivially_copyable<BloombergLP::joexx::Traits> : true_type
{
};
}

#endif
