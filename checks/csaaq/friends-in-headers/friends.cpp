#include <friends.h>

void c_function_declared_in_cpp();
void c_function_defined_in_cpp() { }

class c_class_declared_in_cpp;
class c_class_definedd_in_cpp { };

template <class T> void c_function_template_declared_in_cpp(T);
template <class T> void c_function_template_defined_in_cpp(T) { }

template <class T> class c_class_template_declared_in_cpp;
template <class T> class c_class_template_defined_in_cpp { };

void u_function_declared_in_cpp();
void u_function_defined_in_cpp() { }

class u_class_declared_in_cpp;
class u_class_definedd_in_cpp { };

template <class T> void u_function_template_declared_in_cpp(T);
template <class T> void u_function_template_defined_in_cpp(T) { }

template <class T> class u_class_template_declared_in_cpp;
template <class T> class u_class_template_defined_in_cpp { };

void i_function_declared_in_cpp();
void i_function_defined_in_cpp() { }

class i_class_declared_in_cpp;
class i_class_definedd_in_cpp { };

template <class T> void i_function_template_declared_in_cpp(T);
template <class T> void i_function_template_defined_in_cpp(T) { }

template <class T> class i_class_template_declared_in_cpp;
template <class T> class i_class_template_defined_in_cpp { };

template class i<void>;
template class i<int>;

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
