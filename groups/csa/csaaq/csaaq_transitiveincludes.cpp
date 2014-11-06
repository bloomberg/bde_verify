// csaaq_transitiveincludes.cpp                                       -*-C++-*-

#include <clang/AST/Decl.h>
#include <clang/AST/DeclBase.h>
#include <clang/AST/DeclCXX.h>
#include <clang/AST/PrettyPrinter.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Type.h>
#include <clang/Basic/IdentifierTable.h>
#include <clang/Basic/SourceLocation.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Lex/MacroInfo.h>
#include <clang/Lex/PPCallbacks.h>
#include <clang/Lex/Token.h>
#include <clang/Tooling/Refactoring.h>
#include <csabase_analyser.h>
#include <csabase_config.h>
#include <csabase_debug.h>
#include <csabase_diagnostic_builder.h>
#include <csabase_filenames.h>
#include <csabase_location.h>
#include <csabase_ppobserver.h>
#include <csabase_registercheck.h>
#include <csabase_util.h>
#include <csabase_visitor.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/ADT/Twine.h>
#include <llvm/Support/Casting.h>
#include <llvm/Support/Regex.h>
#include <llvm/Support/raw_ostream.h>
#include <stddef.h>
#include <cctype>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <utils/event.hpp>
#include <utils/function.hpp>
#include <vector>
#include <tuple>
namespace clang { class FileEntry; }
namespace clang { class MacroArgs; }
namespace clang { class Module; }
namespace csabase { class Visitor; }

using namespace csabase;
using namespace clang::tooling;
using namespace clang;

// ----------------------------------------------------------------------------

static std::string const check_name("transitive-includes");

// ----------------------------------------------------------------------------

