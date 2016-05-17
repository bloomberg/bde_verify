// csabase_debug.cpp                                                  -*-C++-*-

#include <csabase_debug.h>
#include <llvm/Support/raw_ostream.h>

// -----------------------------------------------------------------------------

namespace
{
    unsigned int level(0);
    bool         do_debug(false);
}

// -----------------------------------------------------------------------------

namespace
{
    static llvm::raw_ostream& start(unsigned int depth)
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

void csabase::Debug::set_debug(bool value)
{
    do_debug = value;
}

bool csabase::Debug::get_debug()
{
    return do_debug;
}

// -----------------------------------------------------------------------------

csabase::Debug::Debug(char const* message, bool nest)
: message_(message)
, nest_(nest)
{
    if (do_debug)
    {
        start(level) << (nest_ ? "\\ " : "| ") << "'" << message_ << "'\n";
        level += nest;
    }
}

csabase::Debug::~Debug()
{
    if (do_debug && nest_)
    {
        start(level -= nest_) << (nest_ ? "/ " : "| ") << message_ << "\n";
    }
}

// -----------------------------------------------------------------------------

namespace
{
    llvm::raw_null_ostream dummy_stream;
}

llvm::raw_ostream& csabase::Debug::indent() const
{
    return do_debug ? start(level) << "| ": dummy_stream;
}

// ----------------------------------------------------------------------------
// Copyright (C) 2014 Bloomberg Finance L.P.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// ----------------------------- END-OF-FILE ----------------------------------
