// -*-c++-*- csabase_location.h 
// -----------------------------------------------------------------------------
// Copyright 2012 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// -----------------------------------------------------------------------------

#if !defined(INCLUDED_CSABASE_LOCATION)
#define INCLUDED_CSABASE_LOCATION
#ident "$Id$"

#include <llvm/Support/raw_ostream.h>
#include <iosfwd>
#include <string>
#include <stddef.h>

// -----------------------------------------------------------------------------

namespace clang
{
    class SourceLocation;
    class SourceManager;
    class SourceRange;
}

namespace cool
{
    namespace csabase
    {
        class Location;
        llvm::raw_ostream& operator<< (llvm::raw_ostream&, Location const&);
        std::ostream& operator<< (std::ostream&, Location const&);
        bool operator== (Location const&, Location const&);
        bool operator!= (Location const&, Location const&);

        class Range;
        llvm::raw_ostream& operator<< (llvm::raw_ostream&, Range const&);
        std::ostream& operator<< (std::ostream&, Range const&);
        bool operator== (Range const&, Range const&);
        bool operator!= (Range const&, Range const&);
    }
}

// -----------------------------------------------------------------------------

class cool::csabase::Location
{
private:
    std::string d_file;
    size_t      d_line;
    size_t      d_column;

public:
    Location();
    Location(clang::SourceManager const& manager,
             clang::SourceLocation location);

    std::string file() const;
    size_t      line() const;
    size_t      column() const;

    bool operator< (cool::csabase::Location const& location) const;
};

class cool::csabase::Range
{
private:
    Location d_from;
    Location d_to;

public:
    Range();
    Range(clang::SourceManager const& manager,
          clang::SourceRange range);

    const Location& from() const;
    const Location& to() const;

    bool operator< (cool::csabase::Range const& range) const;
};

// -----------------------------------------------------------------------------

#endif
