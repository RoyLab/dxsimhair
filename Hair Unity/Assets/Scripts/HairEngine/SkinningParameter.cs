using System;
using System.Runtime.InteropServices;

namespace HairEngine
{
    [StructLayout(LayoutKind.Sequential)]
    class SkinningParameter
    {
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = Constants.MAX_PATH_LENGTH)]
        string weightfile;

        public SkinningParameter(string weightfile) {
            this.weightfile = weightfile;
        }

        public IntPtr ToIntPtr() {
            IntPtr pointer = Marshal.AllocHGlobal(Marshal.SizeOf(this));
            Marshal.StructureToPtr(this, pointer, true);
            return pointer;
        }
    }
}
