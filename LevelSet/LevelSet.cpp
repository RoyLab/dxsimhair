#include "UnitTest.h"
#include <boost/foreach.hpp>
#include <CGAL/point_generators_3.h>
#include "LevelSet.h"
#include "ConfigReader.h"
#include "wrMath.h"
#include <vector>
#include <fstream>

#include <limits>
using std::cout;
using std::endl;

namespace WR
{ 
    typedef K::FT FT;
    typedef K::Point_3 Point;

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


    ICollisionObject* createCollisionObject(Polyhedron_3_FaceWithId& poly)
    {
        ADFOctree* pTree = new ADFOctree;
        pTree->construct(poly, 2);
        ICollisionObject* pCO = pTree->releaseAndCreateCollisionObject();
        delete pTree;
        return pCO;
    }

    ICollisionObject* createCollisionObject(const wchar_t* fileName)
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

    void runLevelSetBenchMark(const wchar_t* fileName)
    {
        ConfigReader reader("..\\HairSim\\config.ini");
        int level = std::stoi(reader.getValue("maxlevel"));
        int number = std::stoi(reader.getValue("testnumber"));
        bool loadArray = std::stoi(reader.getValue("loadtest"));
        float tmp = std::stof(reader.getValue("boxgap"));
        bool regen = std::stoi(reader.getValue("regenmodel"));

        ADFOctree::set_box_enlarge_size(tmp);

        Polyhedron_3_FaceWithId* pModel = WRG::readFile<Polyhedron_3_FaceWithId>(fileName);
        assert(pModel);

        auto* tester = CGAL::Ext::createDistanceTester<Polyhedron_3_FaceWithId, K>(*pModel);

        ADFCollisionObject* pCO = nullptr;
        if (regen)
        {
            ADFOctree* pTree = new ADFOctree;
            pTree->construct(*pModel, level);
            pCO = pTree->releaseAndCreateCollisionObject();
            delete pTree;
        }
        else
        {
            pCO = new ADFCollisionObject(L"hair");
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
            cout << " Dist1: " << dist << " Dist2: " << std_dist <<
                "\t\t diff: " << dist - std_dist << endl << endl;

            Point crt;
            pCO->position_correlation(p, &crt);
        }

        pCO->query_squared_distance(points[0]);

        if (regen)
            pCO->save_model(L"hair");

        delete pCO;

        delete tester;
        delete pModel;
    }
}