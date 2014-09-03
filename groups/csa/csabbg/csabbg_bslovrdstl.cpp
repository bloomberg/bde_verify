// csabbg_bslovrdstl.cpp                                              -*-C++-*-

#include <clang/AST/Decl.h>
#include <clang/AST/DeclBase.h>
#include <clang/AST/DeclCXX.h>
#include <clang/AST/Type.h>
#include <clang/Basic/IdentifierTable.h>
#include <clang/Basic/SourceLocation.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Lex/MacroInfo.h>
#include <clang/Lex/PPCallbacks.h>
#include <clang/Lex/Token.h>
#include <clang/Rewrite/Core/Rewriter.h>
#include <csabase_analyser.h>
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
#include <map>
#include <set>
#include <string>
#include <utility>
#include <utils/event.hpp>
#include <utils/function.hpp>
#include <vector>
namespace clang { class FileEntry; }
namespace clang { class MacroArgs; }
namespace clang { class Module; }
namespace csabase { class Visitor; }

using namespace csabase;
using namespace clang::ast_matchers;
using namespace clang::ast_matchers::internal;
using namespace clang;

// ----------------------------------------------------------------------------

static std::string const check_name("bsl-overrides-std");

// ----------------------------------------------------------------------------

namespace
{

enum FileType { e_NIL, e_BSL, e_STD, e_OBS };

struct file_info {
    const char *bsl;
    const char *bsl_guard;
    const char *std;
    const char *std_guard;
};

struct data
    // Data attached to analyzer for this check.
{
    data();
        // Create an object of this type.

    std::vector<std::string>                             d_file_stack;
    std::set<std::string>                                d_inc_files;
    bool                                                 d_in_bsl;
    bool                                                 d_in_std;
    bool                                                 d_is_obsolete;
    bool                                                 d_bsl_overrides_std;
    llvm::StringRef                                      d_guard;
    SourceLocation                                       d_guard_pos;
    std::set<std::pair<std::string, SourceLocation>>     d_std_names;
    std::map<std::string, std::string>                   d_file_map;
    std::map<FileID, std::set<std::string>>              d_once;
    std::map<FileID, std::vector<std::pair<std::string, SourceLocation>>>
                                                         d_includes;
    std::map<std::string, std::vector<const file_info *>>
                                                         d_file_info;
};

data::data()
: d_in_bsl(false)
, d_in_std(false)
, d_is_obsolete(false)
{
}

const file_info includes[] = {
    { "bsl_algorithm.h",     "INCLUDED_BSL_ALGORITHM",
      "algorithm",           "INCLUDED_ALGORITHM"          },
    { "bsl_bitset.h",        "INCLUDED_BSL_BITSET",
      "bitset",              "INCLUDED_BITSET"             },
    { "bsl_cassert.h",       "INCLUDED_BSL_CASSERT",
      "cassert",             "INCLUDED_CASSERT"            },
    { "bsl_cctype.h",        "INCLUDED_BSL_CCTYPE",
      "cctype",              "INCLUDED_CCTYPE"             },
    { "bsl_cerrno.h",        "INCLUDED_BSL_CERRNO",
      "cerrno",              "INCLUDED_CERRNO"             },
    { "bsl_cfloat.h",        "INCLUDED_BSL_CFLOAT",
      "cfloat",              "INCLUDED_CFLOAT"             },
    { "bsl_ciso646.h",       "INCLUDED_BSL_CISO646",
      "ciso646",             "INCLUDED_CISO646"            },
    { "bsl_climits.h",       "INCLUDED_BSL_CLIMITS",
      "climits",             "INCLUDED_CLIMITS"            },
    { "bsl_clocale.h",       "INCLUDED_BSL_CLOCALE",
      "clocale",             "INCLUDED_CLOCALE"            },
    { "bsl_cmath.h",         "INCLUDED_BSL_CMATH",
      "cmath",               "INCLUDED_CMATH"              },
    { "bsl_complex.h",       "INCLUDED_BSL_COMPLEX",
      "complex",             "INCLUDED_COMPLEX"            },
    { "bsl_csetjmp.h",       "INCLUDED_BSL_CSETJMP",
      "csetjmp",             "INCLUDED_CSETJMP"            },
    { "bsl_csignal.h",       "INCLUDED_BSL_CSIGNAL",
      "csignal",             "INCLUDED_CSIGNAL"            },
    { "bsl_cstdarg.h",       "INCLUDED_BSL_CSTDARG",
      "cstdarg",             "INCLUDED_CSTDARG"            },
    { "bsl_cstddef.h",       "INCLUDED_BSL_CSTDDEF",
      "cstddef",             "INCLUDED_CSTDDEF"            },
    { "bsl_cstdio.h",        "INCLUDED_BSL_CSTDIO",
      "cstdio",              "INCLUDED_CSTDIO"             },
    { "bsl_cstdlib.h",       "INCLUDED_BSL_CSTDLIB",
      "cstdlib",             "INCLUDED_CSTDLIB"            },
    { "bsl_cstring.h",       "INCLUDED_BSL_CSTRING",
      "cstring",             "INCLUDED_CSTRING"            },
    { "bsl_ctime.h",         "INCLUDED_BSL_CTIME",
      "ctime",               "INCLUDED_CTIME"              },
    { "bsl_cwchar.h",        "INCLUDED_BSL_CWCHAR",
      "cwchar",              "INCLUDED_CWCHAR"             },
    { "bsl_cwctype.h",       "INCLUDED_BSL_CWCTYPE",
      "cwctype",             "INCLUDED_CWCTYPE"            },
    { "bsl_deque.h",         "INCLUDED_BSL_DEQUE",
      "deque",               "INCLUDED_DEQUE"              },
    { "bsl_exception.h",     "INCLUDED_BSL_EXCEPTION",
      "exception",           "INCLUDED_EXCEPTION"          },
    { "bsl_fstream.h",       "INCLUDED_BSL_FSTREAM",
      "fstream",             "INCLUDED_FSTREAM"            },
    { "bsl_functional.h",    "INCLUDED_BSL_FUNCTIONAL",
      "functional",          "INCLUDED_FUNCTIONAL"         },
    { "bsl_hash_map.h",      "INCLUDED_BSL_HASH_MAP",
      "hash_map",            "INCLUDED_HASH_MAP"           },
    { "bsl_hash_set.h",      "INCLUDED_BSL_HASH_SET",
      "hash_set",            "INCLUDED_HASH_SET"           },
    { "bsl_iomanip.h",       "INCLUDED_BSL_IOMANIP",
      "iomanip",             "INCLUDED_IOMANIP"            },
    { "bsl_ios.h",           "INCLUDED_BSL_IOS",
      "ios",                 "INCLUDED_IOS"                },
    { "bsl_iosfwd.h",        "INCLUDED_BSL_IOSFWD",
      "iosfwd",              "INCLUDED_IOSFWD"             },
    { "bsl_iostream.h",      "INCLUDED_BSL_IOSTREAM",
      "iostream",            "INCLUDED_IOSTREAM"           },
    { "bsl_istream.h",       "INCLUDED_BSL_ISTREAM",
      "istream",             "INCLUDED_ISTREAM"            },
    { "bsl_iterator.h",      "INCLUDED_BSL_ITERATOR",
      "iterator",            "INCLUDED_ITERATOR"           },
    { "bsl_limits.h",        "INCLUDED_BSL_LIMITS",
      "limits",              "INCLUDED_LIMITS"             },
    { "bsl_list.h",          "INCLUDED_BSL_LIST",
      "list",                "INCLUDED_LIST"               },
    { "bsl_locale.h",        "INCLUDED_BSL_LOCALE",
      "locale",              "INCLUDED_LOCALE"             },
    { "bsl_map.h",           "INCLUDED_BSL_MAP",
      "map",                 "INCLUDED_MAP"                },
    { "bsl_memory.h",        "INCLUDED_BSL_MEMORY",
      "memory",              "INCLUDED_MEMORY"             },
    { "bsl_new.h",           "INCLUDED_BSL_NEW",
      "new",                 "INCLUDED_NEW"                },
    { "bsl_numeric.h",       "INCLUDED_BSL_NUMERIC",
      "numeric",             "INCLUDED_NUMERIC"            },
    { "bsl_ostream.h",       "INCLUDED_BSL_OSTREAM",
      "ostream",             "INCLUDED_OSTREAM"            },
    { "bsl_queue.h",         "INCLUDED_BSL_QUEUE",
      "queue",               "INCLUDED_QUEUE"              },
    { "bsl_set.h",           "INCLUDED_BSL_SET",
      "set",                 "INCLUDED_SET"                },
    { "bsl_slist.h",         "INCLUDED_BSL_SLIST",
      "slist",               "INCLUDED_SLIST"              },
    { "bsl_sstream.h",       "INCLUDED_BSL_SSTREAM",
      "sstream",             "INCLUDED_SSTREAM"            },
    { "bsl_stack.h",         "INCLUDED_BSL_STACK",
      "stack",               "INCLUDED_STACK"              },
    { "bsl_stdexcept.h",     "INCLUDED_BSL_STDEXCEPT",
      "stdexcept",           "INCLUDED_STDEXCEPT"          },
    { "bsl_streambuf.h",     "INCLUDED_BSL_STREAMBUF",
      "streambuf",           "INCLUDED_STREAMBUF"          },
    { "bsl_string.h",        "INCLUDED_BSL_STRING",
      "string",              "INCLUDED_STRING"             },
    { "bsl_strstream.h",     "INCLUDED_BSL_STRSTREAM",
      "strstream",           "INCLUDED_STRSTREAM"          },
    { "bsl_typeinfo.h",      "INCLUDED_BSL_TYPEINFO",
      "typeinfo",            "INCLUDED_TYPEINFO"           },
    { "bsl_unordered_map.h", "INCLUDED_BSL_UNORDERED_MAP",
      "unordered_map",       "INCLUDED_UNORDERED_MAP"      },
    { "bsl_unordered_set.h", "INCLUDED_BSL_UNORDERED_SET",
      "unordered_set",       "INCLUDED_UNORDERED_SET"      },
    { "bsl_utility.h",       "INCLUDED_BSL_UTILITY",
      "utility",             "INCLUDED_UTILITY"            },
    { "bsl_valarray.h",      "INCLUDED_BSL_VALARRAY",
      "valarray",            "INCLUDED_VALARRAY"           },
    { "bsl_vector.h",        "INCLUDED_BSL_VECTOR",
      "vector",              "INCLUDED_VECTOR"             },

    // GCC has some cross-includes that cause problems.  For example, <ios>
    // <stl_algobase.h> without going through <algorithm>, so the replacement
    // of std::max with bsl::max fails when <ios> is replaced with <bsl_ios.h>.
    // This section will include these sepcial non-standard headers.

    { "bsl_algorithm.h",  "INCLUDED_BSL_ALGORITHM",
      "stl_algobase.h",   "_ALGOBASE_H"              },
    { "bsl_functional.h", "INCLUDED_BSL_FUNCTIONAL",
      "stl_function.h",   "_FUNCTION_H"              },
    { "bsl_utility.h",    "INCLUDED_BSL_UTILITY",
      "stl_pair.h",       "_PAIR_H"                  },
    { "bsl_ios.h",        "INCLUDED_BSL_IOS",
      "postypes.h",       "_GLIBCXX_POSTYPES_H"      },

    // There are some bsls_ files that include standarda headers.

    { "bsl_cstddef.h",          "INCLUDED_BSL_CSTDDEF",
      "bsls_alignment.h",       "INCLUDED_BSLS_ALIGNMENT"        },
    { "bsl_limits.h",           "INCLUDED_BSL_LIMITS",
      "bsls_alignmentutil.h",   "INCLUDED_BSLS_ALIGNMENTUTIL"    },
    { "bsl_cstddef.h",          "INCLUDED_BSL_CSTDDEF",
      "bsls_platformutil.h",    "INCLUDED_BSLS_PLATFORMUTIL"     },
    { "bsl_iosfwd.h",           "INCLUDED_BSL_IOSFWD",
      "bsls_systemclocktype.h", "INCLUDED_BSLS_SYSTEMCLOCK_TYPE" },
    { "bsl_ostream.h",          "INCLUDED_BSL_OSTREAM",
      "bsls_timeinterval.h",    "INCLUDED_BSLS_TIMEINTERVAL"     },
    { "bsl_cstddef.h",          "INCLUDED_BSL_CSTDDEF",
      "bsls_types.h",           "INCLUDED_BSLS_TYPES"            },

    // 'bsl_' equivalents for 'bslstl_' files
    { "bsl_algorithm.h",                  "INCLUDED_BSL_ALGORITHM",
      "bslstl_algorithmworkaround.h",  "INCLUDED_BSLSTL_ALGORITHMWORKAROUND" },
    { "bsl_memory.h",                     "INCLUDED_BSL_MEMORY",
      "bslstl_allocator.h",               "INCLUDED_BSLSTL_ALLOCATOR"        },
    { "bsl_memory.h",                     "INCLUDED_BSL_MEMORY",
      "bslstl_allocatortraits.h",         "INCLUDED_BSLSTL_ALLOCATORTRAITS"  },
    { "bsl_memory.h",                     "INCLUDED_BSL_MEMORY",
      "bslstl_badweakptr.h",              "INCLUDED_BSLSTL_BADWEAKPTR"       },
    { "bsl_iterator.h",                   "INCLUDED_BSL_ITERATOR",
      "bslstl_bidirectionaliterator.h",
                                     "INCLUDED_BSLSTL_BIDIRECTIONALITERATOR" },
    { "bsl_set.h",                        "INCLUDED_BSL_SET",
      "bslstl_bidirectionalnodepool.h",
                                     "INCLUDED_BSLSTL_BIDIRECTIONALNODEPOOL" },
    { "bsl_bitset.h",                     "INCLUDED_BSL_BITSET",
      "bslstl_bitset.h",                  "INCLUDED_BSLSTL_BITSET"           },
    { "bsl_deque.h",                      "INCLUDED_BSL_DEQUE",
      "bslstl_deque.h",                   "INCLUDED_BSLSTL_DEQUE"            },
    { "bsl_functional.h",                 "INCLUDED_BSL_FUNCTIONAL",
      "bslstl_equalto.h",                 "INCLUDED_BSLSTL_EQUALTO"          },
    { "bsl_iterator.h",                   "INCLUDED_BSL_ITERATOR",
      "bslstl_forwarditerator.h",         "INCLUDED_BSLSTL_FORWARDITERATOR"  },
    { "bsl_functional.h",                 "INCLUDED_BSL_FUNCTIONAL",
      "bslstl_hash.h",                    "INCLUDED_BSLSTL_HASH"             },
    { "bsl_unordered_map.h",              "INCLUDED_BSL_UNORDERED_MAP",
      "bslstl_hashtable.h",               "INCLUDED_BSLSTL_HASHTABLE"        },
    { "bsl_unordered_map.h",              "INCLUDED_BSL_UNORDERED_MAP",
      "bslstl_hashtablebucketiterator.h",
                                   "INCLUDED_BSLSTL_HASHTABLEBUCKETITERATOR" },
    { "bsl_unordered_map.h",              "INCLUDED_BSL_UNORDERED_MAP",
      "bslstl_hashtableiterator.h",      "INCLUDED_BSLSTL_HASHTABLEITERATOR" },
    { "bsl_iosfwd.h"                      "INCLUDED_BSL_IOSFWD",
      "bslstl_iosfwd.h",                  "INCLUDED_BSLSTL_IOSFWD"           },
    { "bsl_sstream.h",                    "INCLUDED_BSL_SSTREAM",
      "bslstl_istringstream.h",           "INCLUDED_BSLSTL_ISTRINGSTREAM"    },
    { "bsl_iterator.h",                   "INCLUDED_BSL_ITERATOR",
      "bslstl_iterator.h",                "INCLUDED_BSLSTL_ITERATOR"         },
    { "bsl_unordered_set.h",              "INCLUDED_BSL_UNORDERED_SET",
      "bslstl_iteratorutil.h",            "INCLUDED_BSLSTL_ITERATORUTIL"     },
    { "bsl_list.h",                       "INCLUDED_BSL_LIST",
      "bslstl_list.h",                    "INCLUDED_BSLSTL_LIST"             },
    { "bsl_map.h",                        "INCLUDED_BSL_MAP",
      "bslstl_map.h",                     "INCLUDED_BSLSTL_MAP"              },
    { "bsl_map.h",                        "INCLUDED_BSL_MAP",
      "bslstl_mapcomparator.h",           "INCLUDED_BSLSTL_MAPCOMPARATOR"    },
    { "bsl_map.h",                        "INCLUDED_BSL_MAP",
      "bslstl_multimap.h",                "INCLUDED_BSLSTL_MULTIMAP"         },
    { "bsl_set.h",                        "INCLUDED_BSL_SET",
      "bslstl_multiset.h",                "INCLUDED_BSLSTL_MULTISET"         },
    { "bsl_sstream.h",                    "INCLUDED_BSL_SSTREAM",
      "bslstl_ostringstream.h",           "INCLUDED_BSLSTL_OSTRINGSTREAM"    },
    { "bsl_utility.h",                    "INCLUDED_BSL_UTILITY",
      "bslstl_pair.h",                    "INCLUDED_BSLSTL_PAIR"             },
    { "bsl_queue.h",                      "INCLUDED_BSL_QUEUE",
      "bslstl_queue.h",                   "INCLUDED_BSLSTL_QUEUE"            },
    { "bsl_iterator.h",                   "INCLUDED_BSL_ITERATOR",
      "bslstl_randomaccessiterator.h",
                                      "INCLUDED_BSLSTL_RANDOMACCESSITERATOR" },
    { "bsl_set.h",                        "INCLUDED_BSL_SET",
      "bslstl_set.h",                     "INCLUDED_BSLSTL_SET"              },
    { "bsl_set.h",                        "INCLUDED_BSL_SET",
      "bslstl_setcomparator.h",           "INCLUDED_BSLSTL_SETCOMPARATOR"    },
    { "bsl_memory.h",                     "INCLUDED_BSL_MEMORY",
      "bslstl_sharedptr.h",               "INCLUDED_BSLSTL_SHAREDPTR"        },
    { "bsl_set.h",                        "INCLUDED_BSL_SET",
      "bslstl_simplepool.h",              "INCLUDED_BSLSTL_SIMPLEPOOL"       },
    { "bsl_sstream.h",                    "INCLUDED_BSL_SSTREAM",
      "bslstl_sstream.h",                 "INCLUDED_BSLSTL_SSTREAM"          },
    { "bsl_stack.h",                      "INCLUDED_BSL_STACK",
      "bslstl_stack.h",                   "INCLUDED_BSLSTL_STACK"            },
    { "bsl_bitset.h",                     "INCLUDED_BSL_BITSET",
      "bslstl_stdexceptutil.h",           "INCLUDED_BSLSTL_STDEXCEPTUTIL"    },
    { "bsl_string.h",                     "INCLUDED_BSL_STRING",
      "bslstl_string.h",                  "INCLUDED_BSLSTL_STRING"           },
    { "bsl_sstream.h",                    "INCLUDED_BSL_SSTREAM",
      "bslstl_stringbuf.h",               "INCLUDED_BSLSTL_STRINGBUF"        },
    { "bsl_string.h",                     "INCLUDED_BSL_STRING",
      "bslstl_stringref.h",               "INCLUDED_BSLSTL_STRINGREF"        },
    { "bsl_string.h",                     "INCLUDED_BSL_STRING",
      "bslstl_stringrefdata.h",           "INCLUDED_BSLSTL_STRINGREFDATA"    },
    { "bsl_sstream.h",                    "INCLUDED_BSL_SSTREAM",
      "bslstl_stringstream.h",            "INCLUDED_BSLSTL_STRINGSTREAM"     },
    { "bsl_set.h",                        "INCLUDED_BSL_SET",
      "bslstl_treeiterator.h",            "INCLUDED_BSLSTL_TREEITERATOR"     },
    { "bsl_set.h",                        "INCLUDED_BSL_SET",
      "bslstl_treenode.h",                "INCLUDED_BSLSTL_TREENODE"         },
    { "bsl_set.h",                        "INCLUDED_BSL_SET",
      "bslstl_treenodepool.h",            "INCLUDED_BSLSTL_TREENODEPOOL"     },
    { "bsl_unordered_map.h",              "INCLUDED_BSL_UNORDERED_MAP",
      "bslstl_unorderedmap.h",            "INCLUDED_BSLSTL_UNORDEREDMAP"     },
    { "bsl_unordered_map.h",              "INCLUDED_BSL_UNORDERED_MAP",
      "bslstl_unorderedmapkeyconfiguration.h",
                              "INCLUDED_BSLSTL_UNORDEREDMAPKEYCONFIGURATION" },
    { "bsl_unordered_map.h",              "INCLUDED_BSL_UNORDERED_MAP",
      "bslstl_unorderedmultimap.h",      "INCLUDED_BSLSTL_UNORDEREDMULTIMAP" },
    { "bsl_unordered_set.h",              "INCLUDED_BSL_UNORDERED_SET",
      "bslstl_unorderedmultiset.h",      "INCLUDED_BSLSTL_UNORDEREDMULTISET" },
    { "bsl_unordered_set.h",              "INCLUDED_BSL_UNORDERED_SET",
      "bslstl_unorderedset.h",            "INCLUDED_BSLSTL_UNORDEREDSET"     },
    { "bsl_unordered_set.h",              "INCLUDED_BSL_UNORDERED_SET",
      "bslstl_unorderedsetkeyconfiguration.h",
                              "INCLUDED_BSLSTL_UNORDEREDSETKEYCONFIGURATION" },
    { "bsl_vector.h",                     "INCLUDED_BSL_VECTOR",
      "bslstl_vector.h",                  "INCLUDED_BSLSTL_VECTOR"           },

    // 'bsl_' equivalents for 'bslstp_' files
    { "bsl_algorithm.h",       "INCLUDED_BSL_ALGORITHM",      
      "bslstp_exalgorithm.h",  "INCLUDE_BSLSTP_EXALGORITHM"   }, 
    { "bsl_functional.h",      "INCLUDED_BSL_FUNCTIONAL",     
      "bslstp_exfunctional.h", "INCLUDE_BSLSTP_EXFUNCTIONAL"  }, 
    { "bsl_cstddef.h",         "INCLUDED_BSL_CSTDDEF",        
      "bslstp_hash.h",         "INCLUDE_BSLSTP_HASH"          }, 
    { "bsl_functional.h",      "INCLUDED_BSL_FUNCTIONAL",     
      "bslstp_hashmap.h",      "INCLUDE_BSLSTP_HASHMAP"       }, 
    { "bsl_functional.h",      "INCLUDED_BSL_FUNCTIONAL",     
      "bslstp_hashset.h",      "INCLUDE_BSLSTP_HASHSET"       }, 
    { "bsl_algorithm.h",       "INCLUDED_BSL_ALGORITHM",      
      "bslstp_hashtable.h",    "INCLUDE_BSLSTP_HASHTABLE"     }, 
    { "bsl_functional.h",      "INCLUDED_BSL_FUNCTIONAL",     
      "bslstp_hashtable.h",    "INCLUDE_BSLSTP_HASHTABLE"     }, 
    { "bsl_iterator.h",        "INCLUDED_BSL_ITERATOR",       
      "bslstp_hashtable.h",    "INCLUDE_BSLSTP_HASHTABLE"     }, 
    { "bsl_cstddef.h",         "INCLUDED_BSL_CSTDDEF",        
      "bslstp_iterator.h",     "INCLUDE_BSLSTP_ITERATOR"      }, 
    { "bsl_algorithm.h",       "INCLUDED_BSL_ALGORITHM",      
      "bslstp_slist.h",        "INCLUDE_BSLSTP_SLIST"         }, 
    { "bsl_cstddef.h",         "INCLUDED_BSL_CSTDDEF",        
      "bslstp_slist.h",        "INCLUDE_BSLSTP_SLIST"         }, 
    { "bsl_iterator.h",        "INCLUDED_BSL_ITERATOR",       
      "bslstp_slist.h",        "INCLUDE_BSLSTP_SLIST"         }, 
    { "bsl_cstddef.h",         "INCLUDED_BSL_CSTDDEF",        
      "bslstp_slistbase.h",    "INCLUDE_BSLSTP_SLISTBASE"     }, 
};
size_t num_includes = sizeof includes / sizeof *includes;

const char * obsolete[] = {
    "exception.h",
    "fstream.h",
    "iomanip.h",
    "ios.h",
    "iosfwd.h",
    "iostream.h",
    "istream.h",
    "memory.h",
    "new.h",
    "ostream.h",
    "streambuf.h",
    "strstream.h",
    "typeinfo.h",
};
size_t num_obsolete = sizeof obsolete / sizeof *obsolete;

const char *good_bsl[] = {
    "bslalg_",
    "bslfwd_",
    "bslim_",
    "bslma_",
    "bslmf_",
    "bsls_",
    "bslscm_",
    "bsltf_",
};
size_t num_good_bsl = sizeof good_bsl / sizeof *good_bsl;

struct report
{
    report(Analyser& analyser,
           PPObserver::CallbackType type = PPObserver::e_None);
        // Create an object of this type, that will use the specified
        // 'analyser'.  Optionally specify a 'type' to identify the callback
        // that will be invoked, for preprocessor callbacks that have the same
        // signature.

