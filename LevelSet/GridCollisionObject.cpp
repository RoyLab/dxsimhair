#include <fstream>
#include <algorithm>
#include <memory>

#include <CGAL/bounding_box.h>

#include "GridCollisionObject.h"
#include "macros.h"
#include "XConfigReader.hpp"
#include "xlogger.h"
#include "linmath.h"
#include "wrGeo.h"
#include "xmath.h"
#include "UnitTest.h"

namespace XRwy
{
#define VERY_LARGE 1.0e20f

	using namespace XR;

	//GridCollisionObject::GridCollisionObject(const char* fileName)
	GridCollisionObject* GridCollisionObject::load_file(const char *file_path) {

		std::allocator<GridCollisionObject> grid_allocator;
		GridCollisionObject* grid = grid_allocator.allocate(1);

		grid->tolerance = 0.0;
		grid->correctionRate = 0.95;
		grid->maxstep = 1;

		std::ifstream gridFile(file_path, std::ios::binary);
		if (!gridFile.is_open()) throw std::exception("file not found!");

		Read4Bytes(gridFile, grid->x);
		Read4Bytes(gridFile, grid->y);
		Read4Bytes(gridFile, grid->z);
		grid->yz = grid->y * grid->z;

		float read_step[3];
		ReadNBytes(gridFile, read_step, sizeof(float) * 3);
		grid->step = Vector_3(read_step[0], read_step[1], read_step[2]);

		ReadNBytes(gridFile, grid->bbox, sizeof(float) * 6);
		for (int i = 0; i < 3; i++)
			grid->diag[i] = grid->bbox[3 + i] - grid->bbox[i];

		grid->data = new WR::LevelSetVData[grid->x * grid->yz];
		ReadNBytes(gridFile, grid->data, sizeof(WR::LevelSetVData) * grid->x * grid->yz);

		using namespace std;
		cout << "Grid Info: ..." << endl;
		cout << "reading file path=" << file_path << endl;
		cout << "x=" << grid->x << ", y=" << grid->y << ", z=" << grid->z << endl;
		cout << "bbox(x)=(" << grid->bbox[0] << ',' << grid->bbox[3] << "), bbox(y)=(" << grid->bbox[1] << ',' << grid->bbox[4] << "), bbox(z)=(" << grid->bbox[2] << ',' << grid->bbox[5] << ')' << endl;
		cout << "diag(x)=" << grid->diag[0] << ", diag(y)=" << grid->diag[1] << ", diag(z)=" << grid->diag[2] << endl;
		cout << "tolerance=" << grid->tolerance << endl;
		cout << "correction rate=" << grid->correctionRate << endl;
		cout << "maxstep=" << grid->maxstep << endl;
		cout << "step(x)=" << grid->step[0] << ", step(y)=" << grid->step[1] << ", step(z)=" << grid->step[2] << endl;

		gridFile.close();

		return grid;
	}

	GridCollisionObject::GridCollisionObject(const char* file_path) {
		tolerance = 0.0;
		correctionRate = 0.95;
		maxstep = 1;

		std::ifstream gridFile(file_path, std::ios::binary);
		if (!gridFile.is_open()) throw std::exception("file not found!");

		Read4Bytes(gridFile, x);
		Read4Bytes(gridFile, y);
		Read4Bytes(gridFile, z);
		yz = y * z;

		float read_step[3];
		ReadNBytes(gridFile, read_step, sizeof(float) * 3);
		step = Vector_3(read_step[0], read_step[1], read_step[2]);

		ReadNBytes(gridFile, bbox, sizeof(float) * 6);
		for (int i = 0; i < 3; i++)
			diag[i] = bbox[3 + i] - bbox[i];

		data = new WR::LevelSetVData[x * yz];
		ReadNBytes(gridFile, data, sizeof(WR::LevelSetVData) * x * yz);

		using namespace std;
		cout << "Grid Info: ..." << endl;
		cout << "reading file path=" << file_path << endl;
		cout << "x=" << x << ", y=" << y << ", z=" << z << endl;
		cout << "bbox(x)=(" << bbox[0] << ',' << bbox[3] << "), bbox(y)=(" << bbox[1] << ',' << bbox[4] << "), bbox(z)=(" << bbox[2] << ',' << bbox[5] << ')' << endl;
		cout << "diag(x)=" << diag[0] << ", diag(y)=" << diag[1] << ", diag(z)=" << diag[2] << endl;
		cout << "tolerance=" << tolerance << endl;
		cout << "correction rate=" << correctionRate << endl;
		cout << "maxstep=" << maxstep << endl;
		cout << "step(x)=" << step[0] << ", step(y)=" << step[1] << ", step(z)=" << step[2] << endl;

		gridFile.close();
	}

#define IDX(i, j, k) ((i) * grids[1]*grids[2] + (j) * grids[2] + (k))