namespace
{

std::set<llvm::StringRef> top_level_files {
#undef  X
#define X(n) #n, "bsl_" #n ".h", "stl_" #n ".h"
X(algorithm),     X(array),         X(atomic),           X(bitset),             
X(chrono),        X(codecvt),       X(complex),          X(condition_variable), 
X(deque),         X(exception),     X(forward_list),     X(fstream),            
X(functional),    X(future),        X(initializer_list), X(iomanip),            
X(ios),           X(iosfwd),        X(iostream),         X(istream),            
X(iterator),      X(limits),        X(list),             X(locale),             
X(map),           X(memory),        X(mutex),            X(new),                
X(numeric),       X(ostream),       X(queue),            X(random),             
X(ratio),         X(regex),         X(scoped_allocator), X(set),                
X(sstream),       X(stack),         X(stdexcept),        X(streambuf),          
X(string),        X(strstream),     X(system_error),     X(thread),             
X(tuple),         X(type_traits),   X(typeindex),        X(typeinfo),           
X(unordered_map), X(unordered_set), X(utility),          X(valarray),           
X(vector),        
#undef  X

#undef  X
#define X(n) "c" #n, #n ".h", "bsl_c" #n ".h", "bsl_c_" #n ".h"
X(assert),   X(complex), X(ctype),    X(errno),  X(fenv),    X(float),
X(inttypes), X(iso646),  X(iso646),   X(limits), X(locale),  X(math),
X(setjmp),   X(signal),  X(stdalign), X(stdarg), X(stdbool), X(stddef),
X(stdint),   X(stdio),   X(stdlib),   X(string), X(tgmath),  X(time),
X(uchar),    X(wchar),   X(wctype),
#undef  X

"vstring.h",
};

std::vector<llvm::StringRef> top_level_prefixes {
    "bdlb_",   "bdldfp_", "bdlma_",  "bdls_",  "bdlscm_", "bdlt_", 
    "bslalg_", "bslfwd_", "bslim_",  "bslma_", "bslmf_",  "bsls_", 
    "bslscm_", "bsltf_",  "bslx_",   
};

bool is_top_level(llvm::StringRef name)
{
    if (top_level_files.count(name)) {
        return true;
    }
    for (auto s : top_level_prefixes) {
        if (name.startswith(s)) {
            return true;
        }
    }
    return false;
}

std::map<llvm::StringRef, llvm::StringRef> mapped_files {
    { "algorithmfwd.h",                 "algorithm"           }, 
    { "alloc_traits.h",                 "memory"              }, 
    { "allocator.h",                    "memory"              }, 
    { "atomic_base.h",                  "atomic"              }, 
    { "atomic_lockfree_defines.h",      "atomic"              }, 
    { "auto_ptr.h",                     "memory"              }, 
    { "backward_warning.h",             "iosfwd"              }, 
    { "basic_file.h",                   "ios"                 }, 
    { "basic_ios.h",                    "ios"                 }, 
    { "basic_ios.tcc",                  "ios"                 }, 
    { "basic_string.h",                 "string"              }, 
    { "basic_string.tcc",               "string"              }, 
    { "bessel_function.tcc",            "cmath"               }, 
    { "beta_function.tcc",              "cmath"               }, 
    { "binders.h",                      "functional"          }, 
    { "boost_concept_check.h",          "iterator"            }, 
    { "c++0x_warning.h",                "iosfwd"              }, 
    { "c++allocator.h",                 "memory"              }, 
    { "c++config.h",                    "iosfwd"              }, 
    { "c++io.h",                        "ios"                 }, 
    { "c++locale.h",                    "locale"              }, 
    { "cast.h",                         "pointer.h"           }, 
    { "char_traits.h",                  "string"              }, 
    { "codecvt.h",                      "locale"              }, 
    { "concept_check.h",                "iterator"            }, 
    { "cpp_type_traits.h",              "type_traits"         }, 
    { "cpu_defines.h",                  "iosfwd"              }, 
    { "ctype_base.h",                   "locale"              }, 
    { "ctype_inline.h",                 "locale"              }, 
    { "cxxabi_forced.h",                "cxxabi.h"            }, 
    { "cxxabi_tweaks.h",                "cxxabi.h"            }, 
    { "decimal.h",                      "decimal"             }, 
    { "deque.tcc",                      "deque"               }, 
    { "ell_integral.tcc",               "cmath"               }, 
    { "error_constants.h",              "system_error"        }, 
    { "exception_defines.h",            "exception"           }, 
    { "exception_ptr.h",                "exception"           }, 
    { "exp_integral.tcc",               "cmath"               }, 
    { "forward_list.h",                 "forward_list"        }, 
    { "forward_list.tcc",               "forward_list"        }, 
    { "fstream.tcc",                    "fstream"             }, 
    { "functexcept.h",                  "exception"           }, 
    { "functional_hash.h",              "functional"          }, 
    { "gamma.tcc",                      "cmath"               }, 
    { "gslice.h",                       "valarray"            }, 
    { "gslice_array.h",                 "valarray"            }, 
    { "hash_bytes.h",                   "functional"          }, 
    { "hashtable.h",                    "unordered_map"       }, 
    { "hashtable_policy.h",             "unordered_map"       }, 
    { "hypergeometric.tcc",             "cmath"               }, 
    { "indirect_array.h",               "valarray"            }, 
    { "ios_base.h",                     "ios"                 }, 
    { "istream.tcc",                    "istream"             }, 
    { "legendre_function.tcc",          "cmath"               }, 
    { "list.tcc",                       "list"                }, 
    { "locale_classes.h",               "locale"              }, 
    { "locale_classes.tcc",             "locale"              }, 
    { "locale_facets.h",                "locale"              }, 
    { "locale_facets.tcc",              "locale"              }, 
    { "locale_facets_nonio.h",          "locale"              }, 
    { "locale_facets_nonio.tcc",        "locale"              }, 
    { "localefwd.h",                    "locale"              }, 
    { "mask_array.h",                   "valarray"            }, 
    { "memoryfwd.h",                    "memory"              }, 
    { "messages_members.h",             "locale"              }, 
    { "modified_bessel_func.tcc",       "cmath"               }, 
    { "move.h",                         "utility"             }, 
    { "nested_exception.h",             "exception"           }, 
    { "opt_random.h",                   "random"              }, 
    { "os_defines.h",                   "iosfwd"              }, 
    { "ostream.tcc",                    "ostream"             }, 
    { "ostream_insert.h",               "ostream"             }, 
    { "poly_hermite.tcc",               "cmath"               }, 
    { "poly_laguerre.tcc",              "cmath"               }, 
    { "postypes.h",                     "iosfwd"              }, 
    { "ptr_traits.h",                   "memory"              }, 
    { "random.h",                       "random"              }, 
    { "random.tcc",                     "random"              }, 
    { "range_access.h",                 "iterator"            }, 
    { "rc_string_base.h",               "vstring.h"           }, 
    { "regex.h",                        "regex"               }, 
    { "regex_compiler.h",               "regex"               }, 
    { "regex_constants.h",              "regex"               }, 
    { "regex_cursor.h",                 "regex"               }, 
    { "regex_error.h",                  "regex"               }, 
    { "regex_grep_matcher.h",           "regex"               }, 
    { "regex_grep_matcher.tcc",         "regex"               }, 
    { "regex_nfa.h",                    "regex"               }, 
    { "regex_nfa.tcc",                  "regex"               }, 
    { "riemann_zeta.tcc",               "cmath"               }, 
    { "ropeimpl.h",                     "rope"                }, 
    { "shared_ptr.h",                   "memory"              }, 
    { "shared_ptr_base.h",              "memory"              }, 
    { "slice_array.h",                  "valarray"            }, 
    { "special_function_util.h",        "cmath"               }, 
    { "sso_string_base.h",              "vstring.h"           }, 
    { "sstream.tcc",                    "sstream"             }, 
    { "stl_algo.h",                     "algorithm"           }, 
    { "stl_algobase.h",                 "algorithm"           }, 
    { "stl_bvector.h",                  "vector"              }, 
    { "stl_construct.h",                "memory"              }, 
    { "stl_deque.h",                    "deque"               }, 
    { "stl_function.h",                 "functional"          }, 
    { "stl_heap.h",                     "queue"               }, 
    { "stl_iterator.h",                 "iterator"            }, 
    { "stl_iterator_base_funcs.h",      "iterator"            }, 
    { "stl_iterator_base_types.h",      "iterator"            }, 
    { "stl_list.h",                     "list"                }, 
    { "stl_map.h",                      "map"                 }, 
    { "stl_multimap.h",                 "map"                 }, 
    { "stl_multiset.h",                 "set"                 }, 
    { "stl_numeric.h",                  "numeric"             }, 
    { "stl_pair.h",                     "utility"             }, 
    { "stl_queue.h",                    "queue"               }, 
    { "stl_raw_storage_iter.h",         "memory"              }, 
    { "stl_relops.h",                   "utility"             }, 
    { "stl_set.h",                      "set"                 }, 
    { "stl_stack.h",                    "stack"               }, 
    { "stl_tempbuf.h",                  "memory"              }, 
    { "stl_tree.h",                     "map"                 }, 
    { "stl_uninitialized.h",            "memory"              }, 
    { "stl_vector.h",                   "vector"              }, 
    { "stream_iterator.h",              "iterator"            }, 
    { "streambuf.tcc",                  "streambuf"           }, 
    { "streambuf_iterator.h",           "iterator"            }, 
    { "stringfwd.h",                    "string"              }, 
    { "strstream",                      "sstream"             }, 
    { "time_members.h",                 "locale"              }, 
    { "unique_ptr.h",                   "memory"              }, 
    { "unordered_map.h",                "unordered_map"       }, 
    { "unordered_set.h",                "unordered_set"       }, 
    { "valarray_after.h",               "valarray"            }, 
    { "valarray_array.h",               "valarray"            }, 
    { "valarray_array.tcc",             "valarray"            }, 
    { "valarray_before.h",              "valarray"            }, 
    { "vector.tcc",                     "vector"              }, 
    { "vstring.tcc",                    "vstring.h"           }, 
    { "vstring_fwd.h",                  "vstring.h"           }, 
    { "vstring_util.h",                 "vstring.h"           }, 

    { "bslstl_algorithmworkaround.h",   "bsl_algorithm.h"     }, 
    { "bslstl_allocator.h",             "bsl_memory.h"        }, 
    { "bslstl_allocatortraits.h",       "bsl_memory.h"        }, 
    { "bslstl_badweakptr.h",            "bsl_memory.h"        }, 
    { "bslstl_bidirectionaliterator.h", "bsl_iterator.h"      }, 
    { "bslstl_bitset.h",                "bsl_bitset.h"        }, 
    { "bslstl_deque.h",                 "bsl_deque.h"         }, 
    { "bslstl_equalto.h",               "bsl_functional.h"    }, 
    { "bslstl_forwarditerator.h",       "bsl_iterator.h"      }, 
    { "bslstl_hash.h",                  "bsl_functional.h"    }, 
    { "bslstl_istringstream.h",         "bsl_sstream.h"       }, 
    { "bslstl_iterator.h",              "bsl_iterator.h"      }, 
    { "bslstl_list.h",                  "bsl_list.h"          }, 
    { "bslstl_map.h",                   "bsl_map.h"           }, 
    { "bslstl_multimap.h",              "bsl_map.h"           }, 
    { "bslstl_multiset.h",              "bsl_set.h"           }, 
    { "bslstl_ostringstream.h",         "bsl_sstream.h"       }, 
    { "bslstl_pair.h",                  "bsl_utility.h"       }, 
    { "bslstl_randomaccessiterator.h",  "bsl_iterator.h"      }, 
    { "bslstl_set.h",                   "bsl_set.h"           }, 
    { "bslstl_sharedptr.h",             "bsl_memory.h"        }, 
    { "bslstl_sstream.h",               "bsl_sstream.h"       }, 
    { "bslstl_stack.h",                 "bsl_stack.h"         }, 
    { "bslstl_stdexceptutil.h",         "bsl_stdexcept.h"     }, 
    { "bslstl_string.h",                "bsl_string.h"        }, 
    { "bslstl_stringbuf.h",             "bsl_sstream.h"       }, 
    { "bslstl_stringstream.h",          "bsl_sstream.h"       }, 
    { "bslstl_unorderedmap.h",          "bsl_unordered_map.h" }, 
    { "bslstl_unorderedmultimap.h",     "bsl_unordered_map.h" }, 
    { "bslstl_unorderedmultiset.h",     "bsl_unordered_set.h" }, 
    { "bslstl_unorderedset.h",          "bsl_unordered_set.h" }, 
    { "bslstl_vector.h",                "bsl_vector.h"        }, 
    { "bslstl_allocator.h",             "bsl_memory.h"        }, 
};

std::set<llvm::StringRef> skipped_files {
    "bsl_stdhdrs_epilogue.h",
    "bsl_stdhdrs_prologue.h",
};

std::set<llvm::StringRef> reexporting_files {
    "bael_log.h",
};

struct data
    // Data attached to analyzer for this check.
{
    data();
        // Create an object of this type.

    std::vector<FileID>                                        d_fileid_stack;
    llvm::StringRef                                            d_guard;
    SourceLocation                                             d_guard_pos;
    std::map<FileID, std::map<std::string, SourceLocation>>    d_once;
    std::map<FileID,
             std::vector<std::tuple<std::string, SourceLocation, bool> > >
                                                               d_includes;
    std::map<FileID, llvm::StringRef>                          d_guards;
    std::map<std::tuple<FileID, FileID, SourceLocation>, SourceLocation>
                                                               d_fid_map;
    std::map<std::pair<SourceLocation, SourceLocation>, llvm::StringRef>
                                                               d_file_for_loc;
    std::map<SourceLocation, std::vector<std::pair<FileID, bool>>>
                                                               d_include_stack;
};

data::data()
{
}

struct report : public RecursiveASTVisitor<report>
{
    typedef RecursiveASTVisitor<report> base;

