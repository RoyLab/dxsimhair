// test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <string>
#include <fstream>
#include <boost/log/trivial.hpp>


int _tmain(int argc, _TCHAR* argv[])
{
	char cbuffer[4];
	std::ifstream file("../../models/straight.hair");
	if (file) 
	{
		file.read(cbuffer, sizeof(int));
		std::cout << *reinterpret_cast<int*>(cbuffer) << std::endl;

		int n_particles = *reinterpret_cast<int*>(cbuffer);
		float *position = new float[3 * n_particles];

		file.read((char*)position, sizeof(float) * 3 * n_particles);

		for (int i = 0; i < 10; i++)
		{
			std::cout << position[i] << std::endl;
		}
	}
	file.close();
    BOOST_LOG_TRIVIAL(trace) << 3;
    BOOST_LOG_TRIVIAL(debug) << "A debug severity message";
    BOOST_LOG_TRIVIAL(info) << "An informational severity message";
    BOOST_LOG_TRIVIAL(warning) << "A warning severity message";
    BOOST_LOG_TRIVIAL(error) << "An error severity message";
    BOOST_LOG_TRIVIAL(fatal) << "A fatal severity message";

    std::string a("fadsfds, %d", 3);

	system("pause");

	return 0;
} 

