// elseif.cpp                                                         -*-C++-*-

int f()
{
    if (1) {
        return 0;
    } else if (2) {
        return 1;
    } else switch (1) {
      case 1:
        return 2;
    }
    if (1) {
        return 0;
    } else do {
        return 1;
    } while (0);
    for (;;) {
        if (1) {
            return 0;
        } else break;
    }
    if (1) {
        return 0;
    } else for (;;) {
        return 1;
    }
    if (1) {
        return 0;
    } else {
        return 1;
    }
}

int g()
{
    if (1) {
        return 0;
    }
    else if (2)
        switch (1) {
          case 1:
            return 2;
        }
    if (1) {
        return 0;
    }
    else if (2)
        do {
            return 1;
        } while (0);
    for (;;) {
        if (1) {
            return 0;
        }
        else if (2)
            break;
    }
    for (;;) {
        if (1) {
            return 0;
        }
        else if (2)
            continue;
    }
    if (1) {
        return 0;
    }
    else if (2)
        for (;;) {
            return 1;
        }
    if (1) {
        return 0;
    } else {
        return 1;
    }
    if (1)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

// ----------------------------------------------------------------------------
// Copyright (C) 2018 Bloomberg Finance L.P.
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
