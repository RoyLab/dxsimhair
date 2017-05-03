using System.Collections;
using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using UnityEngine;
using HairEngine;

public class DllImportTestController : MonoBehaviour {

    public Material material;
    public Transform headTransform;

    Vector3[] positions = null, directions = null;
    float[] headMatrix = new float[16]
    {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };

    // Use this for initialization
    void Start () {

        string configDescription = System.IO.File.ReadAllText("C:\\Users\\vivid\\Desktop\\newconfig.ini").Replace("\r\n", "\n");
        HairParameter param = new HairParameter(false, false, false, configDescription);

        CollisionParameter col = new CollisionParameter("", 0.0f, 0.0f, 0.0f); //ignore
        SkinningParameter skin = new SkinningParameter(""); //ignore
        PbdParameter pbd = new PbdParameter("", 0.0f, 0.0f, 0); //ignore

        Func.InitializeHairEngine(param, col, skin, pbd);

        int particleCount = Func.GetHairParticleCount();
        int particlePerStrandCount = Func.GetParticlePerStrandCount();
        int strandCount = particleCount / particlePerStrandCount;
        positions = new Vector3[particleCount];
        directions = new Vector3[particleCount];

        Func.UpdateHairEngine(ApplyTranformToWorldMatrix(), positions, directions, 30.0e-3f);
        foreach (var gameObject in HairLoader.LoadFixedParticlePerStrand(positions, strandCount, particlePerStrandCount))
        {
            gameObject.transform.parent = this.transform;
            gameObject.GetComponent<MeshRenderer>().material = material;
        }

        /* use for testing */
        //HairParameter param = new HairParameter(true, false, true, "hairfilepath");
        //CollisionParameter col = new CollisionParameter("collisionfile", 1.23f, 4.56f, 7.89f);
        //SkinningParameter skin = new SkinningParameter("weightfile");
        //PbdParameter pbd = new PbdParameter("groupfile", 1.34f, 4.67f, 8);
        //Func.InitializeHairEngine(param, col, skin, pbd);

        //Func.UpdateParameter("testkey", "testvalue");

        //float[] rigids = new float[16];
        //for (int i = 0; i < 4; ++i)
        //    for (int j = 0; j < 4; ++j)
        //        rigids[i * 4 + j] = i * i + j * j;

        //Vector3[] vecPositions = new Vector3[1];
        //Vector3[] vecDirections = new Vector3[1];

        //Func.UpdateHairEngine(rigids, vecPositions, vecDirections);

        //int a = Func.GetHairParticleCount();
        //int b = Func.GetParticlePerStrandCount();

        //Func.ReleaseHairEngine();
	}

    float[] ApplyTranformToWorldMatrix() {
        for (int i = 0; i < 4; ++i)
            for (int j = 0; j < 4; ++j)
                headMatrix[i * 4 + j] = (headTransform == null) ? ((i == j) ? 1.0f : 0.0f) : headTransform.localToWorldMatrix[i, j];
        return headMatrix;
    }
	
	// Update is called once per frame
	void Update () {
        if (positions == null || directions == null)
            return;

        var deltaTime = Time.deltaTime;
        Func.UpdateHairEngine(ApplyTranformToWorldMatrix(), positions, directions, 0.03f);

        foreach (var transform in this.transform)
        {
            var gameObject = (transform as Transform).gameObject;
            var mesh = gameObject.GetComponent<MeshFilter>().mesh;
            var vertices = mesh.vertices;
            var positionOffset = gameObject.GetComponent<HairIndexer>().particleBeginIndex;
            for (int i = 0; i < vertices.Length; ++i)
                vertices[i] = positions[i + positionOffset];
            mesh.vertices = vertices;
        }
    }
}
