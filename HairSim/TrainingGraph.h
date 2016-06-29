#pragma once
#include <list>
#include <vector>

struct ID3D11Buffer;

namespace XRwy
{
    class EdgeVisualization
    {
        struct IntPair
        {
            IntPair(int a, int b){ data[0] = a; data[1] = b; }
            int data[2];
        };

        struct Node
        {
            int offset;
            std::list<IntPair> pairs;
        };

        typedef std::vector<Node> WeightOrderEdgeList;

    public:
        EdgeVisualization(){}
        ~EdgeVisualization();

        void loadGraph(const wchar_t* fileName);
        void render();
        void setRange(int mean, int bandwidth);
        void setObject(ID3D11Buffer* buffer, UINT stride)
        {
            pVertexBuffer = buffer;
            this->stride = stride;
        }

    private:
        void updateBuffers();

        WeightOrderEdgeList edgeList;

        int lowBound, highBound;
        int idxStart, idxEnd;
        int nEdge = 0, nStep = 0;

        ID3D11Buffer* pIndexBuffer = nullptr;
        ID3D11Buffer* pVertexBuffer = nullptr;
        UINT stride = 0;
    };
}