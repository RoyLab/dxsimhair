using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using HairEngine;

public class MeshGridCreationController : MonoBehaviour {

	// Use this for initialization
	void Start () {
	}

    public void Create() {
        Debug.Log("Begin creation");
        var mesh = GetComponent<MeshFilter>().mesh;
        if (mesh != null)
        {
            Func.InitCollisionObject(mesh);
        }
        Debug.Log("End creation");
    }
	
	// Update is called once per frame
	void Update () {
		
	}
}
