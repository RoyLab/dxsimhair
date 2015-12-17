#pragma once

struct wrSimpleVertexInput
{
    DirectX::XMFLOAT3 position;
    DirectX::XMFLOAT3 normal;
};

void CreateSphere(float r, int stacks, int slices, wrSimpleVertexInput** vArr, DWORD** iArr);