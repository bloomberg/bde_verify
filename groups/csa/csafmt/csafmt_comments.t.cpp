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
