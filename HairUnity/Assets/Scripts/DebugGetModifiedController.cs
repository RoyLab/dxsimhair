using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class DebugGetModifiedController : MonoBehaviour {

	// Use this for initialization
	void Start () {
        string[] lines = System.IO.File.ReadAllLines(@"C:\Users\vivid\Desktop\MeshInfoModified.txt");
        char[] sep = new char[1] { ' ' };

        int nvertices = int.Parse(lines[0]);
        Vector3[] vertices = new Vector3[nvertices];
        for (int i = 0; i < vertices.Length; ++i) {
            var elements = lines[1 + i].Split(sep);
            vertices[i] = new Vector3(float.Parse(elements[0]), float.Parse(elements[1]), float.Parse(elements[2]));
        }

        int nfaces = int.Parse(lines[nvertices + 1]);
        int[] faces = new int[nfaces * 3];
        for (int i = 0; i < nfaces; ++i) {
            var elements = lines[2 + nvertices + i].Split(sep);
            faces[3 * i] = int.Parse(elements[0]);
            faces[3 * i + 1] = int.Parse(elements[1]);
            faces[3 * i + 2] = int.Parse(elements[2]);
        }

        var mesh = this.GetComponent<MeshFilter>().mesh;
        //mesh.vertices = new Vector3[] { new Vector3(1, 0, 0), new Vector3(0, 0, 0), new Vector3(0, 1, 0) };
        //mesh.triangles = new int[] { 0, 1, 2 };
        mesh.vertices = vertices;
        mesh.triangles = faces;
    }
	
	// Update is called once per frame
	void Update () {
		
	}
}
