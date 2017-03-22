using System;
using System.Runtime.InteropServices;

namespace HairEngine
{
    [StructLayout(LayoutKind.Sequential)]
    class CollisionParameter
    {
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = Constants.MAX_PATH_LENGTH)]
        string collisionfile;

        float correctionTolerance;
        float correctionRate;
        float maxStep;

        public CollisionParameter(string collisionfile, float correctionTolerance, float correctionRate, float maxStep) {
            this.collisionfile = collisionfile;
            this.correctionTolerance = correctionTolerance;
            this.correctionRate = correctionRate;
            this.maxStep = maxStep;
        }

        public IntPtr ToIntPtr() {
            IntPtr pointer = Marshal.AllocHGlobal(Marshal.SizeOf(this));
            Marshal.StructureToPtr(this, pointer, true);
            return pointer;
        }
    }
}
