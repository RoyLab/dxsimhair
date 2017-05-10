using System.Collections;
using System.Collections.Generic;
using System.IO;
using UnityEngine;
using UnityEngine.UI;

public class GetStartController : MonoBehaviour {

    public GameObject hairObject;
    public InputField inputField;
    public Text textField;

    public void OnControl() {
        FileSelectedCallback(inputField.text);
    }

    protected void FileSelectedCallback(string path) {
        var hairController = hairObject.GetComponent<DllImportTestController>();

        string replacedPath = path.Replace("/", "\\");
        if (File.Exists(replacedPath) == false) {
            textField.text = "File not exists";
            return;
        }
        hairController.configurationFilePath = path.Replace("/", "\\");
        hairController.GetStart();

        GameObject.Find("Canvas").SetActive(false);
    }

    protected void OnGUI()
    {
    }

    // Use this for initialization
    void Start () {

	}
	
	// Update is called once per frame
	void Update () {
		
	}
}
