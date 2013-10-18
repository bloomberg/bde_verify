// csamisc_includeguard.cpp                                           -*-C++-*-
// -----------------------------------------------------------------------------
// Copyright 2011 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// -----------------------------------------------------------------------------

#include "framework/analyser.hpp"
#include "framework/pp_observer.hpp"
#include "framework/register_check.hpp"
#include "framework/location.hpp"
#include "cool/array.hpp"
#include "clang/Basic/SourceLocation.h"
#include <algorithm>
#include <string>
#include <ctype.h>
#ident "$Id$"

// -----------------------------------------------------------------------------

static std::string const check_name("include-guard");

// -----------------------------------------------------------------------------

namespace
{
    bool
    is_space(unsigned char c)
    {
        return isspace(c);
    }
    char
    to_upper(unsigned char c)
    {
        return toupper(c);
    }
}

// -----------------------------------------------------------------------------

namespace
{
    std::string
    get_string(cool::csabase::Analyser* analyser, clang::SourceLocation begin, clang::SourceLocation end)
    {
        return std::string(clang::FullSourceLoc(begin, analyser->manager()).getCharacterData(),
                           clang::FullSourceLoc(end, analyser->manager()).getCharacterData());
    }
}

// -----------------------------------------------------------------------------

namespace
{
    static std::string const suffixes[] = { ".h", ".H", ".hh", ".h++", ".hpp", ".hxx" };

    void
    inspect_guard_name(cool::csabase::Analyser* analyser, clang::SourceLocation begin, std::string const& name)
    {
        std::string file(analyser->get_location(begin).file());
        std::string::size_type separator(file.rfind('/'));
        if (file.npos != separator)
        {
            file = file.substr(separator + 1);
        }
        separator = file.rfind('.');
        std::string suffix(file.npos == separator? "": file.substr(separator));
        if (cool::end(suffixes) == std::find(cool::begin(suffixes), cool::end(suffixes), suffix))
        {
            analyser->report(begin, ::check_name, "unknown header file suffix: '" + suffix + "'");
        }
        else
        {
            file = "INCLUDED_" + file.substr(0, separator);
            if (2 <= file.size() && ".t" == file.substr(file.size() - 2))
            {
                file[file.size() - 2] = '_';
            }
            std::transform(file.begin(), file.end(), file.begin(), ::to_upper);
            if (file != name)
            {
                analyser->report(begin, ::check_name, "expected include guard '" + name + "'");
            }
        }
    }
}

// -----------------------------------------------------------------------------

namespace
{
    std::string const not_defined("!defined(");
    void on_if(cool::csabase::Analyser* analyser, clang::SourceRange range)
    {
        std::string expr(::get_string(analyser, range.getBegin(), range.getEnd()));
        expr.erase(std::remove_if(expr.begin(), expr.end(), ::is_space), expr.end());
        if (0 == expr.find(::not_defined) && *(expr.end() - 1) == ')')
        {
            ::inspect_guard_name(analyser, range.getBegin(),
                                 expr.substr(::not_defined.size(), expr.size() - ::not_defined.size() - 1));
        }
    }
}

// -----------------------------------------------------------------------------

namespace
{
    void on_ifndef(cool::csabase::Analyser* analyser, clang::Token token)
    {
        //-dk:TODO remove llvm::errs() << "onIfndef\n";
        ::inspect_guard_name(analyser, token.getLocation(), ::get_string(analyser, token.getLocation(), token.getLastLoc()));
    }
}

// -----------------------------------------------------------------------------

static void
subscribe(cool::csabase::Analyser& analyser, cool::csabase::Visitor&, cool::csabase::PPObserver& observer)
{
    observer.onIf      += std::bind1st(std::ptr_fun(::on_if), &analyser);
    observer.onIfndef  += std::bind1st(std::ptr_fun(::on_ifndef), &analyser);
}

// -----------------------------------------------------------------------------

static cool::csabase::RegisterCheck register_observer(check_name, &::subscribe);
