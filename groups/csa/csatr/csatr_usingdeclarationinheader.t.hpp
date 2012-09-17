// csatr_usingdeclarationinheader.t.hpp                               -*-C++-*-
// ----------------------------------------------------------------------------
// Copyright 2012 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// ----------------------------------------------------------------------------

#if !defined(INCLUDED_CSATR_USINGDECLARATIONINHEADER)
#define INCLUDED_CSATR_USINGDECLARATIONINHEADER
#if !defined(INCLUDED_BDES_IDENT)
#  include <bdes_ident.h>
#endif
#ifndef INCLUDED_CSASCM_VERSION
#  include <csascm_version.h>
#endif

namespace cool
{
    namespace csamisc
    {
        class foo;
    }
    
    namespace csatr
    {
        using cool::csamisc::foo;
    }
}

using cool::csamisc::foo;

namespace cool
{
    namespace csatr
    {
        class UsingDeclarationInHeader
        {
        public:
            UsingDeclarationInHeader();
            void value() const;
        };
    }
}

inline cool::csatr::UsingDeclarationInHeader::UsingDeclarationInHeader()
{
    using cool::csamisc::foo;
}

inline void
cool::csatr::UsingDeclarationInHeader::value() const
{
    using cool::csamisc::foo;
}

#endif
