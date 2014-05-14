// csabde-verify_tool.cpp                                                   -*-C++-*-
#include <csabase_tool.h>
#include <csabase_analyse.h>

int main(int argc_, const char **argv_)
{
    return csabase::run(argc_, argv_);
}

// -----------------------------------------------------------------------------

clang::FrontendPluginRegistry::Add<csabase::PluginAction> registerPlugin(
    "bde_verify", "analyse source");
