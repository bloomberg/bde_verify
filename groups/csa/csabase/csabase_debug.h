// csabase_csadebug.h                                                 -*-C++-*-
// -----------------------------------------------------------------------------
// Copyright 2012 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// -----------------------------------------------------------------------------

#if !defined(INCLUDED_CSABASE_DEBUG)
#define INCLUDED_CSABASE_DEBUG 1
#ident "$Id$"

#include <llvm/Support/raw_ostream.h>

// -----------------------------------------------------------------------------

namespace cool
{
    namespace csabase
    {
        class Debug;
    }
}

// -----------------------------------------------------------------------------

class cool::csabase::Debug
{
public:
    static void set_debug(bool);
    static bool get_debug();

    Debug(char const*, bool nest = true);
    ~Debug();
    template <typename T> llvm::raw_ostream& operator<< (T const& value) const;

private:
    Debug(Debug const&);
    void operator= (Debug const&);
    llvm::raw_ostream& indent() const;

    char const*  message_;
    unsigned int nest_;
};

// -----------------------------------------------------------------------------

template <typename T>
llvm::raw_ostream&
cool::csabase::Debug::operator<< (T const& value) const
{
    return indent() << value;
}

// -----------------------------------------------------------------------------

#endif
