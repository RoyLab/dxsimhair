using System;
using System.Runtime.InteropServices;

namespace HairEngine
{
    [StructLayout(LayoutKind.Sequential)]
    class PbdParameter
    {
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = Constants.MAX_PATH_LENGTH)]
        string groupfile;

        float lambda;
        float chunksize;
        int maxiteration;

        public PbdParameter(string groupfile, float lambda, float chunksize, int maxiteration) {
            this.groupfile = groupfile;
            this.lambda = lambda;
            this.chunksize = chunksize;
            this.maxiteration = maxiteration;
        } 

        public IntPtr ToIntPtr() {
            IntPtr pointer = Marshal.AllocHGlobal(Marshal.SizeOf(this));
            Marshal.StructureToPtr(this, pointer, true);
            return pointer;
        }
    }
}
