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

        template <class TT>
        class TemplateTypenameGood
        {
        public:
            template <class S> TemplateTypenameGood();
            template <class SS> void foo();
        };

        template <class TT>
        void swap(TemplateTypenameGood<TT>&, TemplateTypenameGood<TT>&);
    }
}

template <typename TT>
inline void
bde_verify::csastil::swap(TemplateTypenameBad<TT>&, TemplateTypenameBad<TT>&)
{
}

#endif
