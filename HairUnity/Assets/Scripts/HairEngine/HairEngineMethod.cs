using System;
using System.Runtime.InteropServices;
using UnityEngine;

namespace HairEngine
{
    class Func
    {
        [DllImport("SimHair00004", EntryPoint = "InitializeHairEngine")]
        private static extern int _InitializeHairEngine(IntPtr HairParameterPtr, IntPtr CollisionParameterPtr, IntPtr SkinningParameterPtr, IntPtr PdbParameterPtr);

        public static int InitializeHairEngine(HairParameter hair, CollisionParameter col, SkinningParameter skin, PbdParameter pbd) {
            var ret = _InitializeHairEngine(hair.ToIntPtr(), col.ToIntPtr(), skin.ToIntPtr(), pbd.ToIntPtr());
            return ret;
        }

        [DllImport("SimHair00004", EntryPoint = "UpdateParameter", CharSet = CharSet.Ansi)]
        private static extern int _UpdateParameter(
            [MarshalAs(UnmanagedType.LPStr)]
            string key,
            [MarshalAs(UnmanagedType.LPStr)]
            string value
            );

        public static int UpdateParameter(string key, string value) {
            return _UpdateParameter(key, value);
        }

        [DllImport("SimHair00004", EntryPoint = "UpdateHairEngine")]
        private static extern int _UpdateHairEngine(
            [MarshalAs(UnmanagedType.LPArray, SizeConst = 16)]
            float[] headMatrix,
            [In, Out] Vector3[] particlePositions,
            [In, Out] Vector3[] particleDirections
            );

        //This api will change later
        public static int UpdateHairEngine(float[] headMatrix, Vector3[] particlePositions, Vector3[] particleDirections) {
            var ret = _UpdateHairEngine(headMatrix, particlePositions, particleDirections);
            return ret;
        }

        [DllImport("SimHair00004", EntryPoint = "ReleaseHairEngine")]
        private static extern void _ReleaseHairEngine();

        public static void ReleaseHairEngine() {
            _ReleaseHairEngine();
        }

        [DllImport("SimHair00004", EntryPoint = "GetHairParticleCount")]
        public static extern int GetHairParticleCount();

        [DllImport("SimHair00004", EntryPoint = "GetParticlePerStrandCount")]
        public static extern int GetParticlePerStrandCount();
    }
}
