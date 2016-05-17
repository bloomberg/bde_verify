// spec.cpp                                                           -*-C++-*-

#pragma bde_verify set word_slack 3

void f(int localDate, int offset)
    // Create a 'DateTz' object having a local date value equal to the
    // specified 'localDate' and a time zone offset value from UTC equal to
    // the specified 'offset' (in minutes).  The behavior is undefined
    // unless 'offset' is in the range '( -1440 .. 1440 )'.  Note that this
    // method provides no validation, and it is the user's responsibility
    // to assure the consistency of the resulting value.
{
}

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
