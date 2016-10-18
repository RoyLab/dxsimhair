#include "precompiled.h"
#include "FollicleRenderer.h"

namespace XRwy
{
	void FollicleRenderer::SetConstantBuffer(const ConstBuffer * buffer)
	{
		HRESULT hr;
		D3D11_MAPPED_SUBRESOURCE MappedResource;

		V(pd3dImmediateContext->Map(pConstBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource));
		auto data = reinterpret_cast<ConstBuffer*>(MappedResource.pData);
		CopyMemory(data, buffer, sizeof(ConstBuffer));
		pd3dImmediateContext->Unmap(pConstBuffer, 0);
	}

	void FollicleRenderer::SetRenderState(int i, void *)
	{
		pd3dImmediateContext->GSSetConstantBuffers(0, 1, &pConstBuffer);

		pd3dImmediateContext->VSSetShader(pVS, nullptr, 0);
		pd3dImmediateContext->GSSetShader(pGS, nullptr, 0);
		pd3dImmediateContext->PSSetShader(pPS, nullptr, 0);
	}


	void FollicleRenderer::GetVertexShaderBytecode(void const ** pShaderByteCode, size_t * pByteCodeLength, void *)
	{
		*pShaderByteCode = pVSBlob->GetBufferPointer();
		*pByteCodeLength = pVSBlob->GetBufferSize();
	}

	bool FollicleRenderer::Initialize()
	{
		pd3dDevice = DXUTGetD3D11Device();
		pd3dImmediateContext = DXUTGetD3D11DeviceContext();

		HRESULT hr;
		DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
	#ifdef _DEBUG
		dwShaderFlags |= D3DCOMPILE_DEBUG;
		dwShaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
	#endif

		ID3DBlob* pPSBlob = nullptr;
		ID3DBlob* pGSBlob = nullptr;

		// Compile the vertex shader
		V_RETURN(DXUTCompileFromFile(L"point.hlsl", nullptr, "VS", "vs_4_0", dwShaderFlags, 0, &pVSBlob));

		hr = pd3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &pVS);
		if (FAILED(hr))
		{
			SAFE_RELEASE(pVSBlob);
			return hr;
		}

		// Compile the pixel shader
		V_RETURN(DXUTCompileFromFile(L"point.hlsl", nullptr, "PS", "ps_4_0", dwShaderFlags, 0, &pPSBlob));

		hr = pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &pPS);
		if (FAILED(hr))
		{
			SAFE_RELEASE(pPSBlob);
			return hr;
		}
		SAFE_RELEASE(pPSBlob);

		// Compile the geometry shader
		V_RETURN(DXUTCompileFromFile(L"point.hlsl", nullptr, "GS", "gs_4_0", dwShaderFlags, 0, &pGSBlob));

		hr = pd3dDevice->CreateGeometryShader(pGSBlob->GetBufferPointer(), pGSBlob->GetBufferSize(), nullptr, &pGS);
		if (FAILED(hr))
		{
			SAFE_RELEASE(pGSBlob);
			return hr;
		}
		SAFE_RELEASE(pGSBlob);

		CD3D11_BUFFER_DESC bDesc;
		ZeroMemory(&bDesc, sizeof(CD3D11_BUFFER_DESC));
		bDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bDesc.ByteWidth = sizeof(ConstBuffer);
		bDesc.Usage = D3D11_USAGE_DYNAMIC;
		bDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		V_RETURN(pd3dDevice->CreateBuffer(&bDesc, nullptr, &pConstBuffer));

		return true;
	}

	void FollicleRenderer::Release()
	{
		SAFE_RELEASE(pVSBlob);
		SAFE_RELEASE(pVS);
		SAFE_RELEASE(pPS);
		SAFE_RELEASE(pGS);
		SAFE_RELEASE(pConstBuffer);

		delete this;
	}

}
