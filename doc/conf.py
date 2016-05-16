import sys
import os

if tags.has('bb_cppverify'):
    project       = u'bb_cppverify'
    rst_prolog    = """
    .. |BV| replace:: ``BB_CPPVERIFY``
    .. |Bv| replace:: ``Bb_cppverify``
    .. |bv| replace:: ``bb_cppverify``
    .. |BDE| replace:: BloombergLP C++
"""
elif tags.has('bde_verify'):
    project       = u'bde_verify'
    rst_prolog    = """
    .. |BV| replace:: ``BDE_VERIFY``
    .. |Bv| replace:: ``Bde_verify``
    .. |bv| replace:: ``bde_verify``
    .. |BDE| replace:: BDE
"""
else:
    sys.exit("Specify -t bb_cppverify or -t bde_verify in SPHINXOPTS.")

version           = '1.2.3'
release           = '1.2.3'
author            = u'Hyman Rosen'
copyright         = u'2015, %s' % author

master_doc        = 'index'
source_suffix     = '.rst'

extensions        = []
templates_path    = []
exclude_patterns  = ['*_build']

pygments_style    = 'sphinx'
html_theme        = 'default'
html_static_path  = []
htmlhelp_basename = '%sdoc' %project

latex_elements    = {}
latex_documents   = [(
    master_doc,
    '%s.tex' % project,
    u'%s Documentation' % project,
    author,
    'manual',
)]

man_pages         = [(
    master_doc,
    '%s' % project,
    u'%s Documentation' % project,
    [author],
    1,
)]

texinfo_documents = [(
    master_doc,
    '%s' % project,
    u'%s Documentation' % project,
    author,
    '%s' % project,
    'One line description of project.',
    'Miscellaneous',
)]

## ----------------------------------------------------------------------------
## Copyright (C) 2015 Bloomberg Finance L.P.
##
## Permission is hereby granted, free of charge, to any person obtaining a copy
## of this software and associated documentation files (the "Software"), to
## deal in the Software without restriction, including without limitation the
## rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
## sell copies of the Software, and to permit persons to whom the Software is
## furnished to do so, subject to the following conditions:
##
## The above copyright notice and this permission notice shall be included in
## all copies or substantial portions of the Software.
##
## THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
## IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
## FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
## AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
## LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
## FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
## IN THE SOFTWARE.
## ----------------------------- END-OF-FILE ----------------------------------