    report(Analyser& analyser,
           PPObserver::CallbackType type = PPObserver::e_None);
        // Create an object of this type, that will use the specified
        // 'analyser'.  Optionally specify a 'type' to identify the callback
        // that will be invoked, for preprocessor callbacks that have the same
        // signature.

    std::vector<std::pair<FileID, bool>>& include_stack(SourceLocation sl);
        // Get the include stack for the specified 'sl'.

    llvm::StringRef file_for_location(SourceLocation in, SourceLocation sl);
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

    void change_include(FileID fid, llvm::StringRef name);
        // Change the file name added most recently by push_include for the
        // specified 'fid', and in files which include it if not dependent on
        // BSL_OVERRIDES_STD, to the specified 'name'.

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
                    MacroDirective const *md);
        // Preprocessor callback for macro definition.

    void operator()(Token const&          token,
                    MacroDirective const *md,
                    SourceRange           range,
                    MacroArgs const      *);
        // Preprocessor callback for macro expanding.

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

    void require_file(std::string     name,
                      SourceLocation  sl,
                      llvm::StringRef symbol);
        // Indicate that the specified file 'name' is needed at the specified
        // 'sl' in order to obtain the specified 'symbol'.

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

    bool shouldVisitTemplateInstantiations () const;
        // Return true;

