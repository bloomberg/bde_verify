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
cool::csabase::Debug::set_debug(bool value)
{
    do_debug = value;
}

bool
cool::csabase::Debug::get_debug()
{
    return do_debug;
}

// -----------------------------------------------------------------------------

cool::csabase::Debug::Debug(char const* message, bool nest):
    message_(message),
    nest_(nest)
{
    if (do_debug)
    {
        start(::indent) << (this->nest_? "\\ ": "| ") << "'" << this->message_ << "'\n";
        ::indent += nest;
    }
}

cool::csabase::Debug::~Debug()
{
    if (do_debug && this->nest_)
    {
        start(::indent -= this->nest_) << (this->nest_? "/ ": "| ") << message_ << "\n";
    }
}

// -----------------------------------------------------------------------------

namespace
{
    llvm::raw_null_ostream dummy_stream;
}

llvm::raw_ostream&
cool::csabase::Debug::indent() const
{
    return do_debug? start(::indent) << "| ": dummy_stream;
}
