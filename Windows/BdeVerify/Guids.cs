// Guids.cs
// MUST match guids.h
using System;

namespace BloombergLP.BdeVerify
{
    static class GuidList
    {
        public const string guidBdeVerifyPkgString = "69a97314-26d2-4daa-9cd8-39ca54016999";
        public const string guidBdeVerifyCmdSetString = "5a7e0cdd-d1ea-40ae-95ef-b33ced83f4e5";
        public const string guidToolWindowPersistanceString = "ea43e952-1542-4ba3-9eec-6e00e0e67113";

        public static readonly Guid guidBdeVerifyCmdSet = new Guid(guidBdeVerifyCmdSetString);
    };
}