    bool VisitCXXConstructExpr(CXXConstructExpr *expr);
    bool VisitDeclRefExpr(DeclRefExpr *expr);
    bool VisitNamedDecl(NamedDecl *decl);
    bool VisitNamespaceAliasDecl(NamespaceAliasDecl *decl);
    bool VisitNamespaceDecl(NamespaceDecl *decl);
    bool VisitTagDecl(TagDecl *decl);
    bool VisitTemplateDecl(TemplateDecl *decl);
    bool VisitTypeLoc(TypeLoc tl);
    bool VisitTypedefNameDecl(TypedefNameDecl *decl);
    bool VisitUsingDecl(UsingDecl *decl);
    bool VisitUsingDirectiveDecl(UsingDirectiveDecl *decl);
    bool VisitValueDecl(ValueDecl *decl);
        // Return true after processing the specified 'tl' and 'expr'.

    Analyser&                d_analyser;
    data&                    d_data;
    PPObserver::CallbackType d_type;
};

report::report(Analyser& analyser, PPObserver::CallbackType type)
: d_analyser(analyser)
, d_data(analyser.attachment<data>())
, d_type(type)
{
}

std::vector<std::pair<FileID, bool>>& report::include_stack(SourceLocation sl)
{
    auto& v = d_data.d_include_stack[sl];
    if (!v.size()) {
        SourceManager& m = d_analyser.manager();
        while (sl.isValid()) {
            FileName fn(m.getFilename(sl));
            v.push_back(
                std::make_pair(m.getFileID(sl), is_top_level(fn.name())));
            sl = m.getIncludeLoc(v.back().first);
        }
    }
    return v;
}

