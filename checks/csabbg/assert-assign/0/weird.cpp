// This code caused a crash in bde_verify due to assert-assign calling AST
// matchers from a UnaryExpression callback instead of waiting to do it from
// the end of translation unit.

struct L { ~L(); };
template <int> struct B { L l; B() : l(L()) { } };
struct b : B<0> { b() { } };
void m(int o) { (void)!(o = 1); }

// ----------------------------------------------------------------------------
// Copyright (C) 2015 Bloomberg Finance L.P.
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
