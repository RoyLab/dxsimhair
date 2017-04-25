using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using HairEngine;

public class FullModelParameterController : MonoBehaviour {

    [Range(1.0f, 50.0f)]
    public float k1 = 5.0f;
    public int k1Exp = -5;

    [Range(1.0f, 50.0f)]
    public float k2 = 5.0f;
    public int k2Exp = -5;

    [Range(1.0f, 50.0f)]
    public float k3 = 5.0f;
    public int k3Exp = -5;

    [Range(1.0f, 50.0f)]
    public float mass = 5.0f;
    public int massExp = -7;

    [Range(1.0f, 50.0f)]
    public float damping = 1.0f;
    public int dampingExp = -4;

    [Range(1.0f, 50.0f)]
    public float wind = 5.0f;
    public int windExp = -6;

    public bool collision = false;
    public bool strainlimit = false;

	// Use this for initialization
	void Start () {
		
	}
	
	// Update is called once per frame
	void Update () {
        //Func.UpdateParameter("full_spring1", GetFloatValue(k1, k1Exp));
        //Func.UpdateParameter("full_spring2", GetFloatValue(k2, k2Exp));
        //Func.UpdateParameter("full_spring3", GetFloatValue(k3, k3Exp));
        //Func.UpdateParameter("full_particlemass", GetFloatValue(mass, massExp));
        //Func.UpdateParameter("full_springdamping", GetFloatValue(damping, dampingExp));
        //Func.UpdateParameter("full_collision", (collision ? 1 : 0).ToString());
        //Func.UpdateParameter("full_strainlimit", (strainlimit ? 1 : 0).ToString());
 	}

    string GetFloatValue(float value, int exp)
    {
        return value + "e" + exp;
    }
}