llvm::StringRef report::file_for_location(SourceLocation sl, SourceLocation in)
{
    auto ip = std::make_pair(in, sl);
    auto i = d_data.d_file_for_loc.find(ip);
    if (i != d_data.d_file_for_loc.end()) {
        return i->second;
    }

    SourceManager& m = d_analyser.manager();
    FileID in_id = m.getFileID(in);
    FileID fid = m.getFileID(sl);
    auto& v = include_stack(sl);
    FileID top = fid;
    bool found = false;
    bool just_found = false;
    llvm::StringRef result = m.getFilename(sl);
    for (auto& p : v) {
        llvm::StringRef f = m.getFilename(m.getLocForStartOfFile(p.first));
        llvm::StringRef t = m.getFilename(m.getLocForStartOfFile(top));
        FileName ff(f);
        if (p.first == in_id) {
            result = t;
            break;
        }
        if (skipped_files.count(ff.name())) {
            continue;
        }
        if (!found) {
            if (p.second) {
                found = true;
                just_found = true;
                top = p.first;
            }
            else {
                FileName fn(t);
                auto j = mapped_files.find(ff.name());
                if (j != mapped_files.end()) {
                    found = true;
                    just_found = true;
                    top = p.first;
                }
            }
        }
        else {
            if (reexporting_files.count(ff.name()) ||
                (just_found && d_analyser.is_component_header(ff.name()))) {
                top = p.first;
            }
            just_found = false;
        }
    }
    auto j = mapped_files.find(FileName(result).name());
    if (j != mapped_files.end()) {
        result = j->second;
    }
    if (skipped_files.count(FileName(result).name())) {
        result = "";
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
    SourceManager& m = d_analyser.manager();
    bool in_header = d_analyser.is_component_header(m.getFilename(sl));
    for (FileID f : d_data.d_fileid_stack) {
        if (f == fid) {
            d_data.d_includes[f].push_back(std::make_tuple(
                name, d_analyser.get_line_range(sl).getBegin(), true));
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
            d_data.d_includes[f].push_back(std::make_tuple(
                name, d_analyser.get_line_range(sfl).getBegin(), false));
        }
    }
    llvm::StringRef file = llvm::sys::path::filename(name);
    if (file != name) {
        push_include(fid, file, sl);
    }
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
    SourceManager& m = d_analyser.manager();
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
    std::string name = d_analyser.manager().getPresumedLoc(now).getFilename();
    if (reason == PPCallbacks::EnterFile) {
        d_data.d_fileid_stack.push_back(d_analyser.manager().getFileID(now));
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

// MacroDefined
// MacroUndefined
void report::operator()(Token const&          token,
                        MacroDirective const *md)
{
}

// MacroExpands
void report::operator()(Token const&          token,
                        MacroDirective const *md,
                        SourceRange           range,
                        MacroArgs const      *)
{
    llvm::StringRef macro = token.getIdentifierInfo()->getName();
    const MacroInfo *mi = md->getMacroInfo();
    SourceManager& m = d_analyser.manager();
    Location loc(m, mi->getDefinitionLoc());
    if (loc && !range.getBegin().isMacroID() && macro != "std") {
        require_file(
            file_for_location(mi->getDefinitionLoc(), range.getBegin()),
            range.getBegin(),
            /* std::string("MacroExpands ") + */ macro.str());
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
    return is_guard_for(guard, d_analyser.manager().getFilename(sl));
}

bool report::is_guard_for(const Token& token, SourceLocation sl)
{
    return is_guard_for(token, d_analyser.manager().getFilename(sl));
}

// Ifdef
// Ifndef
void report::operator()(SourceLocation        where,
                        const Token&          token,
                        const MacroDirective *)
{
    SourceManager& m = d_analyser.manager();
    llvm::StringRef tn = token.getIdentifierInfo()->getName();

    clear_guard();

    if (!m.isInSystemHeader(where) && is_guard(token)) {
        set_guard(tn, where);
    }
}

// Defined
void report::operator()(const Token&          token,
                        const MacroDirective *,
                        SourceRange           range)
{
    clear_guard();

    llvm::StringRef tn = token.getIdentifierInfo()->getName();

    if (!d_analyser.manager().isInSystemHeader(range.getBegin()) &&
        is_guard(token)) {
        set_guard(token.getIdentifierInfo()->getName(), range.getBegin());
    }
}

// SourceRangeSkipped
void report::operator()(SourceRange range)
{
    SourceManager& m = d_analyser.manager();
    Location loc(m, range.getBegin());
    if (d_data.d_guard.size() > 0 && !m.isInSystemHeader(range.getBegin())) {
        llvm::Regex r(("ifndef +" + d_data.d_guard + "[[:space:]]+"
                       "# *include +<([^>]+)>").str());
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

void report::require_file(std::string     name,
                          SourceLocation  sl,
                          llvm::StringRef symbol)
{
    if (name.empty()) {
        return;
    }

    SourceManager& m = d_analyser.manager();
    SourceLocation orig_sl = sl;

    sl = m.getExpansionLoc(sl);

    FileID fid = m.getFileID(sl);
    while (m.isInSystemHeader(sl)) {
        sl = m.getIncludeLoc(fid);
        fid = m.getDecomposedIncludedLoc(fid).first;
    }

    FileName ff(m.getFilename(sl));
    FileName fn(name);
    name = fn.name();

    if (name == ff.name() ||
        is_top_level(ff.name()) ||
        skipped_files.count(ff.name())) {
        return;
    }

    for (const auto& p : mapped_files) {
        if (p.first == ff.name() || p.second == ff.name()) {
            return;
        }
    }

    for (const auto& p : d_data.d_includes[fid]) {
        if (std::get<0>(p) == name /*&&
            (!std::get<1>(p).isValid() ||
             !m.isBeforeInTranslationUnit(sl, std::get<1>(p)))*/) {
            return;
        }
    }

    if (!d_data.d_once[fid].count(name) /*||
        m.isBeforeInTranslationUnit(sl, d_data.d_once[fid][name])*/) {
        d_data.d_once[fid][name] = sl;
        d_analyser.report(sl, check_name, "AQK01",
                          "Need #include <%0> for '%1'")
            << name
            << symbol;
        if (d_analyser.is_component_header(ff.name())) {
            d_data.d_once[m.getMainFileID()][name] = sl;
        }
    }
}

void report::inc_for_decl(llvm::StringRef r, SourceLocation sl, const Decl *ds)
{
    SourceManager& m = d_analyser.manager();
    sl = m.getExpansionLoc(sl);

    for (const Decl *d = ds; d; d = look_through_typedef(d)) {
        bool skip = false;
        for (const Decl *p = d; !skip && p; p = p->getPreviousDecl()) {
#if 1
            Location loc(m, p->getLocation());
            FileName fn(loc.file());
            Decl::redecl_iterator rb = p->redecls_begin();
            Decl::redecl_iterator re = p->redecls_end();
            for (; !skip && rb != re; ++rb) {
                SourceLocation rl = rb->getLocation();
                if (rl.isValid() /*&& !m.isBeforeInTranslationUnit(sl, rl)*/) {
                    skip = d_analyser.is_component(file_for_location(rl, sl));
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
                    if (!skip && loc) {
                        require_file(file_for_location(rl, sl), sl, r);
                        skip = true;
                    }
                }
            }
#endif
            const UsingDecl *ud = llvm::dyn_cast<UsingDecl>(p);
            if (!skip && ud) {
                auto sb = ud->shadow_begin();
                auto se = ud->shadow_end();
                for (; !skip && sb != se; ++sb) {
                    const UsingShadowDecl *usd = *sb;
                    for (auto u = usd; !skip && u; u = u->getPreviousDecl()) {
                        inc_for_decl(r, sl, u);
                    }
                }
            }
        }
    }
}
//#define inc_for_decl(r,s,d) inc_for_decl(std::string(__FUNCTION__)+" "+r,s,d)

bool report::shouldVisitTemplateInstantiations() const
{
    return !true;
}

bool report::VisitNamespaceAliasDecl(NamespaceAliasDecl *decl)
{
#if 1
    SourceLocation sl = decl->getLocation();
    if (sl.isValid() &&
        !d_analyser.manager().isInSystemHeader(sl) &&
        !sl.isMacroID() &&
        decl->isExternallyVisible()) {
        std::string name = decl->getNameAsString();
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
        !d_analyser.manager().isInSystemHeader(sl) &&
        !sl.isMacroID() &&
        decl->isExternallyVisible() &&
        decl->getName() != "std") {
        std::string name = decl->getNameAsString();
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
        !d_analyser.manager().isInSystemHeader(sl) &&
        !sl.isMacroID() &&
        decl->isExternallyVisible()) {
        std::string name = decl->getNameAsString();
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
        !d_analyser.manager().isInSystemHeader(sl) &&
        !sl.isMacroID() &&
        decl->isExternallyVisible()) {
        std::string name = decl->getNameAsString();
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
        !d_analyser.manager().isInSystemHeader(sl) &&
        !sl.isMacroID() &&
        decl->isExternallyVisible()) {
        std::string name = decl->getNameAsString();
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
        !d_analyser.manager().isInSystemHeader(sl) &&
        !sl.isMacroID() &&
        decl->isExternallyVisible()) {
        std::string name = decl->getNameAsString();
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
        !d_analyser.manager().isInSystemHeader(sl) &&
        !sl.isMacroID() &&
        decl->isExternallyVisible()) {
        std::string name = decl->getNameAsString();
        inc_for_decl(name, sl, decl);
    }
#endif
    return true;
}

bool report::VisitNamedDecl(NamedDecl *decl)
{
#if 0
    SourceLocation sl = decl->getLocation();
    if (sl.isValid() &&
        !d_analyser.manager().isInSystemHeader(sl) &&
        !sl.isMacroID() &&
        decl->isExternallyVisible()) {
        std::string name = decl->getNameAsString();
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
        !d_analyser.manager().isInSystemHeader(sl) &&
        !sl.isMacroID() &&
        nd->isExternallyVisible()) {
        inc_for_decl(nd->getNameAsString(), sl, nd);
    }
#endif
    return base::VisitUsingDirectiveDecl(decl);
}

bool report::VisitDeclRefExpr(DeclRefExpr *expr)
{
#if 1
    SourceLocation sl = expr->getExprLoc();
    if (sl.isValid() &&
        !d_analyser.manager().isInSystemHeader(sl) &&
        !sl.isMacroID()) {
        const NamedDecl *ds = expr->getFoundDecl();
        const DeclContext *dc = ds->getDeclContext();
        std::string name = expr->getNameInfo().getName().getAsString();
        while (dc->isRecord()) {
            name = llvm::dyn_cast<NamedDecl>(dc)->getNameAsString();
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
    if (sl.isValid() &&
        !d_analyser.manager().isInSystemHeader(sl) &&
        !sl.isMacroID()) {
        const NamedDecl *ds = expr->getConstructor()->getParent();
        const DeclContext *dc = ds->getDeclContext();
        std::string name = ds->getNameAsString();
        while (dc->isRecord()) {
            name = llvm::dyn_cast<NamedDecl>(dc)->getNameAsString();
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

bool report::VisitTypeLoc(TypeLoc tl)
{
#if 1
    SourceManager& m = d_analyser.manager();
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
        if (ds && sl.isValid() && !sl.isMacroID() && !m.isInSystemHeader(sl)) {
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
    observer.onPPMacroDefined       += report(analyser,
                                                      observer.e_MacroDefined);
    observer.onPPMacroUndefined     += report(analyser,
                                                    observer.e_MacroUndefined);
    observer.onPPMacroExpands       += report(analyser,
                                                      observer.e_MacroExpands);
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
