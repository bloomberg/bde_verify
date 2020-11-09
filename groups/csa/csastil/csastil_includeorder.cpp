// csastil_includeorder.cpp                                           -*-C++-*-

#include <clang/Basic/IdentifierTable.h>
#include <clang/Basic/SourceLocation.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Lex/Token.h>
#include <csabase_analyser.h>
#include <csabase_config.h>
#include <csabase_debug.h>
#include <csabase_diagnostic_builder.h>
#include <csabase_ppobserver.h>
#include <csabase_registercheck.h>
#include <csabase_report.h>
#include <csabase_util.h>
#include <utils/array.hpp>
#include <utils/event.hpp>
#include <utils/function.hpp>
#include <algorithm>
#include <cctype>
#include <functional>
#include <string>
#include <utility>
#include <vector>

namespace csabase { class Visitor; }

using namespace clang;
using namespace csabase;

// ----------------------------------------------------------------------------

static std::string const check_name("include-order");

// ----------------------------------------------------------------------------

namespace
{

struct data
{
    typedef std::vector<std::pair<std::string, SourceLocation>> headers_t;

    bool d_saw_header;
    bool d_saw_source;
    headers_t d_header;
    headers_t d_source;

    data()
    : d_saw_header(false)
    , d_saw_source(false)
    {
    }

    void add_include(bool                  in_header,
                     std::string           header,
                     SourceLocation const& where)
    {
        std::string::size_type pos(header.find('.'));
        if (pos != header.npos) {
            header = header.substr(0u, pos);
        }

        headers_t &headers = in_header? d_header : d_source;
        if (headers.empty() || headers.back().first != header) {
            headers.push_back(std::make_pair(header, where));
        }
    }
};

struct has_prefix
{
    typedef std::pair<std::string, SourceLocation> const &argument_type;

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
first_is_greater(std::pair<std::string, SourceLocation> const& entry0,
                 std::pair<std::string, SourceLocation> const& entry1)
{
    return entry1.first < entry0.first;
}

// ----------------------------------------------------------------------------

static bool
is_component(std::pair<std::string, SourceLocation> const& entry)
{
    std::string const&     header(entry.first);
    std::string::size_type start(
        header.find("a_") == 0 || header.find("e_") == 0 ? 2 : 0);
    std::string::size_type under(header.find('_', 0));
    return under != header.npos && 4 < under - start && under - start < 8;
}

// ----------------------------------------------------------------------------

static bool
is_ident(llvm::StringRef name, llvm::StringRef ident)
{
    return name == ident || name == "bdes_ident" || name == "bsls_ident";
}

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

struct report : public Report<data>
{
    INHERIT_REPORT_CTOR(report, Report, data);

    void operator()(SourceLocation, SourceRange range) const // onIf
    {
        if (!a.is_component(range.getBegin())) {
            return;
        }
        std::string value = a.get_source(range).str();
        value.erase(std::remove_if(value.begin(), value.end(), &is_space),
                    value.end());
        value = to_lower(value);
        if (value.find(prefix1) == 0 && value[value.size() - 1] == ')') {
            d.add_include(
                a.is_component_header(range.getBegin()),
                value.substr(
                    prefix1.size(), value.size() - prefix1.size() - 1),
                range.getBegin());
        }
        else if (value.find(prefix2) == 0) {
            d.add_include(
                a.is_component_header(range.getBegin()),
                value.substr(prefix2.size()),
                range.getBegin());
        }
    }

    void operator()(SourceLocation where, Token const& token) const // onIfndef
    {
        if (!a.is_component(token.getLocation())) {
            return;
        }

        if (IdentifierInfo const* id = token.getIdentifierInfo())
        {
            std::string value(id->getNameStart());
            value = to_lower(value);
            if (value.find(prefix0) == 0) {
                d.add_include(
                    a.is_component_header(token.getLocation()),
                    value.substr(prefix0.size()),
                    token.getLocation());
            }
        }
    }

    void operator()(SourceLocation where, bool, std::string const& name)
    {
        if (a.is_component(where)) {
            bool in_header = a.is_component_header(where);
            d.add_include(in_header, name, where);
        }
    }

    void operator()(SourceLocation                now,
                    PPCallbacks::FileChangeReason reason,
                    SrcMgr::CharacteristicKind    type,
                    FileID                        prev)
    {
        if (reason == PPCallbacks::EnterFile) {
            if (!d.d_saw_header || !d.d_saw_source) {
                if (a.is_component_header(now)) {
                    d.d_saw_header = true;
                }
                else if (a.is_component(now)) {
                    d.d_saw_source = true;
                }
            }
        }
    }

    void operator()()  // translation unit done
    {
        if (a.is_test_driver() || a.is_component_header(a.toplevel())) {
            return;
        }
        SourceLocation const *header_ident = check_order(d.d_header, true);
        SourceLocation const *source_ident = check_order(d.d_source, false);
        if (d.d_saw_header && d.d_saw_source &&
            header_ident && !source_ident) {
            a.report(*header_ident, check_name, "SHO08",
                     "Component header includes '..._ident.h' but component "
                     "source does not");
        }
        if (d.d_saw_header && d.d_saw_source &&
            !header_ident && source_ident) {
            a.report(*source_ident, check_name, "SHO08",
                     "Component source includes '..._ident.h' but header does "
                     "not");
        }
    }

