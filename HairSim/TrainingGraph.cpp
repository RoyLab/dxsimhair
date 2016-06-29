#include <DXUT.h>
#include <fstream>

#include "TrainingGraph.h"
#include "wrMacro.h"

namespace XRwy
{
    EdgeVisualization::~EdgeVisualization()
    {
        SAFE_RELEASE(pIndexBuffer);
    }

    void EdgeVisualization::render()
    {
        auto pCtx = DXUTGetD3D11DeviceContext();
        UINT offset = 0;
        // TO-DO
        //pCtx->IASetVertexBuffers(0, 1, &pVertexBuffer, &stride, &offset);
        pCtx->IASetIndexBuffer(pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
        pCtx->DrawIndexed(idxEnd - idxStart, idxStart, 0);
    }

    void EdgeVisualization::updateBuffers()
    {
        DWORD *indices = new DWORD[nEdge*2];
        int ptr = 0;
        for (int i = 0; i < nStep+1; i++)
        {
            auto& list = edgeList[i].pairs;
            for (auto itr = list.begin(); itr != list.end(); itr++)
            {
                indices[ptr++] = itr->data[0] * 25;
                indices[ptr++] = itr->data[1] * 25;
            }
        }
        assert(ptr == nEdge * 2);
/*
        for (int i = 0; i < nEdge * 2; i++)
        {
            indices[i] = i;
        }*/

        D3D11_SUBRESOURCE_DATA subRes;
        CD3D11_BUFFER_DESC bDesc;
        HRESULT hr;

        ZeroMemory(&bDesc, sizeof(CD3D11_BUFFER_DESC));
        ZeroMemory(&subRes, sizeof(D3D11_SUBRESOURCE_DATA));

        bDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
        bDesc.ByteWidth = (2 * nEdge) * sizeof(DWORD);
        bDesc.CPUAccessFlags = 0;
        bDesc.Usage = D3D11_USAGE_DEFAULT;

        subRes.pSysMem = indices;

        V(DXUTGetD3D11Device()->CreateBuffer(&bDesc, &subRes, &pIndexBuffer));
        SAFE_DELETE_ARRAY(indices);
    }

    void EdgeVisualization::loadGraph(const wchar_t* fileName)
    {
        std::ifstream file(fileName, std::ios::binary);
        if (!file.is_open()) throw std::exception("file not found!");

        char buffer[16];
        file.read((char*)&nEdge, 4);
        file.read((char*)&nStep, 4);

        edgeList.resize(nStep + 1);

        for (int i = 0; i < nEdge; i++)
        {
            file.read(buffer, 12);
            int* ptr = reinterpret_cast<int*>(&buffer);
            edgeList[ptr[2]-1].pairs.emplace_back(ptr[0], ptr[1]);
        }

        /* update the offset */
        for (int i = 0, count = 0; i <= nStep; i++)
        {
            edgeList[i].offset = count;
            count += edgeList[i].pairs.size();
        }

        file.close();
        updateBuffers();
    }

    void EdgeVisualization::setRange(int mean, int bandwidth)
    {
        lowBound = mean - bandwidth;
        highBound = mean + bandwidth;

        if (lowBound < 0) lowBound = 0;
        if (highBound < 0) highBound = 0;
        if (lowBound > edgeList.size()-1) lowBound = edgeList.size()-1;
        if (highBound > edgeList.size()-1) highBound = edgeList.size()-1;

        idxStart = edgeList[lowBound].offset * 2;
        if (highBound == edgeList.size()) idxEnd = nEdge*2;
        else idxEnd = edgeList[highBound].offset * 2;
    }
}