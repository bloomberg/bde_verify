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
#pragma bde_verify set wrap_slack 1
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