    void check_order(std::string const&              message,
                     data::headers_t::const_iterator it,
                     data::headers_t::const_iterator end)
    {
        for (; end != (it = std::adjacent_find(it, end, &first_is_greater));
             ++it) {
            auto report = a.report(it[1].second, check_name, "SHO01",
                     "%0 header out of order");
    
            report << message;
        }
    }

    void check_order(std::string const&              message,
                     data::headers_t::const_iterator it,
                     data::headers_t::const_iterator section_end,
                     data::headers_t::const_iterator end)
    {
        check_order(message, it, section_end);
        for (it = section_end;
             end !=
             (it = std::find_if(it, end, has_prefix(a.package() + "_")));
             ++it) {
            auto report = a.report(it->second, check_name, "SHO02",
                     "%0 header coming late");
            report << message;
        }
    }

    SourceLocation const *check_order(data::headers_t const& headers,
                                      bool                   header)
    {
        SourceLocation const *ident_location(0);
        if (headers.empty()) {
            a.report(SourceLocation(), check_name, "SHO03",
                     header ? "Header without include guard included"
                            : "Source without component include");
            return ident_location;
        }
        data::headers_t::const_iterator it(headers.begin());
        if (it->first != a.component() || it++ == headers.end()) {
            a.report(headers[0].second, check_name, "SHO04",
                     header ? "Header without or with wrong include guard"
                            : "Source doesn't include component header first");
        }
        std::string ident = a.config()->value("ident_header");
        if (ident.empty()) {
            ident = "bsls_ident";
        }
        if (is_ident(a.component(), ident) ||
            (a.is_test_driver() && !header)) {
            if (it != headers.end()) {
                if (is_ident(it->first, ident)) {
                    ident_location = &it->second;
                    ++it;
                }
            }
        }
        else if (it == headers.end() || !is_ident(it->first, ident)) {
            auto report = a.report((it == headers.end() ? it - 1 : it)->second,
                     check_name, "SHO06",
                     "Missing include for %0.h");
            report << ident;
        }
        else {
            if (is_ident(it->first, ident)) {
                ident_location = &it->second;
            }
            ++it;
        }

        // These components are or are needed by b??scm_version, and so should
        // not themselves require inclusion of b??scm_version.
        static std::string const subscm[] = {
            "bdes_ident",
            "bdescm_versiontag",
            "bdlscm_version",
            "bdlscm_versiontag",
            "bsls_buildtarget",
            "bsls_ident",
            "bsls_linkcoercion",
            "bsls_platform",
            "bslscm_version",
            "bslscm_versiontag",
        };

        std::string version = a.group().size() ? a.group() + "scm_version"
                                               : a.package() + "_version";
        std::string vh = version + ".h";
        if ((a.package() == "bsls" || a.package() == "bdls" ||
             (&headers == &d.d_header &&
              a.component() == version)) &&
            header && it != headers.end() && it->first == version) {
            auto report = a.report(it->second, check_name, "SHO09",
                     "'%0' components should not include '%1'");
            report << a.package() << vh;
        }

        if (a.package() != "bsls" && a.package() != "bdls" &&
            (&headers != &d.d_header || a.component() != version) &&
            std::find(utils::begin(subscm),
                      utils::end(subscm),
                      a.component()) == utils::end(subscm) &&
            header &&
            (it == headers.end() || it->first != version ||
             it++ == headers.end())) {
            data::headers_t::const_iterator last_it(headers.begin());
            while (last_it != headers.end() && last_it->first != version) {
                ++last_it;
            }
            if (last_it == headers.end()) {
                auto report = a.report((it == headers.end() ? it - 1 : it)->second,
                         check_name, "SHO07",
                         "Missing include for %0");
                report << vh;
            }
            else {
                auto r1 = a.report((it == headers.end() ? it - 1 : it)->second,
                         check_name, "SHO07",
                         "Include for %0 should go here");
                r1 << vh;
                auto r2 = a.report(last_it->second, check_name, "SHO07",
                         "Include for %0 is here",
                         true, DiagnosticIDs::Note);
                r2 << vh;
            }
        }

        data::headers_t::const_iterator end = std::find_if(
            it, headers.end(), std::not1(std::ptr_fun(&is_component)));
        data::headers_t::const_iterator package_end = std::find_if(
            it, end, std::not1(has_prefix(a.package() + "_")));
        check_order("Package", it, package_end, end);
        data::headers_t::const_iterator group_end =
            std::find_if(it, end, std::not1(has_prefix(a.group())));
        check_order("Group", package_end, group_end, end);
        check_order("Component", group_end, end);

        return ident_location;
    }
};

}

// ----------------------------------------------------------------------------

static void
subscribe(Analyser& analyser, Visitor&, PPObserver& observer)
{
    analyser.onTranslationUnitDone += report(analyser);
    observer.onInclude             += report(analyser);
    observer.onIfndef              += report(analyser);
    observer.onIf                  += report(analyser);
    observer.onPPFileChanged       += report(analyser);
}

// ----------------------------------------------------------------------------

static RegisterCheck register_observer(check_name, &subscribe);

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
