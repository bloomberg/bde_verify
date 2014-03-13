// csatr_usingdirectiveinheader.t.hpp                                 -*-C++-*-
// ----------------------------------------------------------------------------
// Copyright 2012 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// ----------------------------------------------------------------------------

#ifndef INCLUDED_CSATR_USINGDIRECTIVEINHEADER
#define INCLUDED_CSATR_USINGDIRECTIVEINHEADER
#if !defined(INCLUDED_BDES_IDENT)
#  include <bdes_ident.h>
#endif
#ifndef INCLUDED_CSASCM_VERSION
#  include <csascm_version.h>
#endif

namespace bde_verify {};
using namespace bde_verify;

namespace foo
{
    using namespace bde_verify;
}

namespace bde_verify
{
    namespace csatr
    {
        class UsingDirectiveInHeader
        {
        public:
            UsingDirectiveInHeader();
            void value() const;
        };
    }
}

bde_verify::csatr::UsingDirectiveInHeader::UsingDirectiveInHeader()
{
    using namespace foo;
}

void
bde_verify::csatr::UsingDirectiveInHeader::value() const
{
    using namespace foo;
}

#endif
