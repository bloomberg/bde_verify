// csaaq_transitiveincludes.cpp                                       -*-C++-*-

#include <clang/AST/RecursiveASTVisitor.h>

#include <clang/Lex/MacroInfo.h>
#include <clang/Lex/Token.h>

#include <csabase_analyser.h>
#include <csabase_debug.h>
#include <csabase_filenames.h>
#include <csabase_ppobserver.h>
#include <csabase_registercheck.h>
#include <csabase_report.h>
#include <csabase_visitor.h>

#include <llvm/ADT/Hashing.h>
#include <llvm/ADT/StringRef.h>

#include <llvm/Support/Regex.h>

#include <cctype>
#include <map>
#include <set>

using namespace csabase;
using namespace clang;

namespace std {

template <typename A, typename B> struct hash<pair<A, B>> {
    size_t operator()(const pair<A, B>& p) const {
        return llvm::hash_combine(hash<A>()(p.first), hash<B>()(p.second));
    }
};

template <typename A, typename B, typename C> struct hash<tuple<A, B, C>> {
    size_t operator()(const tuple<A, B, C>& t) const {
        return llvm::hash_combine(hash<A>()(get<0>(t)),
                                  hash<B>()(get<1>(t)),
                                  hash<C>()(get<2>(t)));
    }
};

template <> struct hash<SourceLocation> {
    size_t operator()(const SourceLocation& sl) const {
        return sl.getRawEncoding();
    }
};

template <> struct hash<llvm::StringRef> {
    size_t operator()(const llvm::StringRef& sr) const {
        return llvm::hash_value(sr);
    }
};

template <> struct hash<FileID> {
    size_t operator()(const FileID& fid) const {
        return fid.getHashValue();
    }
};

}

// ----------------------------------------------------------------------------

static std::string const check_name("transitive-includes");

// ----------------------------------------------------------------------------