    FileType classify(llvm::StringRef name,
                      const std::vector<const file_info *> **pfvi = 0);
        // Return one of the 'FileType' enumerators describing the specified
        // 'name'.  Optionally specify 'pfvi' to receive the corresponding file
        // data.

    llvm::StringRef filetype_tag(FileType ft);
        // Return a string representation of the specified 'ft'.

    void classify_stack();
        // Set the location flags in the associated data using the lowest
        // classifiable file on the include stack.

    void dump_file_stack();
        // Print the file stack, classifying each file.

    void clear_guard();
        // Clear the current include guard info.

    void set_guard(llvm::StringRef guard, SourceLocation where);
        // Set the guard info to the specified 'guard' and 'where'.

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

    void add_include(FileID fid, const std::string& name);
        // Include the file specified by 'name' in the file specified by 'fid'.

    void require_file(std::string     name,
                      FileID          fid,
                      SourceLocation  sl,
                      llvm::StringRef symbol);
        // Indicate that the specified file 'name' is needed at the specified
        // 'sl' and 'fid' in order to obtain the specified 'symbol'.

    void inc_for_std_decl(llvm::StringRef  r,
                          FileID           fid,
                          SourceLocation   sl,
                          Decl            *ds);
        // For the specified name 'r' at location 'sl' referenced by the
        // specified declaration 'ds', determine which header file, if any, is
        // needed by the file specified by 'fid'.


