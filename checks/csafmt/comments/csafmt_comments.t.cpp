// csafmt_comments.t.cpp                                              -*-C++-*-

// @DESCRIPTION: this is a fully value-semantic type, don't you know.

// @DESCRIPTION: this is just a value-semantic type.

//     ( Base_Class )
//           |
//           |   Show inheritance relationship
//           |
//           V
//    ( Derived_Class )
//           |
//           |   Show inheritance relationship
//           |
//           V
// ( Derived_Derived_Class )

//        ( Base_Class )
//              |
//             |   Show inheritance relationship
//            |
//           V
//    ( Derived_Class )

//     ,-----------.
//    (  Base_Class )
//     `-----------'
//           |
//           |   Show inheritance relationship
//           |
//           V
//    ,--------------.
//   (  Derived_Class )
//    `--------------'
//           |
//           |   Show inheritance relationship
//           |
//           V
//  ,----------------------.
// (  Derived_Derived_Class )
//  `----------------------'

        // ====================
        // This can be
        // wrapped.
        // ====================

        // ========================
        // This cannnot be.
        // Wrapped
        // ========================

// Hello,
// world! This can wrap.

// This can't wrap.                                                   Hello
// world! This can't wrap. 

// This can wrap.  S'Ok!                                         Hello
// 'world!' This can wrap.

// This can't wrap.                                                 Hello
// world! This can't wrap. 

// This can wrap.  S'Ok!                                       Hello.
// "world!" This can wrap.

// This can't wrap.                                               Hello.
// world! This can't wrap. 

//@PURPOSE: None. Why even go on?
//@PURPOSE: Lots! Tell you later.
// @ Purpose :Too much space, too many dots...

// Pure procedure? My god! It's full of pure procedures!

#pragma bde_verify push
// BDE_VERIFY pragma: set wrap_slack 1
// ============================================================================
// y                                                                        x
// a b c d
// ============================================================================
#pragma bde_verify pop

//@CLASSES:
//    a::b : first class
//    c_d: not so first class
//
//@DESCRIPTION: flak

// This is a modifiable reference, while this other one contains non-modifiable
// references.

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
