// SimHairDllTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "DllExports.h"
#include <string>
#include <fstream>
#include <iostream>
#include <vector>
#include <sstream>
using namespace std;

using namespace XRwy;
using namespace std;

void apply_param_config(HairParameter &param, const string &conf) {
	for (int i = 0; i < conf.size(); ++i)
		param.root[i] = conf[i];
	param.root[conf.size()] = 0;
}

int main()
{
	//ifstream fin;
	//fin.open("C:\\Users\\vivid\\Desktop\\InitCollisionObject.txt", ios::in);
	//vector<float> vertices;
	//vector<int> faces;
	//string line;
	//bool read_faces = false;
	//while (fin) {
	//	getline(fin, line);
	//	if (line[0] == 'v');
	//	else if (line[0] == 'f')
	//		read_faces = true;
	//	else if (line.size() == 0)
	//		break;
	//	else {
	//		istringstream sin(line);
	//		if (read_faces == false) {
	//			float a, b, c; 
	//			sin >> a >> b >> c;
	//			vertices.push_back(a);
	//			vertices.push_back(b);
	//			vertices.push_back(c);
	//		}
	//		else {
	//			int a, b, c;
	//			sin >> a >> b >> c;
	//			faces.push_back(a);
	//			faces.push_back(b);
	//			faces.push_back(c);
	//		}
	//	}
	//}
	//float *vertices_array = new float[vertices.size()];
	//int *faces_array = new int[faces.size()];


	//for (int i = 0; i < vertices.size(); ++i)
	//	vertices_array[i] = vertices[i] - (i % 3 == 1 ? - 0.78 : 0);
	//for (int i = 0; i < faces.size(); ++i)
	//	faces_array[i] = faces[i];

	//InitCollisionObject(vertices.size() / 3, faces.size() / 3, vertices_array, faces_array);

	//HairParameter param;

	//ifstream fin;
	//fin.open("C:\\Users\\vivid\\Desktop\\newconfig.ini", ios::in);
	//fin.read(param.root, 65536);
	//fin.close();

	//InitializeHairEngine(&param, nullptr, nullptr, nullptr);

	////identity matrix
	//float rigid[16];
	//for (int i = 0; i < 4; ++i)
	//	for (int j = 0; j < 4; ++j)
	//		rigid[i * 4 + j] = (i == j ? 1.0 : 0.0);

	//int size = GetHairParticleCount();
	//float *positions = new float[3 * size];
	//float *directions = new float[3 * size];

	//UpdateHairEngine(rigid, positions, directions, 0.03f);
	//for (int i = 0; i < 3 * size; i += 3)
	//	cout << '(' << positions[i] << ',' << positions[i + 1] << ',' << positions[i + 2] << ')' << endl;
	// 
	//ReleaseHairEngine();
}

