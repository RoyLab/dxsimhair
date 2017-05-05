using System;
using System.Runtime.InteropServices;
using UnityEngine;

namespace HairEngine
{
    class Func
    {
        [DllImport("SimHair00023", EntryPoint = "InitializeHairEngine")]
        private static extern int _InitializeHairEngine(IntPtr HairParameterPtr, IntPtr CollisionParameterPtr, IntPtr SkinningParameterPtr, IntPtr PdbParameterPtr);

        public static int InitializeHairEngine(HairParameter hair, CollisionParameter col, SkinningParameter skin, PbdParameter pbd) {
            var ret = _InitializeHairEngine(hair.ToIntPtr(), col.ToIntPtr(), skin.ToIntPtr(), pbd.ToIntPtr());
            return ret;
        }

        [DllImport("SimHair00023", EntryPoint = "UpdateParameter", CharSet = CharSet.Ansi)]
        private static extern int _UpdateParameter(
            [MarshalAs(UnmanagedType.LPStr)]
            string key,
            [MarshalAs(UnmanagedType.LPStr)]
            string value
            );

        public static int UpdateParameter(string key, string value) {
            return _UpdateParameter(key, value);
        }

        [DllImport("SimHair00023", EntryPoint = "UpdateHairEngine")]
        private static extern int _UpdateHairEngine(
            [MarshalAs(UnmanagedType.LPArray, SizeConst = 16)]
            float[] headMatrix,
            [In, Out] Vector3[] particlePositions,
            [In, Out] Vector3[] particleDirections,
            float deltaTime
            );

        //This api will change later
        public static int UpdateHairEngine(float[] headMatrix, Vector3[] particlePositions, Vector3[] particleDirections, float deltaTime) {
            var ret = _UpdateHairEngine(headMatrix, particlePositions, particleDirections, deltaTime);
            return ret;
        }

        [DllImport("SimHair00023", EntryPoint = "ReleaseHairEngine")]
        private static extern void _ReleaseHairEngine();

        public static void ReleaseHairEngine() {
            _ReleaseHairEngine();
        }

        [DllImport("SimHair00023", EntryPoint = "GetHairParticleCount")]
        public static extern int GetHairParticleCount();

        [DllImport("SimHair00023", EntryPoint = "GetParticlePerStrandCount")]
        public static extern int GetParticlePerStrandCount();

        [DllImport("SimHair00023", EntryPoint = "InitCollisionObject")]
        private static extern int _InitCollisionObject(
            int nVertices,
            int nFaces,
            [In] Vector3[] vertices,
            [In] int[] faces
            );

        private static bool hasInitCollisionObject = false;

        public static int InitCollisionObject(Mesh mesh)
        {
            if (hasInitCollisionObject) return -1;
            hasInitCollisionObject = true;

            return _InitCollisionObject(mesh.vertices.Length, mesh.triangles.Length / 3, mesh.vertices, mesh.triangles);
        }

        [DllImport("SimHair00023", EntryPoint = "GetStrandColor")]
        private static extern int _GetStrandColor(
            [In, Out] int[] colorBuffer
            );

        public static bool GetStrandColor(int[] colorBuffer)
        {
            int ret = _GetStrandColor(colorBuffer);
            return (ret >= 0);
        }
    }
}
