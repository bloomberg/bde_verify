// csatr_usingdirectiveinheader.t.hpp                                 -*-C++-*-
// ----------------------------------------------------------------------------
// Copyright 2012 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// ----------------------------------------------------------------------------

#ifndef INCLUDED_CSATR_USINGDIRECTIVEINHEADER
#define INCLUDED_CSATR_USINGDIRECTIVEINHEADER
#ident "$Id$"

namespace cool {};
using namespace cool;

namespace foo
{
    using namespace cool;
}

namespace cool
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

cool::csatr::UsingDirectiveInHeader::UsingDirectiveInHeader()
{
    using namespace foo;
}

void
cool::csatr::UsingDirectiveInHeader::value() const
{
    using namespace foo;
}

#endif
