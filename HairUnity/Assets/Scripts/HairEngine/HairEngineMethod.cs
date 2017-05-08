using System;
using System.Runtime.InteropServices;
using UnityEngine;

namespace HairEngine
{
    class Func
    {
        [DllImport("SimHair00008", EntryPoint = "InitializeHairEngine")]
        private static extern int _InitializeHairEngine(IntPtr HairParameterPtr, IntPtr CollisionParameterPtr, IntPtr SkinningParameterPtr, IntPtr PdbParameterPtr);

        public static int InitializeHairEngine(HairParameter hair, CollisionParameter col, SkinningParameter skin, PbdParameter pbd) {
            var ret = _InitializeHairEngine(hair.ToIntPtr(), col.ToIntPtr(), skin.ToIntPtr(), pbd.ToIntPtr());
            return ret;
        }

        [DllImport("SimHair00008", EntryPoint = "UpdateParameter", CharSet = CharSet.Ansi)]
        private static extern int _UpdateParameter(
            [MarshalAs(UnmanagedType.LPStr)]
            string key,
            [MarshalAs(UnmanagedType.LPStr)]
            string value
            );

        public static int UpdateParameter(string key, string value) {
            return _UpdateParameter(key, value);
        }

        [DllImport("SimHair00008", EntryPoint = "UpdateHairEngine")]
        private static extern int _UpdateHairEngine(
            [MarshalAs(UnmanagedType.LPArray, SizeConst = 16)]
            float[] headMatrix,
            [MarshalAs(UnmanagedType.LPArray, SizeConst = 16)]
            float[] collisionObjWorld2LocalMatrix,
            [In, Out] Vector3[] particlePositions,
            [In, Out] Vector3[] particleDirections,
            float deltaTime
            );

        private static float[] GetTransformMatrixArray(Matrix4x4 mat)
        {
            float[] ret = new float[16];
            for (int i = 0; i < 4; ++i)
                for (int j = 0; j < 4; ++j)
                    ret[i * 4 + j] = mat[i, j];
            return ret;
        }

        //This api will change later
        public static int UpdateHairEngine(Transform headRigidTransform, Transform colliderTransform, Vector3[] particlePositions, Vector3[] particleDirections, float deltaTime) {

            var headMatrix = GetTransformMatrixArray(headRigidTransform.localToWorldMatrix);
            var collisionWorld2LocalMatrix = GetTransformMatrixArray(colliderTransform.worldToLocalMatrix);

            var ret = _UpdateHairEngine(headMatrix, collisionWorld2LocalMatrix, particlePositions, particleDirections, deltaTime);
            return ret;
        }

        [DllImport("SimHair00008", EntryPoint = "ReleaseHairEngine")]
        private static extern void _ReleaseHairEngine();

        public static void ReleaseHairEngine() {
            _ReleaseHairEngine();
        }

        [DllImport("SimHair00008", EntryPoint = "GetHairParticleCount")]
        public static extern int GetHairParticleCount();

        [DllImport("SimHair00008", EntryPoint = "GetParticlePerStrandCount")]
        public static extern int GetParticlePerStrandCount();

        [DllImport("SimHair00008", EntryPoint = "InitCollisionObject")]
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

        [DllImport("SimHair00008", EntryPoint = "GetStrandColor")]
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
