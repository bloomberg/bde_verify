// csastil_templatetypename.t.h                                       -*-C++-*-

#if !defined(INCLUDED_CSASTIL_TEMPLATETYPENAME)
#define INCLUDED_CSASTIL_TEMPLATETYPENAME
#if !defined(INCLUDED_BDES_IDENT)
#  include <bdes_ident.h>
#endif
#ifndef INCLUDED_CSASCM_VERSION
#  include <csascm_version.h>
#endif

namespace bde_verify
{
    namespace csastil
    {
        template <typename T>
        struct TemplateTypenameBad
        {
        public:
            template <typename S> TemplateTypenameBad();
            template <typename SS> void foo();
        };

        struct TemplateTypenameMember
        {
        public:
            template <typename SS> TemplateTypenameMember();
            template <typename S> void foo();
        };

        template <typename T>
        void swap(TemplateTypenameBad<T>&, TemplateTypenameBad<T>&);

        template <class t_T>
        void swap2(TemplateTypenameBad<t_T>&, TemplateTypenameBad<t_T>&);

        template <class t_Type>
        void swap3(TemplateTypenameBad<t_Type>&, TemplateTypenameBad<t_Type>&);

        template <class TT>
        class TemplateTypenameGood
        {
        public:
            template <class S> TemplateTypenameGood();
            template <class SS> void foo();
        };

        template <class TT>
        void swap(TemplateTypenameGood<TT>&, TemplateTypenameGood<TT>&);

        template <class t_TT>
        void swap2(TemplateTypenameGood<t_TT>&, TemplateTypenameGood<t_TT>&);
    }
}

template <typename TT>
inline void
bde_verify::csastil::swap(TemplateTypenameBad<TT>&, TemplateTypenameBad<TT>&)
{
}

template <typename t_TT>
inline void
bde_verify::csastil::swap2(TemplateTypenameBad<t_TT>&,
                           TemplateTypenameBad<t_TT>&)
{
}

namespace bde_verify
{
    namespace csastil
    {
        template <typename AA, class BB, typename CC> void foo();
        template <class t_A, class t_Bb, class t_CC> void boo();
    }
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