namespace
{

std::set<llvm::StringRef> &top_level_files()
{
    static std::set<llvm::StringRef> s;
    if (!s.size()) {
#undef  X
#define X(n)                                                                  \
    s.insert(#n);                                                             \
    s.insert("bsl_" #n ".h");                                                 \
    s.insert("stl_" #n ".h");
    X(algorithm)     X(array)         X(atomic)           X(bitset)
    X(chrono)        X(codecvt)       X(complex)          X(condition_variable)
    X(deque)         X(exception)     X(forward_list)     X(fstream)
    X(functional)    X(future)        X(initializer_list) X(iomanip)
    X(ios)           X(iosfwd)        X(iostream)         X(istream)
    X(iterator)      X(limits)        X(list)             X(locale)
    X(map)           X(memory)        X(mutex)            X(new)
    X(numeric)       X(ostream)       X(queue)            X(random)
    X(ratio)         X(regex)         X(scoped_allocator) X(set)
    X(sstream)       X(stack)         X(stdexcept)        X(streambuf)
    X(string)        X(strstream)     X(system_error)     X(thread)
    X(tuple)         X(type_traits)   X(typeindex)        X(typeinfo)
    X(unordered_map) X(unordered_set) X(utility)          X(valarray)
    X(vector)
#undef  X

#undef  X
#define X(n)                                                                  \
    s.insert("c" #n);                                                         \
    s.insert(#n ".h");                                                        \
    s.insert("bsl_c" #n ".h");                                                \
    s.insert("bsl_c_" #n ".h");
    X(assert)   X(complex) X(ctype)    X(errno)  X(fenv)    X(float)
    X(inttypes) X(iso646)  X(iso646)   X(limits) X(locale)  X(math)
    X(setjmp)   X(signal)  X(stdalign) X(stdarg) X(stdbool) X(stddef)
    X(stdint)   X(stdio)   X(stdlib)   X(string) X(tgmath)  X(time)
    X(uchar)    X(wchar)   X(wctype)
#undef  X

    s.insert("vstring.h");
    s.insert("unistd.h");
    }

    return s;
}

std::vector<llvm::StringRef> &top_level_prefixes()
{
    static std::vector<llvm::StringRef> s;
    if (!s.size()) {
#undef  X
#define X(n) s.emplace_back(n);
    X("bdlat_")  X("bdlb_")   X("bdlc_")  X("bdlde_") X("bdldfp_")
    X("bdlf_")   X("bdlma_")  X("bdlmt_") X("bdlqq_") X("bdls_")
    X("bdlsb_")  X("bdlscm_") X("bdlsu_") X("bdlt_")  X("bslalg_")
    X("bslfwd_") X("bslh_")   X("bslim_") X("bslma_") X("bslmf_")
    X("bsls_")   X("bslscm_") X("bsltf_") X("bslx_")
#undef X
    };

    return s;
}

bool is_top_level(llvm::StringRef name)
{
    if (top_level_files().count(name)) {
        return true;
    }
    for (auto s : top_level_prefixes()) {
        if (name.startswith(s)) {
            return true;
        }
    }
    return false;
}

std::map<llvm::StringRef, llvm::StringRef> &mapped_files()
{
    static std::map<llvm::StringRef, llvm::StringRef> s;
    if (!s.size()) {
#undef  X
#define X(a, b) s[a] = b;
    X("/bits/algorithmfwd.h",                 "algorithm"          )
    X("/bits/alloc_traits.h",                 "memory"             )
    X("/bits/allocator.h",                    "memory"             )
    X("/bits/atomic_base.h",                  "atomic"             )
    X("/bits/atomic_lockfree_defines.h",      "atomic"             )
    X("/bits/auto_ptr.h",                     "memory"             )
    X("/bits/backward_warning.h",             "iosfwd"             )
    X("/bits/basic_file.h",                   "ios"                )
    X("/bits/basic_ios.h",                    "ios"                )
    X("/bits/basic_ios.tcc",                  "ios"                )
    X("/bits/basic_string.h",                 "string"             )
    X("/bits/basic_string.tcc",               "string"             )
    X("/bits/bessel_function.tcc",            "cmath"              )
    X("/bits/beta_function.tcc",              "cmath"              )
    X("/bits/binders.h",                      "functional"         )
    X("/bits/boost_concept_check.h",          "iterator"           )
    X("/bits/c++0x_warning.h",                "iosfwd"             )
    X("/bits/c++allocator.h",                 "memory"             )
    X("/bits/c++config.h",                    "iosfwd"             )
    X("/bits/c++io.h",                        "ios"                )
    X("/bits/c++locale.h",                    "locale"             )
    X("/bits/cast.h",                         "pointer.h"          )
    X("/bits/char_traits.h",                  "string"             )
    X("/bits/codecvt.h",                      "locale"             )
    X("/bits/concept_check.h",                "iterator"           )
    X("/bits/cpp_type_traits.h",              "type_traits"        )
    X("/bits/cpu_defines.h",                  "iosfwd"             )
    X("/bits/ctype_base.h",                   "locale"             )
    X("/bits/ctype_inline.h",                 "locale"             )
    X("/bits/cxxabi_forced.h",                "cxxabi.h"           )
    X("/bits/cxxabi_tweaks.h",                "cxxabi.h"           )
    X("/bits/decimal.h",                      "decimal"            )
    X("/bits/deque.tcc",                      "deque"              )
    X("/bits/ell_integral.tcc",               "cmath"              )
    X("/bits/error_constants.h",              "system_error"       )
    X("/bits/exception_defines.h",            "exception"          )
    X("/bits/exception_ptr.h",                "exception"          )
    X("/bits/exp_integral.tcc",               "cmath"              )
    X("/bits/forward_list.h",                 "forward_list"       )
    X("/bits/forward_list.tcc",               "forward_list"       )
    X("/bits/fstream.tcc",                    "fstream"            )
    X("/bits/functexcept.h",                  "exception"          )
    X("/bits/functional_hash.h",              "functional"         )
    X("/bits/gamma.tcc",                      "cmath"              )
    X("/bits/gslice.h",                       "valarray"           )
    X("/bits/gslice_array.h",                 "valarray"           )
    X("/bits/hash_bytes.h",                   "functional"         )
    X("/bits/hashtable.h",                    "unordered_map"      )
    X("/bits/hashtable_policy.h",             "unordered_map"      )
    X("/bits/hypergeometric.tcc",             "cmath"              )
    X("/bits/indirect_array.h",               "valarray"           )
    X("/bits/ios_base.h",                     "ios"                )
    X("/bits/istream.tcc",                    "istream"            )
    X("/bits/legendre_function.tcc",          "cmath"              )
    X("/bits/list.tcc",                       "list"               )
    X("/bits/locale_classes.h",               "locale"             )
    X("/bits/locale_classes.tcc",             "locale"             )
    X("/bits/locale_facets.h",                "locale"             )
    X("/bits/locale_facets.tcc",              "locale"             )
    X("/bits/locale_facets_nonio.h",          "locale"             )
    X("/bits/locale_facets_nonio.tcc",        "locale"             )
    X("/bits/localefwd.h",                    "locale"             )
    X("/bits/mask_array.h",                   "valarray"           )
    X("/bits/memoryfwd.h",                    "memory"             )
    X("/bits/messages_members.h",             "locale"             )
    X("/bits/modified_bessel_func.tcc",       "cmath"              )
    X("/bits/move.h",                         "utility"            )
    X("/bits/nested_exception.h",             "exception"          )
    X("/bits/opt_random.h",                   "random"             )
    X("/bits/os_defines.h",                   "iosfwd"             )
    X("/bits/ostream.tcc",                    "ostream"            )
    X("/bits/ostream_insert.h",               "ostream"            )
    X("/bits/poly_hermite.tcc",               "cmath"              )
    X("/bits/poly_laguerre.tcc",              "cmath"              )
    X("/bits/postypes.h",                     "iosfwd"             )
    X("/bits/ptr_traits.h",                   "memory"             )
    X("/bits/random.h",                       "random"             )
    X("/bits/random.tcc",                     "random"             )
    X("/bits/range_access.h",                 "iterator"           )
    X("/bits/rc_string_base.h",               "vstring.h"          )
    X("/bits/regex.h",                        "regex"              )
    X("/bits/regex_compiler.h",               "regex"              )
    X("/bits/regex_constants.h",              "regex"              )
    X("/bits/regex_cursor.h",                 "regex"              )
    X("/bits/regex_error.h",                  "regex"              )
    X("/bits/regex_grep_matcher.h",           "regex"              )
    X("/bits/regex_grep_matcher.tcc",         "regex"              )
    X("/bits/regex_nfa.h",                    "regex"              )
    X("/bits/regex_nfa.tcc",                  "regex"              )
    X("/bits/riemann_zeta.tcc",               "cmath"              )
    X("/bits/ropeimpl.h",                     "rope"               )
    X("/bits/shared_ptr.h",                   "memory"             )
    X("/bits/shared_ptr_base.h",              "memory"             )
    X("/bits/slice_array.h",                  "valarray"           )
    X("/bits/special_function_util.h",        "cmath"              )
    X("/bits/sso_string_base.h",              "vstring.h"          )
    X("/bits/sstream.tcc",                    "sstream"            )
    X("/bits/stl_algo.h",                     "algorithm"          )
    X("/bits/stl_algobase.h",                 "algorithm"          )
    X("/bits/stl_bvector.h",                  "vector"             )
    X("/bits/stl_construct.h",                "memory"             )
    X("/bits/stl_deque.h",                    "deque"              )
    X("/bits/stl_function.h",                 "functional"         )
    X("/bits/stl_heap.h",                     "queue"              )
    X("/bits/stl_iterator.h",                 "iterator"           )
    X("/bits/stl_iterator_base_funcs.h",      "iterator"           )
    X("/bits/stl_iterator_base_types.h",      "iterator"           )
    X("/bits/stl_list.h",                     "list"               )
    X("/bits/stl_map.h",                      "map"                )
    X("/bits/stl_multimap.h",                 "map"                )
    X("/bits/stl_multiset.h",                 "set"                )
    X("/bits/stl_numeric.h",                  "numeric"            )
    X("/bits/stl_pair.h",                     "utility"            )
    X("/bits/stl_queue.h",                    "queue"              )
    X("/bits/stl_raw_storage_iter.h",         "memory"             )
    X("/bits/stl_relops.h",                   "utility"            )
    X("/bits/stl_set.h",                      "set"                )
    X("/bits/stl_stack.h",                    "stack"              )
    X("/bits/stl_tempbuf.h",                  "memory"             )
    X("/bits/stl_tree.h",                     "map"                )
    X("/bits/stl_uninitialized.h",            "memory"             )
    X("/bits/stl_vector.h",                   "vector"             )
    X("/bits/stream_iterator.h",              "iterator"           )
    X("/bits/streambuf.tcc",                  "streambuf"          )
    X("/bits/streambuf_iterator.h",           "iterator"           )
    X("/bits/stringfwd.h",                    "string"             )
    X("/bits/strstream",                      "sstream"            )
    X("/bits/time_members.h",                 "locale"             )
    X("/bits/unique_ptr.h",                   "memory"             )
    X("/bits/unordered_map.h",                "unordered_map"      )
    X("/bits/unordered_set.h",                "unordered_set"      )
    X("/bits/valarray_after.h",               "valarray"           )
    X("/bits/valarray_array.h",               "valarray"           )
    X("/bits/valarray_array.tcc",             "valarray"           )
    X("/bits/valarray_before.h",              "valarray"           )
    X("/bits/vector.tcc",                     "vector"             )
    X("/bits/vstring.tcc",                    "vstring.h"          )
    X("/bits/vstring_fwd.h",                  "vstring.h"          )
    X("/bits/vstring_util.h",                 "vstring.h"          )

    X("/bslstl_algorithmworkaround.h",        "bsl_algorithm.h"    )
    X("/bslstl_allocator.h",                  "bsl_memory.h"       )
    X("/bslstl_allocatortraits.h",            "bsl_memory.h"       )
    X("/bslstl_badweakptr.h",                 "bsl_memory.h"       )
    X("/bslstl_bidirectionaliterator.h",      "bsl_iterator.h"     )
    X("/bslstl_bitset.h",                     "bsl_bitset.h"       )
    X("/bslstl_deque.h",                      "bsl_deque.h"        )
    X("/bslstl_equalto.h",                    "bsl_functional.h"   )
    X("/bslstl_forwarditerator.h",            "bsl_iterator.h"     )
    X("/bslstl_hash.h",                       "bsl_functional.h"   )
    X("/bslstl_istringstream.h",              "bsl_sstream.h"      )
    X("/bslstl_iterator.h",                   "bsl_iterator.h"     )
    X("/bslstl_list.h",                       "bsl_list.h"         )
    X("/bslstl_map.h",                        "bsl_map.h"          )
    X("/bslstl_multimap.h",                   "bsl_map.h"          )
    X("/bslstl_multiset.h",                   "bsl_set.h"          )
    X("/bslstl_ostringstream.h",              "bsl_sstream.h"      )
    X("/bslstl_pair.h",                       "bsl_utility.h"      )
    X("/bslstl_randomaccessiterator.h",       "bsl_iterator.h"     )
    X("/bslstl_set.h",                        "bsl_set.h"          )
    X("/bslstl_sharedptr.h",                  "bsl_memory.h"       )
    X("/bslstl_sstream.h",                    "bsl_sstream.h"      )
    X("/bslstl_stack.h",                      "bsl_stack.h"        )
    X("/bslstl_stdexceptutil.h",              "bsl_stdexcept.h"    )
    X("/bslstl_string.h",                     "bsl_string.h"       )
    X("/bslstl_stringbuf.h",                  "bsl_sstream.h"      )
    X("/bslstl_stringstream.h",               "bsl_sstream.h"      )
    X("/bslstl_unorderedmap.h",               "bsl_unordered_map.h")
    X("/bslstl_unorderedmultimap.h",          "bsl_unordered_map.h")
    X("/bslstl_unorderedmultiset.h",          "bsl_unordered_set.h")
    X("/bslstl_unorderedset.h",               "bsl_unordered_set.h")
    X("/bslstl_vector.h",                     "bsl_vector.h"       )
    X("/bslstl_allocator.h",                  "bsl_memory.h"       )
    }

    return s;
}

std::string get_mapped(llvm::StringRef s)
{
    for (size_t rs = s.rfind('/'); rs != s.npos; rs = s.rfind('/', rs)) {
        auto i = mapped_files().find(s.substr(rs));
        if (i != mapped_files().end()) {
            return i->second;
        }
    }
    return "";
}

bool is_mapped(llvm::StringRef s)
{
    return !get_mapped(s).empty();
}

llvm::Regex (&skipped_files())[2]
{

static llvm::Regex s[2] = {
    { "(^|/)bsl_stdhdrs_(epi|pro)logue(_recursive)?[.]h$" },
    { ".+/(bits|stlport)/[^/.]+([.]h)?$"                  },
};

    return s;
}

bool is_skipped(llvm::StringRef name)
{
    for (auto& re : skipped_files()) {
        if (re.match(name)) {
            return true;
        }
    }
    return false;
}

std::set<llvm::StringRef> &reexporting_files()
{

static auto &s = *new std::set<llvm::StringRef>({
    "bael_log.h",
});

    return s;
}

std::map<llvm::StringRef, std::set<llvm::StringRef>> &if_included_map()
{
    static std::map<llvm::StringRef, std::set<llvm::StringRef>> s;
    if (!s.size()) {
        s["bsl_ios.h"].insert("bsl_iostream.h");
        s["bsl_ios.h"].insert("bsl_streambuf.h");
        s["bsl_ios.h"].insert("bsl_strstream.h");
        s["bsl_iosfwd.h"].insert("bsl_ios.h");
        s["bsl_istream.h"].insert("bsl_iostream.h");
        s["bsl_ostream.h"].insert("bsl_iostream.h");
        s["bsl_streambuf.h"].insert("bsl_iostream.h");
        s["ios.h"].insert("bsl_iostream.h");
        s["ios.h"].insert("bsl_streambuf.h");
        s["ios.h"].insert("bsl_strstream.h");
        s["iosfwd"].insert("bsl_ios.h");
        s["istream"].insert("bsl_iostream.h");
        s["math.h"].insert("bsl_cmath.h");
        s["ostream"].insert("bsl_iostream.h");
        s["streambuf"].insert("bsl_iostream.h");
    }

    return s;
}

bool reexports(llvm::StringRef outer, llvm::StringRef inner)
{
    llvm::SmallVector<char, 1000> buf;

    outer = llvm::sys::path::filename(outer);
    inner = llvm::sys::path::filename(inner);

    if (outer == inner) {
        return true;
    }

    if (outer == "bsl_ios.h" &&
        (inner == "bsl_iosfwd.h" ||
         inner == "iosfwd")) {
        return true;
    }

    if (outer == "bsl_cmath.h" && inner == "math.h") {
        return true;
    }

    if (outer == "bsl_iostream.h" &&
        (inner == "bsl_ios.h" ||
         inner == "ios" ||
         inner == "bsl_istream.h" ||
         inner == "istream" ||
         inner == "bsl_ostream.h" ||
         inner == "ostream" ||
         inner == "streambuf" ||
         inner == "bsl_streambuf.h")) {
        return true;
    }

    if (outer == "bsl_streambuf.h" &&
        (inner == "bsl_ios.h" ||
         inner == "ios")) {
        return true;
    }

    if (outer == "bsl_strstream.h" &&
        (inner == "bsl_ios.h" ||
         inner == "ios")) {
        return true;
    }

    if (reexporting_files().count(outer)) {
        return true;
    }

    buf.clear();
    if (outer == ("bsl_" + inner + ".h").toStringRef(buf)) {
        return true;
    }

    buf.clear();
    if (outer == ("bsl_c_" + inner).toStringRef(buf)) {
        return true;
    }

    return false;
}

struct data
    // Data attached to analyzer for this check.
{
    data();

    std::vector<FileID>                                        d_fileid_stack;
    std::string                                                d_guard;
    SourceLocation                                             d_guard_pos;
    std::map<FileID, std::map<std::string, SourceLocation>>    d_once;
    std::map<FileID, std::set<std::string>>                    d_includes;
    std::set<std::string>                                      d_all_includes;
    std::map<FileID, std::string>                              d_guards;
    std::map<std::tuple<FileID, FileID, SourceLocation>, SourceLocation>
                                                               d_fid_map;
    std::map<std::pair<SourceLocation, SourceLocation>, std::string>
                                                               d_file_for_loc;
    std::map<SourceLocation, std::vector<std::pair<FileID, bool>>>
                                                               d_include_stack;
    std::set<std::pair<FileID, const Decl *>>                  d_decls;
    bool                                                       d_ovr;
};

data::data()
: d_ovr(false)
{
}

struct report : public RecursiveASTVisitor<report>, Report<data>
{
    INHERIT_REPORT_CTOR(report, Report, data);

    typedef RecursiveASTVisitor<report> base;

    std::vector<std::pair<FileID, bool>>& include_stack(SourceLocation sl);
        // Get the include stack for the specified 'sl'.

    std::string file_for_location(SourceLocation in, SourceLocation sl);
        // Return the header file appropriate for including the specified 'sl'
        // at the specified 'in', calculated as follows:
        // 1) If 'sl' is (transitively) included through the component header
        //    and the main file isn't the component header and 'in' is in the
        //    component header, return the component header. Otherwise,
        // 2) If 'sl' is (transitively) included below 'in' through a file in
        //    the 'top_level_files' set without any intervening includes of a
        //    file in the 'top_level_files' set, return that file.  Otherwise,
        // 3) Return the file containing 'sl'.

    void clear_guard();
        // Clear the current include guard info.

    void set_guard(llvm::StringRef guard, SourceLocation where);
        // Set the guard info to the specified 'guard' and 'where'.

    void push_include(FileID fid, llvm::StringRef name, SourceLocation sl);
        // Mark that the specified 'name' is included at the specified 'sl'
        // in the specified 'fid', and in files which include it if not
        // dependent on BSL_OVERRIDES_STD.

    void operator()(SourceLocation   where,
                    const Token&     inc,
                    llvm::StringRef  name,
                    bool             angled,
                    CharSourceRange  namerange,
                    const FileEntry *entry,
                    llvm::StringRef  path,
                    llvm::StringRef  relpath,
                    const Module    *imported);
        // Preprocessor callback for included file.

    void map_file(std::string name);
        // Find a possibly different file to include that includes the
        // specified 'name'.

    void operator()(SourceLocation                now,
                    PPCallbacks::FileChangeReason reason,
                    SrcMgr::CharacteristicKind    type,
                    FileID                        prev);
        // Preprocessor callback for file changed.

    bool is_named(Token const& token, llvm::StringRef name);
        // Return 'true' iff the specified 'token' is the specified 'name'.

    void operator()(Token const&          token,
                    MacroDirective const *md,
                    SourceRange           range,
                    MacroArgs const      *);
        // Preprocessor callback for macro expanding.

    void operator()(Token const&          token,
                    MacroDirective const *md);
        // Preprocessor callback for macro (un)defined.

    void operator()(SourceLocation        where,
                    const Token&          token,
                    const MacroDirective *md);
        // Preprocessor callback for 'ifdef'/'ifndef'.

    void operator()(const Token&          token,
                    const MacroDirective *md,
                    SourceRange           range);
        // Preprocessor callback for 'defined(.)'.

    void operator()(SourceRange range);
        // Preprocessor callback for skipped ranges.

    void operator()(SourceLocation where,
                    SourceRange    condition,
                    bool           value);
        // Preprocessor callback for 'if'.

    void operator()(SourceLocation where,
                    SourceRange    condition,
                    bool           value,
                    SourceLocation ifloc);
        // Preprocessor callback for 'elif'.

    void operator()(SourceLocation     where,
                    SourceLocation     ifloc);
        // Preprocessor callback for 'else'.

    bool files_match(llvm::StringRef included_file,
                     llvm::StringRef wanted_file);
        // Return 'true' if the specified 'wanted_file' is a match for the
        // specified 'included_file'.  The files match if they are the same, if
        // the included file is a forwarded version of the wanted file, or if,
        // in BSL_OVERRIDES_STD mode, the included file is the standard form of
        // the wanted file.

    void require_file(std::string     name,
                      SourceLocation  srcloc,
                      llvm::StringRef symbol,
                      SourceLocation  symloc);
        // Indicate that the specified file 'name' is needed at the specified
        // 'srcloc' in order to obtain the specified 'symbol' located at the
        // specified 'symloc'.

    void inc_for_decl(llvm::StringRef  r,
                      SourceLocation   sl,
                      const Decl      *ds);
        // For the specified name 'r' at location 'sl' referenced by the
        // specified declaration 'ds', determine which header file, if any, is
        // needed.

    const NamedDecl *look_through_typedef(const Decl *ds);
        // If the specified 'ds' is a typedef for a record, return the
        // definition for the record if it exists.  Return null otherwise.

    void operator()();
        // Callback for end of main file.

    bool is_guard(llvm::StringRef guard);
    bool is_guard(const Token& token);
        // Return true if the specified 'guard' or 'token' looks like a header
        // guard ("INCLUDED_...").

    bool is_guard_for(llvm::StringRef guard, llvm::StringRef file);
    bool is_guard_for(const Token& token, llvm::StringRef file);
    bool is_guard_for(llvm::StringRef guard, SourceLocation sl);
    bool is_guard_for(const Token& token, SourceLocation sl);
        // Return true if the specified 'guard' or 'token' is a header guard
        // for the specified 'file' or 'sl'.

    std::string map_if_included(FileID fid, std::string name);
        // Return a mapped file for the specified 'name' if that file is
        // included within the specified 'fid'.

    std::string name_for(const NamedDecl *decl);
        // Return a diagnostic name for the specified 'decl'.

    bool shouldVisitTemplateInstantiations () const;
        // Return true;

    bool VisitCXXConstructExpr(CXXConstructExpr *expr);
    bool VisitDeclRefExpr(DeclRefExpr *expr);
    bool VisitNamedDecl(NamedDecl *decl);
    bool VisitNamespaceAliasDecl(NamespaceAliasDecl *decl);
    bool VisitNamespaceDecl(NamespaceDecl *decl);
    bool VisitQualifiedTypeLoc(QualifiedTypeLoc tl);
    bool VisitTagDecl(TagDecl *decl);
    bool VisitTemplateDecl(TemplateDecl *decl);
    bool VisitTypeLoc(TypeLoc tl);
    bool VisitTypedefNameDecl(TypedefNameDecl *decl);
    bool VisitTypedefTypeLoc(TypedefTypeLoc tl);
    bool VisitUsingDecl(UsingDecl *decl);
    bool VisitUsingDirectiveDecl(UsingDirectiveDecl *decl);
    bool VisitValueDecl(ValueDecl *decl);
        // Return true after processing the specified 'tl' and 'expr'.
};

std::vector<std::pair<FileID, bool>>& report::include_stack(SourceLocation sl)
{
    auto& v = d_data.d_include_stack[sl];
    if (!v.size()) {
        while (sl.isValid()) {
            FileName fn(m.getFilename(sl));
            v.emplace_back(m.getFileID(sl), is_top_level(fn.name()));
            sl = m.getIncludeLoc(v.back().first);
        }
    }
    return v;
}

std::string report::map_if_included(FileID fid, std::string name)
{
    name = llvm::sys::path::filename(name);
    auto i = if_included_map().find(name);
    if (i != if_included_map().end()) {
        for (const auto& m : i->second) {
            if (d_data.d_all_includes.count(m)) {
                return map_if_included(fid, m);
            }
        }
    }
    std::string n = "bsl_" + name + ".h";
    if (d_data.d_all_includes.count(n)) {
        return n;
    }
    n = "bsl_c" + name;
    if (d_data.d_all_includes.count(n)) {
        return n;
    }
    return name;
}

std::string report::file_for_location(SourceLocation sl, SourceLocation in)
{
    auto ip = std::make_pair(in, sl);
    auto i = d_data.d_file_for_loc.find(ip);
    if (i != d_data.d_file_for_loc.end()) {
        return i->second;
    }

    FileID in_id = m.getFileID(in);
    FileID fid = m.getFileID(sl);
    auto& v = include_stack(sl);
    FileID top = fid;
    bool found = false;
    bool just_found = false;
    std::string result = m.getFilename(sl);
    for (auto& p : v) {
        SourceLocation fl = m.getLocForStartOfFile(p.first);
        SourceLocation tl = m.getLocForStartOfFile(top);
        llvm::StringRef f = m.getFilename(fl);
        llvm::StringRef t = m.getFilename(tl);
        FileName ff(f);
        if (p.first == in_id) {
            result = t;
            break;
        }
        if (is_skipped(f) && !is_mapped(f)) {
            continue;
        }
        if (!found) {
            if (p.second || is_mapped(f)) {
                found = true;
                just_found = true;
                top = p.first;
            }
        }
        else {
            if (reexports(f, t) ||
                (just_found && d_analyser.is_component(ff.name()))) {
                top = p.first;
            }
            just_found = false;
        }
    }
    if (!d_analyser.is_component(result)) {
        if (is_mapped(result)) {
            result = get_mapped(result);
        }
        result = map_if_included(fid, result);
        if (is_skipped(result)) {
            result = "";
        }
    }
    return d_data.d_file_for_loc[ip] = result;
}

void report::clear_guard()
{
    d_data.d_guard = "";
    d_data.d_guard_pos = SourceLocation();
}

void report::set_guard(llvm::StringRef guard, SourceLocation where)
{
    d_data.d_guard = guard;
    d_data.d_guard_pos = d_analyser.get_line_range(where).getBegin();
}

void report::push_include(FileID fid, llvm::StringRef name, SourceLocation sl)
{
    bool in_header = d_analyser.is_component_header(m.getFilename(sl));
    for (FileID f : d_data.d_fileid_stack) {
        if (f == fid) {
            d_data.d_includes[f].insert(name);
            d_data.d_all_includes.insert(name);
        }
        else if (in_header && f == m.getMainFileID()) {
            SourceLocation sfl = sl;
            auto t = std::make_tuple(fid, f, sl);
            auto i = d_data.d_fid_map.find(t);
            if (i != d_data.d_fid_map.end()) {
                sfl = i->second;
            }
            else {
                FileID flid;
                while (sfl.isValid()) {
                    flid = m.getFileID(sfl);
                    if (flid == f) {
                        break;
                    }
                    sfl = m.getIncludeLoc(flid);
                }
                if (sfl.isValid()) {
                    unsigned offset = m.getFileOffset(sfl);
                    unsigned line = m.getLineNumber(flid, offset);
                    if (line > 1) {
                        SourceLocation prev =
                            m.translateLineCol(flid, line - 1, 0);
                        llvm::StringRef p = d_analyser.get_source_line(prev);
                        static llvm::Regex guard("^ *# *ifn?def  *INCLUDED_");
                        if (guard.match(p)) {
                            sfl = prev;
                        }
                    }
                }
                d_data.d_fid_map[t] = sfl;
            }
            d_data.d_includes[f].insert(name);
            d_data.d_all_includes.insert(name);
        }
    }
#if 0
    llvm::StringRef file = llvm::sys::path::filename(name);
    if (file != name) {
        push_include(fid, file, sl);
    }
#endif
}

// InclusionDirective
void report::operator()(SourceLocation   where,
                        const Token&     inc,
                        llvm::StringRef  name,
                        bool             angled,
                        CharSourceRange  namerange,
                        const FileEntry *entry,
                        llvm::StringRef  path,
                        llvm::StringRef  relpath,
                        const Module    *imported)
{
    FileID fid = m.getFileID(where);
    if (d_data.d_guard_pos.isValid() &&
        fid != m.getFileID(d_data.d_guard_pos)) {
        clear_guard();
    }

    push_include(
        fid, name, d_data.d_guard_pos.isValid() ? d_data.d_guard_pos : where);

    clear_guard();
}

// FileChanged
void report::operator()(SourceLocation                now,
                        PPCallbacks::FileChangeReason reason,
                        SrcMgr::CharacteristicKind    type,
                        FileID                        prev)
{
    if (reason == PPCallbacks::EnterFile) {
        d_data.d_fileid_stack.emplace_back(m.getFileID(now));
    } else if (reason == PPCallbacks::ExitFile) {
        if (d_data.d_fileid_stack.size() > 0) {
            d_data.d_fileid_stack.pop_back();
        }
    }
}

bool report::is_named(Token const& token, llvm::StringRef name)
{
    return token.isAnyIdentifier() &&
           token.getIdentifierInfo()->getName() == name;
}

// MacroExpands
void report::operator()(Token const&          token,
                        MacroDirective const *md,
                        SourceRange           range,
                        MacroArgs const      *)
{
    llvm::StringRef macro = token.getIdentifierInfo()->getName();
    const MacroInfo *mi = md->getMacroInfo();
    Location loc(m, mi->getDefinitionLoc());
    if (loc && !range.getBegin().isMacroID() && macro != "std") {
        require_file(
            file_for_location(mi->getDefinitionLoc(), range.getBegin()),
            range.getBegin(),
            /*std::string("MacroExpands ") +*/ macro.str(),
            mi->getDefinitionLoc());
    }
}

// MacroDefined/MacroUndefined
void report::operator()(Token const&          token,
                        MacroDirective const *md)
{
    llvm::StringRef macro = token.getIdentifierInfo()->getName();
    if (macro == "BSL_OVERRIDES_STD") {
        d.d_ovr = d_type == PPObserver::e_MacroDefined;
    }
}

bool report::is_guard(llvm::StringRef guard)
{
    return guard.startswith("INCLUDED_");
}

bool report::is_guard(const Token& token)
{
    return token.isAnyIdentifier() &&
           is_guard(token.getIdentifierInfo()->getName());
}

bool report::is_guard_for(llvm::StringRef guard, llvm::StringRef file)
{
    if (!is_guard(guard)) {
        return false;                                                 // RETURN
    }

    FileName fn(file);
    std::string s = "INCLUDED_" + fn.component().upper();
    for (char& c : s) {
        if (!std::isalnum(c)) {
            c = '_';
        }
    }
    return s == guard || s + "_" + fn.extension().substr(1).upper() == guard;
}

bool report::is_guard_for(const Token& token, llvm::StringRef file)
{
    return token.isAnyIdentifier() &&
           is_guard_for(token.getIdentifierInfo()->getName(), file);
}

bool report::is_guard_for(llvm::StringRef guard, SourceLocation sl)
{
    return is_guard_for(guard, m.getFilename(sl));
}

bool report::is_guard_for(const Token& token, SourceLocation sl)
{
    return is_guard_for(token, m.getFilename(sl));
}

// Ifdef
// Ifndef
void report::operator()(SourceLocation        where,
                        const Token&          token,
                        const MacroDirective *)
{
    llvm::StringRef tn = token.getIdentifierInfo()->getName();

    clear_guard();

    if (is_guard(token)) {
        set_guard(tn, where);
    }
}

// Defined
void report::operator()(const Token&          token,
                        const MacroDirective *,
                        SourceRange           range)
{
    clear_guard();

    if (is_guard(token)) {
        set_guard(token.getIdentifierInfo()->getName(), range.getBegin());
    }
}

// SourceRangeSkipped
void report::operator()(SourceRange range)
{
    Location loc(m, range.getBegin());
    if (d_data.d_guard.size() > 0) {
        llvm::Regex r("ifndef +" + d_data.d_guard + "[[:space:]]+"
                      "# *include +<([^>]+)>");
        llvm::StringRef source = d_analyser.get_source(range);
        llvm::SmallVector<llvm::StringRef, 7> matches;
        if (r.match(source, &matches)) {
            push_include(m.getFileID(range.getBegin()),
                         matches[1],
                         d_data.d_guard_pos.isValid() ? d_data.d_guard_pos :
                                                        range.getBegin());
        }
        clear_guard();
    }
}

// If
void report::operator()(SourceLocation where,
                        SourceRange    condition,
                        bool           value)
{
    clear_guard();
}

// Elif
void report::operator()(SourceLocation where,
                        SourceRange    condition,
                        bool           value,
                        SourceLocation ifloc)
{
    clear_guard();
}

// Else/Endif
void report::operator()(SourceLocation where, SourceLocation ifloc)
{
    if (d_type == PPObserver::e_Else) {
        clear_guard();
    }
}

const NamedDecl *report::look_through_typedef(const Decl *ds)
{
#if 0
    const TypedefDecl *td;
    const CXXRecordDecl *rd;
    if ((td = llvm::dyn_cast<TypedefDecl>(ds)) &&
        (rd = td->getUnderlyingType().getTypePtr()->getAsCXXRecordDecl()) &&
        rd->hasDefinition()) {
        return rd->getDefinition();
    }
#endif
    return 0;
}

bool report::files_match(llvm::StringRef included_file,
                         llvm::StringRef wanted_file)
{
    return included_file == wanted_file
        || included_file == ("bslfwd_" + wanted_file).str()
        || (d.d_ovr && ("bsl_" + included_file + ".h").str() == wanted_file);
}

void report::require_file(std::string     name,
                          SourceLocation  srcloc,
                          llvm::StringRef symbol,
                          SourceLocation  symloc)
{
    if (name.empty()) {
        return;
    }

    if (a.is_standard_namespace(symbol)) {
        return;
    }

    srcloc = m.getExpansionLoc(srcloc);
    FileID fid = m.getFileID(srcloc);
    FileName ff(m.getFilename(srcloc));

    FileName fn(name);
    name = fn.name();

    if (name == ff.name() ||
        is_top_level(ff.name()) ||
        is_skipped(ff.name())) {
        return;
    }

    for (const auto& p : mapped_files()) {
        if (p.first == ff.name() || p.second == ff.name()) {
            return;
        }
    }

    for (const auto& s : d_data.d_includes[fid]) {
        if (files_match(llvm::sys::path::filename(s), name)) {
            return;
        }
    }

    if (!d_data.d_once[fid].count(name) /*||
        m.isBeforeInTranslationUnit(srcloc, d_data.d_once[fid][name])*/) {
        d_data.d_once[fid][name] = srcloc;
        d_analyser.report(srcloc, check_name, "AQK01",
                          "Need #include <%0> for '%1'")
            << name
            << symbol;
        if (d_analyser.is_component_header(ff.name())) {
            d_data.d_once[m.getMainFileID()][name] = srcloc;
        }
    }
}

void report::inc_for_decl(llvm::StringRef r, SourceLocation sl, const Decl *ds)
{
    sl = m.getExpansionLoc(sl);
    if (!d_analyser.is_component(sl)) {
        return;
    }
    if (!d_data.d_decls.insert({m.getFileID(sl), ds->getCanonicalDecl()})
             .second) {
        return;
    }

#if 0
    if (const UsingDecl *ud = llvm::dyn_cast<UsingDecl>(ds)) {
        auto sb = ud->shadow_begin();
        auto se = ud->shadow_end();
        for (; sb != se; ++sb) {
            const UsingShadowDecl *usd = *sb;
            for (auto u = usd; u; u = u->getPreviousDecl()) {
                inc_for_decl(r, sl, u);
            }
        }
        return;
    }
#endif

    for (const Decl *d = ds; d; d = look_through_typedef(d)) {
        bool skip = false;
        Decl *prefer = 0;
        for (const Decl *p = d; !skip && p; p = p->getPreviousDecl()) {
#if 1
            Location loc(m, p->getLocation());
            FileName fn(loc.file());
            Decl::redecl_iterator rb = p->redecls_begin();
            Decl::redecl_iterator re = p->redecls_end();
            for (; !skip && rb != re; ++rb) {
                SourceLocation rl = rb->getLocation();
                if (rl.isValid() /*&& !m.isBeforeInTranslationUnit(sl, rl)*/) {
                    llvm::StringRef file = file_for_location(rl, sl);
                    skip = d_analyser.is_component(file) ||
                           d_data.d_includes[m.getMainFileID()].count(file);
                }
                if (auto decl = llvm::dyn_cast<VarDecl>(*rb)) {
                    if (decl->isThisDeclarationADefinition()) {
                        prefer = *rb;
                    }
                }
                if (auto decl = llvm::dyn_cast<FunctionDecl>(*rb)) {
                    if (decl->isThisDeclarationADefinition()) {
                        prefer = *rb;
                    }
                }
                if (auto decl = llvm::dyn_cast<TagDecl>(*rb)) {
                    if (decl->getRBraceLoc().isValid()) {
                        prefer = *rb;
                    }
                }
                if (llvm::dyn_cast<UsingDecl>(*rb)) {
                    prefer = *rb;
                }
            }
#endif
        }
        for (const Decl *p = d; !skip && p; p = p->getPreviousDecl()) {
#if 1
            Location loc(m, p->getLocation());
            FileName fn(loc.file());
            Decl::redecl_iterator rb = p->redecls_begin();
            Decl::redecl_iterator re = p->redecls_end();
            for (; !skip && rb != re; ++rb) {
                SourceLocation rl = rb->getLocation();
                if (rl.isValid() /*&& !m.isBeforeInTranslationUnit(sl, rl)*/) {
                    Location loc(m, rl);
                    if (!skip && loc && (!prefer || prefer == *rb)) {
                        require_file(file_for_location(rl, sl), sl, r, rl);
                        skip = true;
                    }
                }
            }
#endif
        }
    }
}
//#define inc_for_decl(r,s,d) inc_for_decl(std::string(__FUNCTION__)+" "+r,s,d)

std::string report::name_for(const NamedDecl *decl)
{
    std::string result;
    llvm::raw_string_ostream s(result);
    PrintingPolicy pp(d_analyser.context()->getLangOpts());
    pp.Indentation = 4;
    pp.SuppressSpecifiers = false;
    pp.SuppressTagKeyword = false;
    pp.SuppressTag = false;
    pp.SuppressScope = false;
    pp.SuppressUnwrittenScope = false;
    pp.SuppressInitializers = false;
    pp.ConstantArraySizeAsWritten = true;
    pp.AnonymousTagLocations = true;
    pp.Bool = true;
    pp.TerseOutput = false;
    pp.PolishForDeclaration = true;
    pp.IncludeNewlines = false;
    decl->getNameForDiagnostic(s, pp, true);
    return s.str();
}

bool report::shouldVisitTemplateInstantiations() const
{
    return !true;
}

bool report::VisitNamespaceAliasDecl(NamespaceAliasDecl *decl)
{
#if 1
    SourceLocation sl = decl->getLocation();
    if (sl.isValid() &&
        !sl.isMacroID() &&
        decl->isExternallyVisible()) {
        std::string name = name_for(decl);
        inc_for_decl(name, sl, decl);
    }
#endif
    return true;
}

bool report::VisitNamespaceDecl(NamespaceDecl *decl)
{
#if 1
    SourceLocation sl = decl->getLocation();
    if (sl.isValid() &&
        !sl.isMacroID() &&
        decl->isExternallyVisible() &&
        decl->getName() != "std") {
        std::string name = name_for(decl);
        inc_for_decl(name, sl, decl);
    }
#endif
    return true;
}

bool report::VisitTemplateDecl(TemplateDecl *decl)
{
#if 1
    SourceLocation sl = decl->getLocation();
    if (sl.isValid() &&
        !sl.isMacroID() &&
        decl->isExternallyVisible()) {
        std::string name = name_for(decl);
        inc_for_decl(name, sl, decl);
    }
#endif
    return true;
}

bool report::VisitTagDecl(TagDecl *decl)
{
#if 1
    SourceLocation sl = decl->getLocation();
    if (sl.isValid() &&
        !sl.isMacroID() &&
        decl->isExternallyVisible()) {
        std::string name = name_for(decl);
        inc_for_decl(name, sl, decl);
    }
#endif
    return true;
}

bool report::VisitTypedefNameDecl(TypedefNameDecl *decl)
{
#if 1
    SourceLocation sl = decl->getLocation();
    if (sl.isValid() &&
        !sl.isMacroID() &&
        decl->isExternallyVisible()) {
        std::string name = name_for(decl);
        inc_for_decl(name, sl, decl);
    }
#endif
    return true;
}

bool report::VisitUsingDecl(UsingDecl *decl)
{
#if 1
    SourceLocation sl = decl->getLocation();
    if (sl.isValid() &&
        !sl.isMacroID() &&
        decl->isExternallyVisible()) {
        std::string name = name_for(decl);
        inc_for_decl(name, sl, decl);
    }
#endif
    return true;
}

bool report::VisitValueDecl(ValueDecl *decl)
{
#if 1
    SourceLocation sl = decl->getLocation();
    if (sl.isValid() &&
        !sl.isMacroID() &&
        decl->isExternallyVisible()) {
        std::string name = name_for(decl);
        inc_for_decl(name, sl, decl);
    }
#endif
    return true;
}

bool report::VisitNamedDecl(NamedDecl *decl)
{
#if 1
    SourceLocation sl = decl->getLocation();
    if (sl.isValid() &&
        !sl.isMacroID() &&
        !llvm::dyn_cast<TemplateTypeParmDecl>(decl) &&
        !llvm::dyn_cast<TemplateTemplateParmDecl>(decl) &&
        decl->isExternallyVisible()) {
        std::string name = name_for(decl);
        inc_for_decl(name, sl, decl);
    }
#endif
    return base::VisitNamedDecl(decl);
}

bool report::VisitUsingDirectiveDecl(UsingDirectiveDecl *decl)
{
#if 1
    NamespaceDecl *nd = decl->getNominatedNamespace();
    SourceLocation sl = decl->getLocation();
    if (sl.isValid() &&
        !sl.isMacroID() &&
        nd->isExternallyVisible()) {
        inc_for_decl(name_for(nd), sl, nd);
    }
#endif
    return base::VisitUsingDirectiveDecl(decl);
}

bool report::VisitDeclRefExpr(DeclRefExpr *expr)
{
#if 1
    SourceLocation sl = expr->getExprLoc();
    if (sl.isValid() &&
        !sl.isMacroID()) {
        const NamedDecl *ds = expr->getFoundDecl();
        const DeclContext *dc = ds->getDeclContext();
        std::string name = expr->getNameInfo().getName().getAsString();
        while (dc->isRecord()) {
            ds = llvm::dyn_cast<NamedDecl>(dc);
            name = name_for(ds);
            dc = dc->getParent();
        }
        if (dc->isFileContext() ||
            dc->isExternCContext() ||
            dc->isExternCXXContext()) {
            inc_for_decl(name, sl, ds);
        }
    }
#endif
    return base::VisitDeclRefExpr(expr);
}

bool report::VisitCXXConstructExpr(CXXConstructExpr *expr)
{
#if 1
    SourceLocation sl = expr->getExprLoc();
    const NamedDecl *ds = 0;
    std::string name;
    if (const VarDecl *vd = d_analyser.get_parent<VarDecl>(expr)) {
        TypeLoc tl = vd->getTypeSourceInfo()->getTypeLoc().getUnqualifiedLoc();
        for (;;) {
            ElaboratedTypeLoc etl = tl.getAs<ElaboratedTypeLoc>();
            if (etl.isNull()) {
                break;
            }
            tl = etl.getNamedTypeLoc();
        }
        TypedefTypeLoc ttl = tl.getAs<TypedefTypeLoc>();
        if (!ttl.isNull()) {
            ds = ttl.getTypedefNameDecl();
            name = ds->getNameAsString();
            sl = ttl.getBeginLoc();
        }
    }

    if (sl.isValid() &&
        !sl.isMacroID()) {
        if (!ds) {
            ds = expr->getConstructor()->getParent();
            name = name_for(ds);
        }
        const DeclContext *dc = ds->getDeclContext();
        while (dc->isRecord()) {
            name = name_for(llvm::dyn_cast<NamedDecl>(dc));
            dc = dc->getParent();
        }
        if (dc->isFileContext() ||
            dc->isExternCContext() ||
            dc->isExternCXXContext()) {
            inc_for_decl(name, sl, ds);
        }
    }
#endif
    return base::VisitCXXConstructExpr(expr);
}

bool report::VisitQualifiedTypeLoc(QualifiedTypeLoc tl)
{
    const Type *type = tl.getTypePtr();
    SourceLocation sl = tl.getBeginLoc();
    if (!m.isWrittenInSameFile(tl.getBeginLoc(), tl.getEndLoc())) {
        sl = m.getExpansionLoc(sl);
    }
    PrintingPolicy pp(d_analyser.context()->getLangOpts());
    pp.SuppressTagKeyword = true;
    pp.SuppressInitializers = true;
    pp.TerseOutput = true;
    std::string r = QualType(type, 0).getAsString(pp);
    NamedDecl *ds = d_analyser.lookup_name(r);
    if (!ds) {
        tl.getTypePtr()->isIncompleteType(&ds);
    }
    if (ds && sl.isValid() && !sl.isMacroID()) {
        r = name_for(ds);
        inc_for_decl(r, sl, ds);
    }
    return base::VisitQualifiedTypeLoc(tl);
}

bool report::VisitTypedefTypeLoc(TypedefTypeLoc tl)
{
#if 1
    TypedefNameDecl *ds = tl.getTypedefNameDecl();
    SourceLocation sl = tl.getBeginLoc();
    if (!m.isWrittenInSameFile(tl.getBeginLoc(), tl.getEndLoc())) {
        sl = m.getExpansionLoc(sl);
    }
    if (ds && sl.isValid() && !sl.isMacroID()) {
        std::string r = name_for(ds);
        inc_for_decl(r, sl, ds);
    }
#endif
    return base::VisitTypedefTypeLoc(tl);
}

bool report::VisitTypeLoc(TypeLoc tl)
{
#if 0
    const Type *type = tl.getTypePtr();
    if (type->getAs<TypedefType>() || !type->isBuiltinType()) {
        SourceLocation sl = tl.getBeginLoc();
        if (!m.isWrittenInSameFile(tl.getBeginLoc(), tl.getEndLoc())) {
            sl = m.getExpansionLoc(sl);
        }
        PrintingPolicy pp(d_analyser.context()->getLangOpts());
        pp.SuppressTagKeyword = true;
        pp.SuppressInitializers = true;
        pp.TerseOutput = true;
        std::string r = QualType(type, 0).getAsString(pp);
        NamedDecl *ds = d_analyser.lookup_name(r);
        if (!ds) {
            if (const TypedefType *tt = type->getAs<TypedefType>()) {
                ds = tt->getDecl();
            }
            else {
                tl.getTypePtr()->isIncompleteType(&ds);
            }
        }
        if (ds && sl.isValid() && !sl.isMacroID()) {
            inc_for_decl(r, sl, ds);
        }
    }
#endif
    return base::VisitTypeLoc(tl);
}

// TranslationUnitDone
void report::operator()()
{
    TraverseDecl(d_analyser.context()->getTranslationUnitDecl());
}

void subscribe(Analyser& analyser, Visitor&, PPObserver& observer)
    // Hook up the callback functions.
{
    observer.onPPInclusionDirective += report(analyser,
                                                observer.e_InclusionDirective);
    observer.onPPFileChanged        += report(analyser,
                                                       observer.e_FileChanged);
    observer.onPPMacroExpands       += report(analyser,
                                                      observer.e_MacroExpands);
    observer.onPPMacroDefined       += report(analyser,
                                                      observer.e_MacroDefined);
    observer.onPPMacroUndefined     += report(analyser,
                                                    observer.e_MacroUndefined);
    observer.onPPIfdef              += report(analyser, observer.e_Ifdef);
    observer.onPPIfndef             += report(analyser, observer.e_Ifndef);
    observer.onPPDefined            += report(analyser, observer.e_Defined);
    observer.onPPSourceRangeSkipped += report(analyser,
                                                observer.e_SourceRangeSkipped);
    observer.onPPIf                 += report(analyser,  observer.e_If);
    observer.onPPElif               += report(analyser, observer.e_Elif);
    observer.onPPElse               += report(analyser, observer.e_Else);
    observer.onPPEndif              += report(analyser, observer.e_Endif);
    analyser.onTranslationUnitDone  += report(analyser);
}

}  // close anonymous namespace

// ----------------------------------------------------------------------------

static RegisterCheck c1(check_name, &subscribe);

// ----------------------------------------------------------------------------
// Copyright (C) 2014 Bloomberg Finance L.P.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
// ----------------------------- END-OF-FILE ----------------------------------
