using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class HairController : MonoBehaviour {
    // Use this for initialization
    public Material material;

	void Start () {
        LoadHair("straight");
        LoadHair("curly");
        LoadHair("short");
        LoadHair("wavy");
	}

    void LoadHair(string name) {
        var gameObjectList = HairLoader.LoadStaticHair(name);
        var gameObjectParent = new GameObject();
        foreach (var gameObject in gameObjectList)
        {
            gameObject.transform.parent = gameObjectParent.transform;
            gameObject.GetComponent<MeshRenderer>().sharedMaterial = material;
        }
    }
	
	// Update is called once per frame
	void Update () {
	}
}
