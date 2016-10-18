#include <fstream>
#include <algorithm>
#include "GridCollisionObject.h"
#include "macros.h"
#include "XConfigReader.hpp"
#include "xlogger.h"
#include "linmath.h"
#include "wrGeo.h"
#include "xmath.h"

namespace XRwy
{
#define VERY_LARGE 1.0e20f

	using namespace XR;

	GridCollisionObject::GridCollisionObject(const char* fileName)
	{
		ParameterDictionary gridParas;
		ConfigReader reader("../config2.ini");
		reader.getParamDict(gridParas);
		reader.close();

		tolerance = std::stof(gridParas["overcorrectiontol"]);
		correctionRate = std::stof(gridParas["correctionrate"]);
		maxstep = std::stof(gridParas["maxstep"]);

		std::ifstream gridFile(fileName, std::ios::binary);
		if (!gridFile.is_open()) throw std::exception("file not found!");

		Read4Bytes(gridFile, x);
		Read4Bytes(gridFile, y);
		Read4Bytes(gridFile, z);
		yz = y * z;

		Read4Bytes(gridFile, step);
		ReadNBytes(gridFile, bbox, sizeof(float) * 6);
		for (int i = 0; i < 3; i++)
			diag[i] = bbox[3 + i] - bbox[i];

		data = new WR::LevelSetVData[x*yz];
		ReadNBytes(gridFile, data, sizeof(WR::LevelSetVData)*x*yz);

		gridFile.close();
	}

	GridCollisionObject::~GridCollisionObject()
	{
		SAFE_DELETE_ARRAY(data);
	}


	void GridCollisionObject::computeIdx(const Point_3& p, float& i, float& j, float& k) const
	{
		i = (p.x() - bbox[0]) / step;
		j = (p.y() - bbox[1]) / step;
		k = (p.z() - bbox[2]) / step;
	}


	float GridCollisionObject::query_distance(const Point_3& p) const
	{
		float i, j, k;
		computeIdx(p, i, j, k);

		int ci = static_cast<int>(i);
		int cj = static_cast<int>(j);
		int ck = static_cast<int>(k);

		if (ci < 0 || ci > x-2 || cj < 0 || cj > y-2 || ck < 0 || ck > z-2) return VERY_LARGE;
		float localcoord[] = { i - ci, j - cj, k - ck };
		float dists[] = { query(i, j, k)->value, query(i + 1, j, k)->value,
			query(i, j + 1, k)->value, query(i + 1, j + 1, k)->value,
			query(i, j, k + 1)->value, query(i + 1, j, k + 1)->value,
			query(i, j + 1, k + 1)->value, query(i + 1, j + 1, k + 1)->value};

		float dist;
		WRG::trilinear_intp(dists, localcoord, &dist);

		return dist;
	}

	float GridCollisionObject::query_squared_distance(const Point_3& p) const
	{
		return 0.0;
	}

	bool GridCollisionObject::exceed_threshhold(const Point_3& p, float thresh) const
	{
		XLOG_ERROR << UNIMPLEMENTED_DECLARATION;
		return false;
	}

	bool GridCollisionObject::position_correlation(const Point_3& p, Point_3* pCorrect, float thresh) const
	{
		if (p.x() > bbox[3] || p.x() < bbox[0] ||
			p.y() > bbox[4] || p.y() < bbox[1] ||
			p.z() > bbox[5] || p.z() < bbox[2])
			return false;

		float i, j, k, dist;
		int ci, cj, ck;
		bool bChanged = false;
		*pCorrect = p;

		int n = 3;
		while (n--)
		{
			computeIdx(*pCorrect, i, j, k);

			int ci = static_cast<int>(i);
			int cj = static_cast<int>(j);
			int ck = static_cast<int>(k);

			if (ci < 0 || ci > x - 2 || cj < 0 ||
				cj > y - 2 || ck < 0 || ck > z - 2)
			{
				if (!bChanged) return false;
				else dist = VERY_LARGE;
			}
			else
			{
				float localcoord[] = { i - ci, j - cj, k - ck };
				float dists[] = { query(i, j, k)->value, query(i + 1, j, k)->value,
					query(i, j + 1, k)->value, query(i + 1, j + 1, k)->value,
					query(i, j, k + 1)->value, query(i + 1, j, k + 1)->value,
					query(i, j + 1, k + 1)->value, query(i + 1, j + 1, k + 1)->value };
				WRG::trilinear_intp(dists, localcoord, &dist);
			}

			if (dist > thresh)
			{
				if (!bChanged || dist < thresh + tolerance)
				{
					//if (bChanged)
					//	std::cout << "iter: " << 9 - n << " ";
					return bChanged;
				}
			}

			// correction
			float localcoord[] = { i - ci, j - cj, k - ck };
			float* igrad = query(i + (localcoord[0] < 0.5 ? 0 : 1), 
				j + (localcoord[1] < 0.5 ? 0 : 1), k + (localcoord[2] < 0.5 ? 0 : 1))->grad;

			//float *grads[] = { query(i, j, k)->grad, query(i + 1, j, k)->grad,
			//	query(i, j + 1, k)->grad, query(i + 1, j + 1, k)->grad,
			//	query(i, j, k + 1)->grad, query(i + 1, j, k + 1)->grad,
			//	query(i, j + 1, k + 1)->grad, query(i + 1, j + 1, k + 1)->grad };
			//WRG::trilinear_intp_batch<float, 3>((float**)grads, localcoord, (float*)igrad);

			// TODO should use neighbor interpolation

			float diff;
			if (dist < thresh)
				diff = dist - (thresh + tolerance);
			else diff = dist - thresh;

			float corrStep = -diff*correctionRate;
			float corr[3];
			vec3_scale(corr, igrad, corrStep);

			*pCorrect = Point_3(pCorrect->x() + corr[0], pCorrect->y() + corr[1], pCorrect->z() + corr[2]);
			bChanged = true;
		}
		//std::cout << "iter: " << 9 - n << " ";
		return bChanged;
	}
}


