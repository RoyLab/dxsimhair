#include "HairScattering.hpp"
#include "BitmapWriter.hpp"
#include "Marschner.h"
#include <cmath>

#define M_PI 3.14159265358979323846

void hairtest()
{
	const double d1 = 0.0174532925f;
	IECore::MarschnerBCSDF<float> A(1.55f, 0.54f, 1.0f, -8 * d1, -4 * d1, -12 * d1, 8 * d1, 4 * d1, 16 * d1, 0.4f, 1.5*d1, 0.1f, 0.5f);
	IECore::MarschnerBCSDF<float> B(1.55f, 0.64f, 1.0f, -8 * d1, -4 * d1, -12 * d1, 8 * d1, 4 * d1, 16 * d1, 0.4f, 1.5*d1, 0.1f, 0.5f);
	IECore::MarschnerBCSDF<float> C(1.55f, 0.8f, 1.0f, -8 * d1, -4 * d1, -12 * d1, 8 * d1, 4 * d1, 16 * d1, 0.4f, 1.5*d1, 0.1f, 0.5f);

	LightScattering l;
	const int c = 20;

	int w = c, h = c;
	float **r = new float*[h];
	float **g = new float*[h];
	float **b = new float*[h];

	for (int i = 0; i < h; i++)
	{
		r[i] = new float[w];
		g[i] = new float[w];
		b[i] = new float[w];

		for (int j = 0; j < w; j++)
		{
			float x = M_PI*2 * i / c;
			float y =  M_PI * j / c - M_PI /2.0f;

			std::array<float, 2> a, b1;
			b1[0] = 0; b1[1] = M_PI / 4;
			a[0] = x; a[1] = y;

			//l.scattering(0, 0, thetai, thetao, 0);

			r[i][j] = A.NR(a, b1);//A.MR(a, b1);//* +A.MTT(a, b1)*A.NTT(a, b1), A.MTRT(a, b1)*A.NTRT(a, b1);
			if (j == 0)
				std::cout << r[i][j] << '\t' << a[0] << '\t' << a[1] << '\n';
			//g[i][j] = B(a, b1) * 0;
			//b[i][j] = C(a, b1) * 0;
			//std::cout << r[i][j] << '\t' << x << '\t' << y << std::endl;
		}
	}

	r[0][0] = 0;
	g[0][0] = 0;
	b[0][0] = 0;

	r[0][2] = 0;
	g[0][2] = 0;
	b[0][2] = 0;

	XR::writeBitmap("D:/test.bmp", w, h, r, g, b);
}