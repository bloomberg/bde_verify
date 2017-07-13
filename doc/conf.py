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

version           = '1.2.15'
release           = '1.2.15'
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
## Licensed under the Apache License, Version 2.0 (the "License");
## you may not use this file except in compliance with the License.
## You may obtain a copy of the License at
##
##     http://www.apache.org/licenses/LICENSE-2.0
##
## Unless required by applicable law or agreed to in writing, software
## distributed under the License is distributed on an "AS IS" BASIS,
## WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
## See the License for the specific language governing permissions and
## limitations under the License.
## ----------------------------- END-OF-FILE ----------------------------------
