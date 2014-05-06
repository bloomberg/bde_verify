// csabase_csadebug.cpp                                               -*-C++-*-
// -----------------------------------------------------------------------------
// Copyright 2012 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// -----------------------------------------------------------------------------

#include <csabase_debug.h>
#include <llvm/Support/raw_ostream.h>
#ident "$Id$"

// -----------------------------------------------------------------------------

namespace
{
    unsigned int indent(0);
    bool         do_debug(false);
}

// -----------------------------------------------------------------------------

namespace
{
    static llvm::raw_ostream&
    start(unsigned int depth)
    {
        llvm::raw_ostream& out(llvm::errs());
        for (unsigned int i(0); i != depth; ++i)
        {
            out << " ";
        }
        return out;
    }
}

// -----------------------------------------------------------------------------

void
csabase::Debug::set_debug(bool value)
{
    do_debug = value;
}

bool
csabase::Debug::get_debug()
{
    return do_debug;
}

// -----------------------------------------------------------------------------

csabase::Debug::Debug(char const* message, bool nest):
    message_(message),
    nest_(nest)
{
    if (do_debug)
    {
        start(::indent) << (nest_? "\\ ": "| ") << "'" << message_ << "'\n";
        ::indent += nest;
    }
}

csabase::Debug::~Debug()
{
    if (do_debug && nest_)
    {
        start(::indent -= nest_) << (nest_? "/ ": "| ") << message_ << "\n";
    }
}

// -----------------------------------------------------------------------------

namespace
{
    llvm::raw_null_ostream dummy_stream;
}

llvm::raw_ostream&
csabase::Debug::indent() const
{
    return do_debug? start(::indent) << "| ": dummy_stream;
}
