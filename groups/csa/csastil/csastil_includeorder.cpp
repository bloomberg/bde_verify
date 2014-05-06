// csastil_includeorder.cpp                                           -*-C++-*-
// ----------------------------------------------------------------------------
// Copyright 2012 Dietmar Kuehl http://www.dietmar-kuehl.de              
// Distributed under the Boost Software License, Version 1.0. (See file  
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).     
// ----------------------------------------------------------------------------

#include <csabase_analyser.h>
#include <csabase_location.h>
#include <csabase_ppobserver.h>
#include <csabase_registercheck.h>
#include <csabase_util.h>
#include <utils/array.hpp>
#include <cctype>
#include <algorithm>
#include <functional>
#include <utility>
#include <vector>

namespace CB = csabase;

// ----------------------------------------------------------------------------

static std::string const check_name("include-order");

// ----------------------------------------------------------------------------

namespace
{
    struct include_order
    {
        typedef std::vector<std::pair<std::string, clang::SourceLocation> > headers_t;
        headers_t d_header;
        headers_t d_source;
        void add_include(bool                         in_header,
                         std::string                  header,
                         clang::SourceLocation const& where)
        {
            std::string::size_type pos(header.find('.'));
            if (pos != header.npos) {
                header = header.substr(0u, pos);
            }
            
            headers_t& headers(in_header? d_header: d_source);
            if (headers.empty() || headers.back().first != header) {
                headers.push_back(std::make_pair(header, where));
            }
        }
    };
}

// ----------------------------------------------------------------------------

namespace
{
    struct has_prefix
    {
        typedef std::pair<std::string, clang::SourceLocation> const& argument_type;
        has_prefix(std::string const& prefix)
            : d_prefix(prefix)
        {
        }
        bool operator()(argument_type entry) const
        {
            return entry.first.find(d_prefix) == 0;
        }
        std::string d_prefix;
    };
}

// ----------------------------------------------------------------------------

static bool
first_is_greater(std::pair<std::string, clang::SourceLocation> const& entry0,
                 std::pair<std::string, clang::SourceLocation> const& entry1)
{
    return entry1.first < entry0.first;
}

// ----------------------------------------------------------------------------

static bool
is_component(std::pair<std::string, clang::SourceLocation> const& entry)
{
    std::string const&     header(entry.first);
    std::string::size_type start(header.find("a_") == 0 || header.find("e_") == 0? 2: 0);
    std::string::size_type under(header.find('_', 0));
    return under != header.npos && 4 < under - start && under - start < 8;
}

// ----------------------------------------------------------------------------

static void
check_order(CB::Analyser*                              analyser,
            std::string const&                         message,
            include_order::headers_t::const_iterator it,
            include_order::headers_t::const_iterator end)
{
    for (; end != (it = std::adjacent_find(it, end, &first_is_greater));
         ++it) {
        analyser->report(it[1].second, check_name, "SHO01",
                         "%0 header out of order")
            << message;
    }
}

static void
check_order(CB::Analyser*                              analyser,
            std::string const&                         message,
            include_order::headers_t::const_iterator it,
            include_order::headers_t::const_iterator section_end,
            include_order::headers_t::const_iterator end)
{
    check_order(analyser, message, it, section_end);
    for (it = section_end;
         end != (it = std::find_if(it, end, has_prefix(analyser->package() + "_")));
         ++it) {
        analyser->report(it->second, check_name, "SHO02",
                         "%0 header coming late")
            << message;
    }
}

static clang::SourceLocation const*
check_order(CB::Analyser*                   analyser,
            include_order::headers_t const& headers,
            bool                            header)
{
    clang::SourceLocation const* bdes_ident_location(0);
    if (headers.empty()) {
        analyser->report(clang::SourceLocation(), check_name, "SHO03",
                         header
                         ? "Header without include guard included"
                         : "Source without component include");
        return bdes_ident_location;
    }
    include_order::headers_t::const_iterator it(headers.begin());
    if (it->first != analyser->component() || it++ == headers.end()) {
        analyser->report(headers[0].second, check_name, "SHO04",
                         header
                         ? "Header without or with wrong include guard"
                         : "Source doesn't include component header first");
    }
    std::string ident =
        analyser->group() == "bsl" || analyser->group() == "bdl" ?
            "bsls_ident" : "bdes_ident";
    if (analyser->component() == ident ||
        (analyser->is_test_driver() && !header)) {
        if (it != headers.end()) {
            if (it->first == "bdes_ident") {
                bdes_ident_location = &it->second;
            }
            if (it->first == ident) {
                ++it;
            }
        }
    }
    else if (it == headers.end() || it->first != ident) {
        analyser->report((it == headers.end() ? it - 1: it)->second,
                         check_name, "SHO06",
                         "Missing include for %0.h")
            << ident;
    }
    else {
        if (it->first == "bdes_ident") {
            bdes_ident_location = &it->second;
        }
        ++it;
    }

    // These components are or are needed by b??scm_version, and so should not
    // themselves require inclusion of b??scm_version.
    static std::string const subscm[] = {
        "bdes_ident",
        "bdescm_versiontag",
        "bsls_buildtarget",
        "bsls_ident",
        "bsls_linkcoercion",
        "bsls_platform",
        "bslscm_version",
        "bslscm_versiontag",
    };

    std::string version = analyser->group() + "scm_version";
    if (   std::find(utils::begin(subscm),
                     utils::end(subscm),
                     analyser->component()) == utils::end(subscm)
        && header
        && (it == headers.end()
            || it->first != version
            || it++ == headers.end())) {
        analyser->report((it == headers.end() ? it - 1 : it)->second,
                         check_name, "SHO07",
                         "Missing include for %0.h")
            << version;
    }

    include_order::headers_t::const_iterator end
          = std::find_if(it, headers.end(),
                         std::not1(std::ptr_fun(&is_component)));
    include_order::headers_t::const_iterator package_end = std::find_if(
        it, end, std::not1(has_prefix(analyser->package() + "_")));
    check_order(analyser, "Package", it, package_end, end);
    include_order::headers_t::const_iterator group_end
          = std::find_if(it, end, std::not1(has_prefix(analyser->group())));
    check_order(analyser, "Group", package_end, group_end, end);
    check_order(analyser, "Component", group_end, end);

    return bdes_ident_location;
}

