using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;

public class GetStartController : MonoBehaviour {

    public GameObject hairObject;
    public InputField filePathInputField;

    public void OnControl() {
        var hairController = hairObject.GetComponent<DllImportTestController>();
        hairController.configurationFilePath = filePathInputField.text;
        hairController.GetStart();

        GameObject.Find("Canvas").SetActive(false);
    }

	// Use this for initialization
	void Start () {

	}
	
	// Update is called once per frame
	void Update () {
		
	}
}
