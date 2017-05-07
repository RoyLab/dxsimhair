using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class DebugPrintWorld2LocalMatrix : MonoBehaviour {

	// Use this for initialization
	void Start () {
		
	}
	
	// Update is called once per frame
	void Update () {
        var mat = this.transform.worldToLocalMatrix;
        Debug.Log(mat.ToString());
	}
}