// ----------------------------------------------------------------------------

static inline bool
is_space(unsigned char c)
{
    return std::isspace(c);
}

// ----------------------------------------------------------------------------

namespace
{
    std::string const prefix0("included_");
    std::string const prefix1("!defined(included_");
    std::string const prefix2("!definedincluded_");

    struct binder
    {
        binder(csabase::Analyser* analyser)
            : d_analyser(analyser)
        {
        }

        void operator()(clang::SourceLocation,
                        clang::SourceRange range) const // onIf
        {
            if (!d_analyser->is_component(range.getBegin())) {
                return;
            }
            include_order& data(d_analyser->attachment<include_order>()); 
            char const* begin(d_analyser->manager().getCharacterData(range.getBegin()));
            char const* end(d_analyser->manager().getCharacterData(range.getEnd()));
            std::string value(begin, end);
            value.erase(std::remove_if(value.begin(), value.end(),
                                       &is_space), value.end());
            value = csabase::to_lower(value);
            if (value.find(prefix1) == 0 && value[value.size() - 1] == ')') {
                data.add_include(d_analyser->is_component_header(range.getBegin()),
                                 value.substr(prefix1.size(), value.size() - prefix1.size() - 1),
                                 range.getBegin());
            }
            else if (value.find(prefix2) == 0) {
                data.add_include(d_analyser->is_component_header(range.getBegin()),
                                 value.substr(prefix2.size()),
                                 range.getBegin());
            }
        }
        void operator()(clang::SourceLocation where,
                        clang::Token const& token) const // onIfndef
        {
            if (!d_analyser->is_component(token.getLocation())) {
                return;
            }

            include_order& data(d_analyser->attachment<include_order>());
            if (clang::IdentifierInfo const* id = token.getIdentifierInfo())
            {
                std::string value(id->getNameStart());
                value = csabase::to_lower(value);
                if (value.find(prefix0) == 0) {
                    data.add_include(d_analyser->is_component_header(token.getLocation()),
                                     value.substr(prefix0.size()),
                                     token.getLocation());
                }
            }
        }
        void operator()(clang::SourceLocation where,
                        bool,
                        std::string const& name)
        {
            if (d_analyser->is_component(where)) {
                include_order& data(d_analyser->attachment<include_order>()); 
                bool in_header(d_analyser->is_component_header(where));
                data.add_include(in_header, name, where);
            }
        }
        void operator()()  // translation unit done
        {
            include_order& data(d_analyser->attachment<include_order>());
            clang::SourceLocation const* header_bdes_ident(
                check_order(d_analyser, data.d_header, true));
            clang::SourceLocation const* source_bdes_ident(
                check_order(d_analyser, data.d_source, false));
            if ((header_bdes_ident == 0) != (source_bdes_ident == 0)) {
                d_analyser->report(*(header_bdes_ident ? header_bdes_ident :
                                                         source_bdes_ident),
                                   check_name, "SHO08",
                                   "bdes_ident.h is used inconsistently with "
                                   "the %0")
                    << (header_bdes_ident ? "source" : "header");
            }
        }

        csabase::Analyser* d_analyser;
    };
}

// ----------------------------------------------------------------------------

static void
subscribe(csabase::Analyser& analyser, csabase::Visitor&, csabase::PPObserver& observer)
{
    analyser.onTranslationUnitDone += binder(&analyser);
    observer.onInclude             += binder(&analyser);
    observer.onIfndef              += binder(&analyser);
    observer.onIf                  += binder(&analyser);
}

// ----------------------------------------------------------------------------

static csabase::RegisterCheck register_observer(check_name, &subscribe);
