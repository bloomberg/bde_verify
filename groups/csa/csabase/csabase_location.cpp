// -*-c++-*- csabase_location.cpp 
// -----------------------------------------------------------------------------
// Copyright 2011 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// -----------------------------------------------------------------------------

#include <csabase_location.h>
#include <ostream>
#ident "$Id$"

// -----------------------------------------------------------------------------

cool::csabase::Location::Location(std::string const& file, size_t line, size_t column):
    file_(file),
    line_(line),
    column_(column)
{
}

// -----------------------------------------------------------------------------

std::string
cool::csabase::Location::file() const
{
    return this->file_;
}

size_t
cool::csabase::Location::line() const
{
    return this->line_;
}

size_t
cool::csabase::Location::column() const
{
    return this->column_;
}

bool
cool::csabase::Location::operator< (cool::csabase::Location const& location) const
{
    return this->file_ != location.file_? this->file_ < location.file_
        :  this->line_ != location.line_? this->line_ < location.line_
        :  this->column_ < location.column_;
}

// -----------------------------------------------------------------------------

llvm::raw_ostream&
cool::csabase::operator<< (llvm::raw_ostream& out, cool::csabase::Location const& loc)
{
    return out << loc.file() << ":" << loc.line() << ":" << loc.column();
}

// -----------------------------------------------------------------------------

std::ostream&
cool::csabase::operator<< (std::ostream& out, cool::csabase::Location const& loc)
{
    return out << loc.file() << ":" << loc.line() << ":" << loc.column();
}
