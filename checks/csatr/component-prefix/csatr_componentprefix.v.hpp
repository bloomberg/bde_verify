// csatr_componentprefix.v.hpp                                        -*-C++-*-

#ifndef INCLUDED_CSATR_COMPONENTPREFIX
#define INCLUDED_CSATR_COMPONENTPREFIX 1

#if !defined(INCLUDED_BDES_IDENT)
#  include <bdes_ident.h>
#endif
#ifndef INCLUDED_CSASCM_VERSION
#  include <csascm_version.h>
#endif

// ----------------------------------------------------------------------------

namespace bde_verify
{
    namespace csatr
    {
        enum BadEnum
        {
            badBadEnumTag
        };

        typedef BadEnum BadTypedef;

        struct BadStruct
        {
            enum GoodEnum
            {
                goodGoodEnumTag
            };
            int member;
            int method();
        };

        typedef struct BadStruct1
        {
            int member;
            int method();
        } BadStruct2;

        typedef struct
        {
            int member;
            int method();
        } BadStruct3;

        class BadClass
        {
        public:
            int member;
            int method();
        };

        template <class>
        class BadTemplate
        {
        public:
            int member;
            int method();
        };

        template <class>
        int badTemplate()
        {
            int goodBadTemplateVariable;
            return goodBadTemplateVariable;
        }

        int badFunction()
        {
            int goodBadFunctionVariable = badTemplate<int>();
            return goodBadFunctionVariable;
        }

        int badVariable(0);

        enum ComponentPrefixGoodEnum
        {
            badGoodEnumTag,
            componentPrefixGoodEnumTag
        };

        typedef BadEnum ComponentPrefixGoodTypedef;

        struct ComponentPrefixGoodStruct
        {
            enum GoodEnum
            {
                goodGoodEnumTag
            };
            int member;
            int method();
        };

        class ComponentPrefixGoodClass
        {
        public:
            int member;
            int method();
        };

        void swap(ComponentPrefixGoodClass&, ComponentPrefixGoodClass&);

        template <class>
        class ComponentPrefixGoodTemplate
        {
        public:
            int member;
            int method();
        };

        template <class T>
        void swap(ComponentPrefixGoodTemplate<T>&,
                  ComponentPrefixGoodTemplate<T>&);

        template <class>
        int componentPrefixGoodTemplate()
        {
            int goodGoodTemplateVariable;
            return goodGoodTemplateVariable;
        }

        int componentPrefixGoodFunction()
        {
            int goodGoodFunctionVariable = componentPrefixGoodTemplate<int>();
            return goodGoodFunctionVariable;
        }

        int componentPrefixGoodVariable(0);

        class GoodDeclaration;
    }
}

// ----------------------------------------------------------------------------

inline void
bde_verify::csatr::swap(bde_verify::csatr::ComponentPrefixGoodClass&,
                  bde_verify::csatr::ComponentPrefixGoodClass&)
{
    extern int someVariable;
}

// ----------------------------------------------------------------------------

namespace bde_verify
{
    namespace csatr
    {
        enum class BadEnumClass
        {
            badBadEnumClassTag
        };
    }

    enum class ComponentPrefixGoodEnumClass
    {
        badGoodEnumClassTag,
        componentPrefixGoodEnumClassTag
    };
}

#endif

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
