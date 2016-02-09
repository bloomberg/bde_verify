// Guids.cs
// MUST match guids.h
using System;

namespace BloombergLP.BDE_VERIFY_VS
{
    static class GuidList
    {
        public const string guidBDE_VERIFY_VSPkgString = "69a97314-26d2-4daa-9cd8-39ca54016999";
        public const string guidBDE_VERIFY_VSCmdSetString = "5a7e0cdd-d1ea-40ae-95ef-b33ced83f4e5";
        public const string guidToolWindowPersistanceString = "ea43e952-1542-4ba3-9eec-6e00e0e67113";

        public static readonly Guid guidBDE_VERIFY_VSCmdSet = new Guid(guidBDE_VERIFY_VSCmdSetString);
    };
}