	GridCollisionObject::GridCollisionObject(CGAL::Polyhedron_3<CGAL::FloatKernel> &iMesh, int slice)
	{
		using namespace CGAL;
		using K = FloatKernel;
		using Polyhedron = Polyhedron_3<K>;

		Ext::DistanceTester<Polyhedron, K> tester(iMesh);

		auto bbox = CGAL::bounding_box(iMesh.points_begin(), iMesh.points_end());
		std::cout << "BoundingBox is: " << bbox << std::endl;
		WRG::enlarge(bbox, 0.1f);

		auto diag = bbox.max() - bbox.min();
		auto resolution = diag / static_cast<float>(slice);
		int res[] = { static_cast<int>(std::ceil(diag.x() / resolution[0])) + 1
			, static_cast<int>(std::ceil(diag.y() / resolution[1])) + 1
			, static_cast<int>(std::ceil(diag.z() / resolution[2])) + 1 };
		int grids[] = { res[0] + 2, res[1] + 2, res[2] + 2 };
		int total = (grids[0]) * (grids[1]) * (grids[2]);

		float xPos = bbox.min().x() - resolution[0];
		float yPos = bbox.min().y() - resolution[1];
		float zPos = bbox.min().z() - resolution[2];

		WR::LevelSetVData* data = new WR::LevelSetVData[total];
		std::cout << "Allocate Memory: " << sizeof(WR::LevelSetVData) * total << " bytes\n";

		int count = 0;
		for (int i = 0; i < grids[0]; i++)
		{
			std::cout << i << std::endl;
			yPos = bbox.min().y() - resolution[1];
			for (int j = 0; j < grids[1]; j++)
			{
				zPos = bbox.min().z() - resolution[2];
				for (int k = 0; k < grids[2]; k++)
				{
					int idx = IDX(i, j, k);
					float tmp = tester.query_signed_distance(K::Point_3(xPos, yPos, zPos));
					assert(!std::isnan(tmp) && tmp > -1e10 && tmp < 1e10);

					data[idx].value = tmp;
					if (tmp < 0) count++;
					assert(!std::isnan(data[idx].value));
					zPos += resolution[2];
				}
				yPos += resolution[1];
			}
			xPos += resolution[0];
		}
		std::cout << count << " / " << total << " points are inside.\n";

		float sum = 0.0f;
		for (int i = 1; i < res[0] + 1; i++)
		{
			for (int j = 1; j < res[1] + 1; j++)
			{
				for (int k = 1; k < res[2] + 1; k++)
				{
					int idx = IDX(i, j, k);
					data[idx].grad[0] = (data[IDX(i + 1, j, k)].value - data[IDX(i - 1, j, k)].value) / (2 * resolution[0]);
					data[idx].grad[1] = (data[IDX(i, j + 1, k)].value - data[IDX(i, j - 1, k)].value) / (2 * resolution[1]);
					data[idx].grad[2] = (data[IDX(i, j, k + 1)].value - data[IDX(i, j, k - 1)].value) / (2 * resolution[2]);
					sum += vec3_len(data[idx].grad);
					assert(!std::isnan(vec3_len(data[idx].grad)));
					//vec3_norm(data[idx].grad, data[idx].grad);
				}
			}
		}

		auto avg_len = sum / (res[0] * res[1] * res[2]);
		std::cout << avg_len << std::endl;

		//std::ofstream oFile(outputFile, std::ios::binary);

		//Write4Bytes(oFile, res[0]);
		//Write4Bytes(oFile, res[1]);
		//Write4Bytes(oFile, res[2]);
		//Write4Bytes(oFile, resolution);

		//float wBBox[6] = { bbox.xmin(), bbox.ymin(), bbox.zmin(), };
		//for (int i = 0; i < 3; i++)
		//	wBBox[3 + i] = wBBox[i] + resolution[i] * (res[i] - 1);
		//WriteNBytes(oFile, wBBox, sizeof(float) * 6);

		//for (int i = 1; i < res[0] + 1; i++)
		//{
		//	for (int j = 1; j < res[1] + 1; j++)
		//	{
		//		for (int k = 1; k < res[2] + 1; k++)
		//		{
		//			int idx = IDX(i, j, k);
		//			WriteNBytes(oFile, data + idx, sizeof(WR::LevelSetVData));
		//		}
		//	}
		//}
		//oFile.close();
		//delete[]data;

		this->x = res[0];
		this->y = res[1];
		this->z = res[2];
		this->yz = y * z;

		this->bbox[0] = bbox.xmin();
		this->bbox[1] = bbox.ymin();
		this->bbox[2] = bbox.zmin();
		for (int i = 0; i < 3; ++i) {
			this->bbox[3 + i] = this->bbox[i] + resolution[i] * (res[i] - 1);
			this->diag[i] = this->bbox[3 + i] - this->bbox[i];
		}

		this->tolerance = 0.0;
		this->correctionRate = 0.95;
		this->maxstep = 1;

		this->step = resolution;

		this->data = new WR::LevelSetVData[res[0] * res[1] * res[2]];
		int data_idx = 0;
		for (int i = 1; i < res[0] + 1; i++)
		{
			for (int j = 1; j < res[1] + 1; j++)
			{
				for (int k = 1; k < res[2] + 1; k++)
				{
					int idx = IDX(i, j, k);
					this->data[data_idx] = data[idx];

					//if the grad is zero, then apply a avg_len direction to x coor
					if (this->data[data_idx].grad[0] == 0.0 && this->data[data_idx].grad[0] == 0.0 && this->data[data_idx].grad[2] == 0.0)
						this->data[data_idx].grad[0] = avg_len;

					++data_idx;
				}
			}
		}

		delete[] data;
	}

	void GridCollisionObject::write_to_file(const char* file_path) {
		std::ofstream fout(file_path, std::ios::binary); 

		int a = 3;
		Write4Bytes(fout, x);
		Write4Bytes(fout, y);
		Write4Bytes(fout, z);
		float write_step[] = { step[0], step[1], step[2] };		
		WriteNBytes(fout, write_step, sizeof(float) * 3);
		WriteNBytes(fout, bbox, sizeof(float) * 6);
		WriteNBytes(fout, data, sizeof(WR::LevelSetVData) * x * y * z);
	}

	GridCollisionObject::~GridCollisionObject()
	{
		SAFE_DELETE_ARRAY(data);
	}


	void GridCollisionObject::computeIdx(const Point_3& p, float& i, float& j, float& k) const
	{
		i = (p.x() - bbox[0]) / step[0];
		j = (p.y() - bbox[1]) / step[1];
		k = (p.z() - bbox[2]) / step[2];
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


