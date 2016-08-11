#include "UnitTest.h"
#include "ADFOctree.h"
#include <boost/foreach.hpp>
#include <CGAL/point_generators_3.h>
#include <vector>
#include <fstream>
#include <DirectXMath.h>

#define WR_EXPORTS
#include "LevelSet.h"
#include "ADFCollisionObject.h"
#include "ConfigReader.h"
#include "linmath.h"
#include "GridCollisionObject.h"

using std::cout;
using std::endl;

namespace WR
{ 
    typedef K::FT FT;
    typedef K::Point_3 Point;


	DirectX::XMMATRIX ComputeHeadTransformation(const float* trans4x4)
	{
		using namespace DirectX;
		auto target0 = XMFLOAT4X4(trans4x4);
		auto trans0 = XMFLOAT3(0.0f, -0.643f, 0.282f);
		auto scale0 = XMFLOAT3(5.346f, 5.346f, 5.346f);
		return XMMatrixAffineTransformation(XMLoadFloat3(&scale0), XMVectorZero(), XMVectorZero(), XMLoadFloat3(&trans0))*XMMatrixTranspose(XMLoadFloat4x4(&target0));
	}

    void loadPoints(const wchar_t* fileName, std::vector<Point>& parr)
    {
        std::ifstream file(fileName);
        assert(file);
        while (!file.eof())
        {
            Point p;
            file >> p;
            parr.push_back(p);
        }
        file.close();
    }


	extern "C" WR_API ICollisionObject* CreateGridCollisionObject(const char* fileName)
	{
		return new XRwy::GridCollisionObject(fileName);
	}


	extern "C" WR_API ICollisionObject* createCollisionObject(Polyhedron_3_FaceWithId& poly)
    {
        ADFOctree* pTree = new ADFOctree;
        pTree->construct(poly, 2);
        ICollisionObject* pCO = pTree->releaseAndCreateCollisionObject();
        delete pTree;
        return pCO;
    }

	extern "C" WR_API ICollisionObject* loadCollisionObject(const wchar_t* fileName)
    {
        return new ADFCollisionObject(fileName);
    }

	extern "C" WR_API ICollisionObject* createCollisionObject2(const wchar_t* fileName)
    {
        Polyhedron_3_FaceWithId* pModel = WRG::readFile<Polyhedron_3_FaceWithId>(fileName);
        assert(pModel);
        ICollisionObject* pCO = createCollisionObject(*pModel);
        assert(pCO);
        delete pModel;
        return pCO;
    }

    template <class Poly>
    float max_coordinate(const Poly& poly)
    {
        float max_coord = (std::numeric_limits<float>::min)();
        BOOST_FOREACH(Poly::Vertex_handle v, vertices(poly))
        {
            Point p = v->point();
            max_coord = (std::max)(max_coord, p.x());
            max_coord = (std::max)(max_coord, p.y());
            max_coord = (std::max)(max_coord, p.z());
        }
        return max_coord;
    }

	using namespace DirectX;

    void runLevelSetBenchMark(const wchar_t* fileName)
    {
        ConfigReader reader("..\\config.ini");
        int level = std::stoi(reader.getValue("maxlevel"));
        int number = std::stoi(reader.getValue("testnumber"));
        bool loadArray = std::stoi(reader.getValue("loadtest"));
        float tmp = std::stof(reader.getValue("boxgap"));
        bool regen = std::stoi(reader.getValue("regenmodel"));

        ADFOctree::set_box_enlarge_size(tmp);

        Polyhedron_3_FaceWithId* pModel = WRG::readFile<Polyhedron_3_FaceWithId>(fileName);
        assert(pModel);

		mat4x4 identity;  mat4x4_identity(identity);
		auto mat = ComputeHeadTransformation(reinterpret_cast<float*>(identity));

		for (auto itr = pModel->vertices_begin(); itr != pModel->vertices_end(); itr++)
		{
			auto &vertex = itr->point();
			vec4 v{ vertex[0], vertex[1], vertex[2], 1.0f };
			DirectX::XMVECTOR pos = DirectX::XMLoadFloat4(reinterpret_cast<DirectX::XMFLOAT4*>(v));
			auto newpos = DirectX::XMVector3Transform(pos, mat);
			XMFLOAT4 last; XMStoreFloat4(&last, newpos);
			itr->point() = Polyhedron_3_FaceWithId::Point_3(last.x, last.y, last.z);
		}

        auto* tester = CGAL::Ext::createDistanceTester<Polyhedron_3_FaceWithId, K>(*pModel);

        ICollisionObject* pCO = nullptr;
        if (regen)
        {
			/* generate octree level set obj */
            ADFOctree* pTree = new ADFOctree;
            pTree->construct(*pModel, level);
            pCO = pTree->releaseAndCreateCollisionObject();
            delete pTree;
        }
        else
        {
            //pCO = new ADFCollisionObject(L"hair");

			/* generate grid level set obj */
			pCO = CreateGridCollisionObject("D:/hair project/models/head.grid");
        }

        double size = max_coordinate(*pModel);
        std::vector<Point> points;
        if (loadArray)
        {
            loadPoints(L"test.dat", points);
            number = points.size();
        }
        else
        {
            points.reserve(number);
            CGAL::Random_points_in_cube_3<Point> gen(size);
            for (unsigned int i = 0; i < number; ++i)
                points.push_back(*gen++);
        }

        while (number--)
        {
            auto & p = points[number];
            double dist = pCO->query_distance(p);
            double std_dist = tester->query_signed_distance(p);
            cout << "Point: " << p << endl;
			if (dist < 1000)
				cout << " Dist1: " << dist << " Dist2: " << std_dist <<
                "\t\t diff: " << dist - std_dist << endl;

            Point crt;
            if (pCO->position_correlation(p, &crt))
				cout << "                                  corrected distance: " << pCO->query_distance(crt) << endl;

			cout << endl;
        }

        //if (regen)
        //    pCO->save_model(L"hair");

        delete pCO;

        delete tester;
        delete pModel;
    }
}