    Decl *look_through_typedef(Decl *ds);
        // If the specified 'ds' is a typedef for a record, return the
        // definition for the record if it exists.  Return null otherwise.

    void operator()();
        // Callback for end of main file.

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

FileType report::classify(llvm::StringRef name,
                          const std::vector<const file_info *> **pfvi)
{
    FileName fn(name);
    name = fn.name();
    const std::vector<const file_info *> &fvi = d_data.d_file_info[name];
    if (pfvi) {
        *pfvi = &fvi;
    }
    for (const file_info *f : fvi) {
        if (name == f->std) {
            return FileType::e_STD;                                   // RETURN
        }
        if (name == f->bsl) {
            return FileType::e_BSL;                                   // RETURN
        }
    }

    for (const char *prefix : good_bsl) {
        if (name.startswith(prefix)) {
            return FileType::e_BSL;                                   // RETURN
        }
    }

    for (const char *file : obsolete) {
        if (fn.name() == file) {
            return FileType::e_OBS;                                   // RETURN
        }
    }

    return FileType::e_NIL;
}

llvm::StringRef report::filetype_tag(FileType ft)
{
    switch (ft) {
      case FileType::e_BSL: return "BSL";                             // RETURN
      case FileType::e_STD: return "STD";                             // RETURN
      case FileType::e_OBS: return "OBS";                             // RETURN
      case FileType::e_NIL: return "   ";                             // RETURN
    }
}

void report::classify_stack()
{
    d_data.d_in_bsl = false;
    d_data.d_in_std = false;
    d_data.d_is_obsolete = false;
    for (const auto& file : d_data.d_file_stack) {
        switch (classify(file)) {
          case FileType::e_BSL:
            d_data.d_in_bsl = true;
            return;                                                   // RETURN
          case FileType::e_STD:
            d_data.d_in_std = true;
            return;                                                   // RETURN
          case FileType::e_OBS:
            d_data.d_is_obsolete = true;
            break;
          case FileType::e_NIL:
            break;
        }
    }
}

void report::dump_file_stack()
{
    for (const auto &file : d_data.d_file_stack) {
        ERRS() << filetype_tag(classify(file)) << " " << file; ERNL();
    }
    ERNL();
}

void report::clear_guard()
{
    d_data.d_guard = "";
    d_data.d_guard_pos = SourceLocation();
}

void report::set_guard(llvm::StringRef guard, SourceLocation where)
{
    d_data.d_guard = guard;
    d_data.d_guard_pos = where;
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
    if (!d_data.d_in_std && !d_data.d_in_bsl) {
        FileName fn(name);
        d_data.d_inc_files.insert(fn.name());
        // ERRS() << "Toplevel " << name; ERNL();
    } else {
        // ERRS() << "Nested " << name; ERNL();
    }

    FileID fid = d_analyser.manager().getFileID(where);
    if (d_data.d_guard_pos.isValid() &&
        fid != d_analyser.manager().getFileID(d_data.d_guard_pos)) {
        clear_guard();
    }

    d_data.d_includes[fid].push_back(std::make_pair(
        name, d_data.d_guard_pos.isValid() ? d_data.d_guard_pos : where));

    const std::vector<const file_info *> *pfvi;

    if (!d_data.d_in_bsl &&
        !d_data.d_in_std &&
        d_analyser.is_component(where) &&
        !d_analyser.manager().isInSystemHeader(where) &&
        classify(name, &pfvi) == FileType::e_STD) {
        for (const file_info *fi : *pfvi) {
            if (d_data.d_guard == fi->std_guard) {
                SourceRange r = d_analyser.get_line_range(d_data.d_guard_pos);
                llvm::StringRef s = d_analyser.get_source(r);
                size_t pos = s.find(d_data.d_guard);
                if (pos != s.npos) {
                    d_analyser.report(r.getBegin(), check_name, "SB02",
                                      "Replacing include guard %0 with %1")
                        << fi->std_guard
                        << fi->bsl_guard;
                    d_analyser.rewriter().ReplaceText(
                        getOffsetRange(r, pos, d_data.d_guard.size() - 1),
                        fi->bsl_guard);
                    d_analyser.report(where, check_name, "SB01",
                                      "Replacing header %2%0%3 with <%1>")
                        << fi->std
                        << fi->bsl
                        << (angled ? "<" : "\"")
                        << (angled ? ">" : "\"");
                    d_analyser.rewriter().ReplaceText(
                        namerange.getAsRange(),
                        "<" + std::string(fi->bsl) + ">");
                    if (d_data.d_guard == fi->std_guard) {
                        SourceRange r =
                            d_analyser.get_line_range(d_data.d_guard_pos);
                        r = d_analyser.get_line_range(
                            r.getEnd().getLocWithOffset(1));
                        r = d_analyser.get_line_range(
                            r.getEnd().getLocWithOffset(1));
                        llvm::StringRef s = d_analyser.get_source(r, false);
                        size_t pos = s.find("#define " + d_data.d_guard.str());
                        if (pos != s.npos) {
                            d_analyser.report(r.getBegin(), check_name, "SB03",
                                     "Removing include guard definition of %0")
                                << d_data.d_guard;
                            Rewriter::RewriteOptions ro;
                            ro.RemoveLineIfEmpty = true;
                            d_analyser.rewriter().RemoveText(r, ro);
                        }
                    }
                }
            }
        }
    }

    clear_guard();
}

void report::map_file(std::string name)
{
    if (d_data.d_file_map.find(name) == d_data.d_file_map.end()) {
        std::string file = name;
        for (const auto& s : d_data.d_file_stack) {
            auto c = classify(s);
            if (c == e_STD || c == e_BSL) {
                file = s;
            }
        }
        // ERRS() << "Mapping " << name << " to " << file; ERNL();
        d_data.d_file_map[name] = file;
    }
}

// FileChanged
void report::operator()(SourceLocation                now,
                        PPCallbacks::FileChangeReason reason,
                        SrcMgr::CharacteristicKind    type,
                        FileID                        prev)
{
    // dump_file_stack();
    std::string name = d_analyser.manager().getPresumedLoc(now).getFilename();
    if (reason == PPCallbacks::EnterFile) {
        d_data.d_file_stack.push_back(name);
    } else if (reason == PPCallbacks::ExitFile) {
        if (d_data.d_file_stack.size() > 0) {
            d_data.d_file_stack.pop_back();
        }
    }

    classify_stack();

    if (d_data.d_in_std || d_data.d_in_bsl) {
        map_file(name);
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
    if (d_type == PPObserver::e_MacroUndefined) {
        if (is_named(token, "std")) {
            d_data.d_bsl_overrides_std = false;
        }
        return;                                                       // RETURN
    }

    if (md) {
        if (const MacroInfo *mi = md->getMacroInfo()) {
            Location loc(d_analyser.manager(), mi->getDefinitionLoc());
            if (loc) {
                map_file(loc.file());
                // ERRS() << token.getIdentifierInfo()->getName(); ERNL();
                // dump_file_stack();
            }
            int nt = mi->getNumTokens();
            if (is_named(token, "std") &&
                mi->isObjectLike() &&
                nt == 1 &&
                is_named(mi->getReplacementToken(0), "bsl")) {
                d_data.d_bsl_overrides_std = true;
            } else if (d_data.d_bsl_overrides_std &&
                       d_analyser.is_component(mi->getDefinitionLoc()) &&
                       !d_analyser.manager().isInSystemHeader(
                            mi->getDefinitionLoc())) {
                for (int i = 0; i < nt; ++i) {
                    const Token &token = mi->getReplacementToken(i);
                    if (is_named(token, "std")) {
                        if (!d_analyser.rewriter().ReplaceText(
                                 token.getLocation(),
                                 token.getLength(),
                                 "bsl")) {
                            d_analyser.report(token.getLocation(),
                                              check_name, "SB07",
                                              "Replacing 'std' with 'bsl' in "
                                              "macro definition");
                        }
                        for (int j = i + 1; j < nt; ++j) {
                            const Token &name = mi->getReplacementToken(j);
                            if (name.isAnyIdentifier()) {
                                d_data.d_std_names.insert(std::make_pair(
                                    name.getIdentifierInfo()->getName().str(),
                                    name.getLocation()));
                                break;
                            }
                        }
                    }
                }
            }
        }
    }
}

// MacroExpands
void report::operator()(Token const&          token,
                        MacroDirective const *md,
                        SourceRange           range,
                        MacroArgs const      *)
{
    const MacroInfo *mi = md->getMacroInfo();

    if (d_analyser.is_component(range.getBegin()) &&
        !token.getLocation().isMacroID()) {
        Location loc(d_analyser.manager(), mi->getDefinitionLoc());
        if (loc) {
            require_file(loc.file(),
                         d_analyser.manager().getFileID(range.getBegin()),
                         range.getBegin(),
                         token.getIdentifierInfo()->getName());
        }
    }

    if (!d_data.d_in_bsl &&
        !d_data.d_in_std &&
        d_analyser.is_component(range.getBegin()) &&
        !d_analyser.manager().isInSystemHeader(range.getBegin()) &&
        is_named(token, "std") &&
        mi->isObjectLike() &&
        mi->getNumTokens() == 1 &&
        is_named(mi->getReplacementToken(0), "bsl")) {
        SourceLocation loc = d_analyser.manager().getFileLoc(range.getBegin());
        Location l(d_analyser.manager(), loc);
        FileID fid = d_analyser.manager().getFileID(loc);
        llvm::StringRef buf = d_analyser.manager().getBufferData(fid);
        unsigned offset = d_analyser.manager().getFileOffset(range.getBegin());
        static llvm::Regex qname("^std *:: *([_[:alpha:]][_[:alnum:]]*)");
        llvm::SmallVector<llvm::StringRef, 7> matches;
        if (qname.match(buf.substr(offset), &matches)) {
            d_data.d_std_names.insert(std::make_pair(matches[1].str(), loc));
        }
        if (!d_analyser.rewriter().ReplaceText(range, "bsl")) {
            d_analyser.report(loc, check_name, "SB04",
                              "Replacing macro 'std' with 'bsl'");
        }
    }
}

// Ifdef
// Ifndef
void report::operator()(SourceLocation        where,
                        const Token&          token,
                        const MacroDirective *)
{
    clear_guard();

    if (!d_data.d_in_bsl &&
        !d_data.d_in_std &&
        !d_analyser.manager().isInSystemHeader(where) &&
        token.isAnyIdentifier() &&
        token.getIdentifierInfo()->getName().startswith("INCLUDED_")) {
        Location loc(d_analyser.manager(), where);
        set_guard(token.getIdentifierInfo()->getName(), where);
    }
}

// Defined
void report::operator()(const Token&          token,
                        const MacroDirective *md,
                        SourceRange           range)
{
    clear_guard();

    llvm::StringRef tn;

    if (!d_data.d_in_bsl &&
        !d_data.d_in_std &&
        !d_analyser.manager().isInSystemHeader(range.getBegin()) &&
        md &&
        token.isAnyIdentifier() &&
        (tn = token.getIdentifierInfo()->getName()).startswith("INCLUDED_")) {
        Location loc(d_analyser.manager(), range.getBegin());
        FileName fn(loc.file());
        std::string s = "INCLUDED_" + fn.component().upper();
        if (tn != s && tn != s + "_H") {
            set_guard(token.getIdentifierInfo()->getName(), range.getBegin());
        }
    }
}

// SourceRangeSkipped
void report::operator()(SourceRange range)
{
    if (d_data.d_guard.size() > 0 &&
        d_analyser.is_component(range.getBegin()) &&
        !d_analyser.manager().isInSystemHeader(range.getBegin())) {
        llvm::StringRef g = d_data.d_guard.str();
        llvm::Regex r ("ifndef +(" + g.str() + ")[[:space:]]+" +
                       "#include +<(" + g.drop_front(9).lower() +
                       "[.]?h?)>[[:space:]]+(# *define +" + g.str() +
                       "[[:space:]]*)?");
        llvm::StringRef source = d_analyser.get_source(range);
        llvm::SmallVector<llvm::StringRef, 7> matches;
        const std::vector<const file_info *> *pfvi;
        if (r.match(source, &matches)) {
            FileType ft = classify(matches[2], &pfvi);
            if (ft == e_STD) {
                for (const file_info *fi : *pfvi) {
                    if (d_data.d_guard == fi->std_guard) {
                        std::pair<size_t, size_t> m =
                            mid_match(source, matches[1]);
                        d_analyser.report(
                            range.getBegin().getLocWithOffset(m.first),
                            check_name, "SB02",
                            "Replacing include guard %0 with %1")
                            << fi->std_guard
                            << fi->bsl_guard;
                        d_analyser.rewriter().ReplaceText(
                            getOffsetRange(
                                range, m.first, matches[1].size() - 1),
                            fi->bsl_guard);
                        m = mid_match(source, matches[2]);
                        d_analyser.report(
                            range.getBegin().getLocWithOffset(m.first),
                            check_name, "SB01",
                            "Replacing header <%0> with <%1>")
                            << matches[2]
                            << fi->bsl;
                        d_analyser.rewriter().ReplaceText(
                            getOffsetRange(
                                range, m.first, matches[2].size() - 1),
                            fi->bsl);
                        if (matches[3].size() > 0) {
                            m = mid_match(source, matches[3]);
                            d_analyser.report(
                                range.getBegin().getLocWithOffset(m.first),
                                check_name, "SB03",
                                "Removing include guard definition of %0")
                                << d_data.d_guard;
                            Rewriter::RewriteOptions ro;
                            ro.RemoveLineIfEmpty = true;
                            d_analyser.rewriter().RemoveText(
                                getOffsetRange(
                                    range, m.first, matches[3].size() - 1),
                                ro);
                        }
                    }
                }
            }
            else if (ft == e_BSL) {
                FileID fid = d_analyser.manager().getFileID(range.getBegin());
                d_data.d_includes[fid].push_back(std::make_pair(
                    matches[2],
                    d_analyser.get_line_range(range.getBegin()).getBegin()));
            }
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

// Else
void report::operator()(SourceLocation where, SourceLocation ifloc)
{
    clear_guard();
}

void report::add_include(FileID fid, const std::string& name)
{
    SourceLocation sl = d_analyser.manager().getLocForStartOfFile(fid);
    Location loc(d_analyser.manager(), sl);
    const std::vector<const file_info *> *pfvi_name;
    classify(name, &pfvi_name);
    std::string guard;
    if (d_analyser.is_component_header(loc.file())) {
        for (const file_info *fi_name : *pfvi_name) {
            guard = fi_name->bsl_guard;
        }
        if (!guard.size()) {
            guard = llvm::StringRef("INCLUDED_" +
                                    name.substr(0, name.rfind("."))).upper();
        }

    }
    SourceLocation ip = sl;
    SourceLocation last_ip = sl;
    for (const auto& p : d_data.d_includes[fid]) {
        const std::vector<const file_info *> *pfvi_inc;
        classify(p.first, &pfvi_inc);
        if (pfvi_name == pfvi_inc) {
            return;
        }
        llvm::StringRef inc =
            pfvi_inc->size() ? pfvi_inc->front()->bsl : p.first;
        if (ip == sl &&
            last_ip != sl &&
            !inc.endswith("_version.h") &&
            !inc.endswith("_ident.h") &&
            (pfvi_inc->size() && pfvi_name->size() ?
                 pfvi_inc->front()->bsl > pfvi_name->front()->bsl :
                 inc > name)) {
            ip = p.second;
        }
        last_ip = p.second;
    }
    ip = d_analyser.get_line_range(ip == sl ? last_ip : ip).getBegin();
    if (d_analyser.is_component(ip) &&
        !d_analyser.manager().isInSystemHeader(ip)) {
        d_analyser.report(ip, check_name, "IS02",
                          "Inserting #include <%0>")
            << name;
        if (guard.size()) {
            d_analyser.rewriter().InsertTextAfter(
                ip, "#ifndef " + guard + "\n");
        }
        d_analyser.rewriter().InsertTextAfter(ip, "#include <" + name + ">\n");
        if (guard.size()) {
            d_analyser.rewriter().InsertTextAfter(ip, "#endif\n");
        }
    }
}

Decl *report::look_through_typedef(Decl *ds)
{
    TypedefDecl *td;
    CXXRecordDecl *rd;
    if ((td = llvm::dyn_cast<TypedefDecl>(ds)) &&
        (rd = td->getUnderlyingType().getTypePtr()->getAsCXXRecordDecl()) &&
        rd->hasDefinition()) {
        return rd->getDefinition();
    }
    return 0;
}

void report::require_file(std::string     name,
                          FileID          fid,
                          SourceLocation  sl,
                          llvm::StringRef symbol)
{
    FileName fn(name);
    name = fn.name();

    const std::vector<const file_info *> *pfvi;
    FileType ft = classify(name, &pfvi);
    if (ft == e_STD) {
        for (const file_info *fi : *pfvi) {
            name = fi->bsl;
        }
    }

    // Check if already included.
    for (const auto& p : d_data.d_includes[fid]) {
        if (p.first == name) {
            return;                                                   // RETURN
        }
    }

    if (ft == e_STD || ft == e_BSL) {
        if (!d_data.d_once[fid].count(name)) {
            d_data.d_once[fid].insert(name);
            d_analyser.report(sl, check_name, "IS01",
                            "Need #include <%0> for symbol %1")
                << name
                << symbol;
        }
    }
}

void report::inc_for_std_decl(llvm::StringRef r,
                              FileID fid,
                              SourceLocation sl,
                              Decl *ds)
{
    Location loc(d_analyser.manager(), ds->getLocation());
    FileName fn(loc.file());
    if (d_data.d_file_map.count(loc.file())) {
        std::string name;
        Decl::redecl_iterator rb = ds->redecls_begin();
        Decl::redecl_iterator re = ds->redecls_end();
        for (; rb != re; ++rb) {
            if (d_analyser.manager().isBeforeInTranslationUnit(
                    rb->getLocation(), sl)) {
                Location loc(d_analyser.manager(), rb->getLocation());
                require_file(loc.file(), fid, sl, r);
            }
        }
    }
}

bool isNamespace(const DeclContext *dc, llvm::StringRef ns)
{
    for (;;) {
        if (!dc->isNamespace()) {
            return false;
        }
        const NamespaceDecl *nd = llvm::cast<NamespaceDecl>(dc);
        if (nd->isInline()) {
            dc = nd->getParent();
        } else if (!dc->getParent()->getRedeclContext()->isTranslationUnit()) {
            return false;
        } else {
            const IdentifierInfo *ii = nd->getIdentifier();
            return ii && ii->getName() == ns;
        }
    }
}

// TranslationUnitDone
void report::operator()()
{
    // ERRS(); d_analyser.context()->getTranslationUnitDecl()->dump(); ERNL();
    MatchFinder mf;
    OnMatch<> m1([&](const BoundNodes &nodes) {
        const DeclRefExpr *expr = nodes.getNodeAs<DeclRefExpr>("dr");
        SourceLocation sl = expr->getExprLoc();
        if (d_analyser.is_component(sl) &&
            !d_analyser.manager().isInSystemHeader(sl)) {
            const DeclContext *dc = expr->getDecl()->getDeclContext();
            std::string name = expr->getNameInfo().getName().getAsString();
            while (dc->isRecord()) {
                name = llvm::dyn_cast<NamedDecl>(dc)->getNameAsString();
                dc = dc->getParent();
            }
            if (isNamespace(dc, "std")) {
                d_data.d_std_names.insert(std::make_pair(name, sl));
            }
        }
    });
    OnMatch<> m2([&](const BoundNodes &nodes) {
        const CXXConstructExpr *expr = nodes.getNodeAs<CXXConstructExpr>("ce");
        SourceLocation sl = expr->getExprLoc();

        if (d_analyser.is_component(sl) &&
            !d_analyser.manager().isInSystemHeader(sl)) {
            const DeclContext *dc =
                expr->getConstructor()->getParent()->getDeclContext();
            std::string name =
                expr->getConstructor()->getParent()->getNameAsString();
            while (dc->isRecord()) {
                name = llvm::dyn_cast<NamedDecl>(dc)->getNameAsString();
                dc = dc->getParent();
            }
            if (isNamespace(dc, "std")) {
                d_data.d_std_names.insert(std::make_pair(name, sl));
            }
        }
    });
    OnMatch<> m3([&](const BoundNodes &nodes) {
        TypeLoc tl = *nodes.getNodeAs<TypeLoc>("et");
        SourceLocation sl = tl.getBeginLoc();
        Location l(d_analyser.manager(), sl);
        std::string r;
        if (d_analyser.is_component(sl) &&
            llvm::StringRef(r = QualType(tl.getTypePtr(), 0).getAsString())
                .startswith("bsl::")) {
            r = r.substr(0, r.find("<"));
            if (Decl *ds = d_analyser.lookup_name(r)) {
                FileID fid = d_analyser.manager().getFileID(sl);
                inc_for_std_decl(r, fid, sl, ds);
                if ((ds = look_through_typedef(ds)) != 0) {
                    inc_for_std_decl(r, fid, sl, ds);
                }
            }
        }
    });
    mf.addDynamicMatcher(
        decl(forEachDescendant(declRefExpr().bind("dr"))), &m1);
    mf.addDynamicMatcher(
        decl(forEachDescendant(constructExpr().bind("ce"))), &m2);
    mf.addDynamicMatcher(
        decl(forEachDescendant(typeLoc(anything()).bind("et"))), &m3);
    mf.match(*d_analyser.context()->getTranslationUnitDecl(),
             *d_analyser.context());

    for (const auto& rp : d_data.d_std_names) {
        llvm::StringRef r = rp.first;
        SourceLocation sl = rp.second;
        FileID fid = d_analyser.manager().getFileID(sl);

        if (!d_analyser.is_component(sl) ||
            d_analyser.manager().isInSystemHeader(sl)) {
            continue;
        }

        Decl *ds = d_analyser.lookup_name(("std::" + r).str());
        Decl *dg = d_analyser.lookup_name(r.str());
        DeclContext *dgc = dg ? dg->getDeclContext() : nullptr;
        // ERRS() << (ds ? "Found " : "Did not find ") << "std::" << r; ERNL();
        // ERRS() << (dg ? "Found " : "Did not find ") << "   ::" << r; ERNL();
        if (ds) {
            if (dgc &&
                (dgc->isTranslationUnit() ||
                 dgc->isExternCContext() ||
                 dgc->isExternCXXContext()) &&
                llvm::dyn_cast<UsingDecl>(ds) &&
                !sl.isMacroID() &&
                d_analyser.get_source(SourceRange(sl, sl.getLocWithOffset(5)),
                                      false) == "std::") {
                d_analyser.report(sl, check_name, "SB05", "Removing namespace "
                                                          "qualification");
                d_analyser.rewriter().ReplaceText(sl, 3, "   ");
            }
            inc_for_std_decl(r, fid, sl, ds);
            if ((ds = look_through_typedef(ds)) != 0) {
                inc_for_std_decl(r, fid, sl, ds);
            }
        }
    }

    for (const auto& fp : d_data.d_once) {
        for (const auto& name : fp.second) {
            add_include(fp.first, name);
        }
    }
}

void subscribe(Analyser& analyser, Visitor&, PPObserver& observer)
    // Hook up the callback functions.
{
    data &d = analyser.attachment<data>();
    for (const auto& f : includes) {
        d.d_file_info[f.std].push_back(&f);
        d.d_file_info[f.bsl].push_back(&f);
    }

    observer.onPPInclusionDirective += report(analyser);
    observer.onPPFileChanged        += report(analyser);
    observer.onPPMacroDefined       += report(analyser,
                                                      observer.e_MacroDefined);
    observer.onPPMacroUndefined     += report(analyser,
                                                    observer.e_MacroUndefined);
    observer.onPPMacroExpands       += report(analyser);
    observer.onPPIfdef              += report(analyser, observer.e_Ifdef);
    observer.onPPIfndef             += report(analyser, observer.e_Ifndef);
    observer.onPPDefined            += report(analyser);
    observer.onPPSourceRangeSkipped += report(analyser);
    observer.onPPIf                 += report(analyser);
    observer.onPPElif               += report(analyser);
    observer.onPPElse               += report(analyser);
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
