using System.Collections;
using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using UnityEngine;
using HairEngine;

public class DllImportTestController : MonoBehaviour {

    public Material material;

    Vector3[] positions, directions;
    float[] headMatrix = new float[16];
    bool hasLoad = false;

    // Use this for initialization
    void Start () {
        HairParameter hair = new HairParameter(false, false, false, "C:\\Users\\vivid\\Desktop\\Test\\0425.anim2");
        CollisionParameter col = new CollisionParameter("collisionfiletest", 1.234f, 3.456f, 5.789f);
        SkinningParameter skin = new SkinningParameter("weightfiletest");
        PbdParameter pbd = new PbdParameter("groupfiletest", 1.234f, 3.456f, 7890);
        Debug.Log(Func.InitializeHairEngine(hair, col, skin, pbd));

        var particleCount = Func.GetHairParticleCount();
        var particlePerStrandCount = Func.GetParticlePerStrandCount();
        var strandCount = particleCount / particlePerStrandCount;
        var particlePerStrand = new int[strandCount];
        for (int i = 0; i < strandCount; ++i)
            particlePerStrand[i] = particlePerStrandCount;

        positions = new Vector3[particleCount];
        directions = new Vector3[particleCount];

        Func.UpdateHairEngine(headMatrix, positions, directions);
        foreach (var gameObject in HairLoader.Load(positions, particlePerStrand)) {
            gameObject.transform.parent = this.transform;
            gameObject.GetComponent<MeshRenderer>().material = material;
        }

        hasLoad = true;
        Debug.Log("Success");
	}
	
	// Update is called once per frame
	void Update () {
        if (hasLoad == false)
            return;

        Func.UpdateHairEngine(headMatrix, positions, directions);
        foreach (var transform in this.transform) {
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
