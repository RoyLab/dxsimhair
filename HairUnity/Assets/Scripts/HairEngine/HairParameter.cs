using System;
using System.Runtime.InteropServices;


namespace HairEngine
{
    [StructLayout(LayoutKind.Sequential)]
    public class HairParameter {
        [MarshalAs(UnmanagedType.U1)]
        bool hasGuide;

        [MarshalAs(UnmanagedType.U1)]
        bool hasCollision;

        [MarshalAs(UnmanagedType.U1)]
        bool hasPbd;

        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = Constants.MAX_PATH_LENGTH)]
        string hairfilePath;

        public static HairParameter FromIntPtr(IntPtr pointer) {
            return Marshal.PtrToStructure(pointer, typeof(HairParameter)) as HairParameter;
        }

        public IntPtr ToIntPtr() {
            IntPtr pointer = Marshal.AllocHGlobal(Marshal.SizeOf(this));
            Marshal.StructureToPtr(this, pointer, true);
            return pointer;
        } 

        public HairParameter(bool hasGuide, bool hasCollision, bool hasPbd, string hairfilePath) {
            this.hasGuide = hasGuide;
            this.hasCollision = hasCollision;
            this.hasPbd = hasPbd;
            this.hairfilePath = hairfilePath;
        }
    }
}
