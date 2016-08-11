#include <CGAL/Simple_cartesian.h>
#include <CGAL/Polyhedron_3.h>
#include <CGAL/IO/Polyhedron_iostream.h>
#include <CGAL/bounding_box.h>

#include <fstream>
#include <DirectXMath.h>
#include <cmath>
#include "UnitTest.h"
#include "wrGeo.h"
#include "linmath.h"
#include "ICollisionObject.h"
#include "macros.h"

using namespace CGAL;
using namespace DirectX;

typedef Simple_cartesian<float> K;
typedef Polyhedron_3<K> Polyhedron;

DirectX::XMMATRIX ComputeHeadTransformation(const float* trans4x4)
{
	auto target0 = XMFLOAT4X4(trans4x4);
	auto trans0 = XMFLOAT3(0.0f, -0.643f, 0.282f);
	auto scale0 = XMFLOAT3(5.346f, 5.346f, 5.346f);
	return XMMatrixAffineTransformation(XMLoadFloat3(&scale0), XMVectorZero(), XMVectorZero(), XMLoadFloat3(&trans0))*XMMatrixTranspose(XMLoadFloat4x4(&target0));
}

void ApplyTransformation(Polyhedron& poly)
{
	mat4x4 identity;  mat4x4_identity(identity);
	auto mat = ComputeHeadTransformation(reinterpret_cast<float*>(identity));

	for (auto itr = poly.vertices_begin(); itr != poly.vertices_end(); itr++)
	{
		auto &vertex = itr->point();
		vec4 v{ vertex[0], vertex[1], vertex[2], 1.0f };
		DirectX::XMVECTOR pos = DirectX::XMLoadFloat4(reinterpret_cast<DirectX::XMFLOAT4*>(v));
		auto newpos = DirectX::XMVector3Transform(pos, mat);
		XMFLOAT4 last; XMStoreFloat4(&last, newpos);
		itr->point() = Polyhedron::Point_3(last.x, last.y, last.z);
	}
}

#define IDX(i, j, k) ((i) * grids[1]*grids[2] + (j) * grids[2] + (k))

void GenerateGridLevelSet(const char* fileName, float resolution, const char* outputFile)
{
	Polyhedron iMesh;
	std::ifstream iFile(fileName);
	iFile >> iMesh;
	iFile.close();
	std::cout << "Read off file: " << fileName << " nVertices: "
		<< iMesh.size_of_vertices() << std::endl;

	ApplyTransformation(iMesh);
	Ext::DistanceTester<Polyhedron, K> tester(iMesh);

	auto bbox = CGAL::bounding_box(iMesh.points_begin(), iMesh.points_end());
	std::cout << "BoundingBox is: " << bbox << std::endl;
	WRG::enlarge(bbox, 0.1f);

	auto diag = bbox.max() - bbox.min();
	int res[] = { static_cast<int>(std::ceil(diag.x() / resolution))+1
		, static_cast<int>(std::ceil(diag.y() / resolution))+1
		, static_cast<int>(std::ceil(diag.z() / resolution))+1 };
	int grids[] = { res[0] + 2, res[1] + 2, res[2] + 2 };
	int total = (grids[0]) * (grids[1]) * (grids[2]);

	float xPos = bbox.min().x() - resolution;
	float yPos = bbox.min().y() - resolution;
	float zPos = bbox.min().z() - resolution;

	WR::LevelSetVData* data = new WR::LevelSetVData[total];
	std::cout << "Allocate Memory: " << sizeof(WR::LevelSetVData) * total << " bytes\n";

	int count = 0;
	for (int i = 0; i < grids[0]; i++)
	{
		yPos = bbox.min().y() - resolution;
		for (int j = 0; j < grids[1]; j++)
		{
			zPos = bbox.min().z() - resolution;
			for (int k = 0; k < grids[2]; k++)
			{
				int idx = IDX(i, j, k);
				float tmp = tester.query_signed_distance(K::Point_3(xPos, yPos, zPos));
				assert(!std::isnan(tmp) && tmp > -1e10 && tmp < 1e10);

				data[idx].value = tmp;
				if (tmp < 0) count++;
				assert(!std::isnan(data[idx].value));
				zPos += resolution;
			}
			yPos += resolution;
		}
		xPos += resolution;
	}
	std::cout << count << " / " << total << " points are inside.\n";

	float sum = 0.0f;
	for (int i = 1; i < res[0]+1; i++)
	{
		for (int j = 1; j < res[1]+1; j++)
		{
			for (int k = 1; k < res[2]+1; k++)
			{
				int idx = IDX(i, j, k);
				data[idx].grad[0] = (data[IDX(i+1, j, k)].value - data[IDX(i-1, j, k)].value) / (2 * resolution);
				data[idx].grad[1] = (data[IDX(i, j+1, k)].value - data[IDX(i, j-1, k)].value) / (2 * resolution);
				data[idx].grad[2] = (data[IDX(i, j, k+1)].value - data[IDX(i, j, k-1)].value) / (2 * resolution);
				sum += vec3_len(data[idx].grad);
				assert(!std::isnan(vec3_len(data[idx].grad)));
				//vec3_norm(data[idx].grad, data[idx].grad);
			}
		}
	}
	std::cout << "Ave. length: " << sum / (res[0] * res[1] * res[2]) << std::endl;

	std::ofstream oFile(outputFile, std::ios::binary);

	Write4Bytes(oFile, res[0]);
	Write4Bytes(oFile, res[1]);
	Write4Bytes(oFile, res[2]);
	Write4Bytes(oFile, resolution);

	float wBBox[6] = { bbox.xmin(), bbox.ymin(), bbox.zmin(), };
	for (int i = 0; i < 3; i++)
		wBBox[3 + i] = wBBox[i] + resolution * (res[i]-1);
	WriteNBytes(oFile, wBBox, sizeof(float) * 6);

	for (int i = 1; i < res[0] + 1; i++)
	{
		for (int j = 1; j < res[1] + 1; j++)
		{
			for (int k = 1; k < res[2] + 1; k++)
			{
				int idx = IDX(i, j, k);
				WriteNBytes(oFile, data + idx, sizeof(WR::LevelSetVData));
			}
		}
	}
	oFile.close();
	delete[]data;
}