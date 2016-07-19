#pragma once
#include <d3d11.h>
#include "XRwy_h.h"
#include "IRenderer.h"


namespace XRwy
{
	class FollicleRenderer :
		public IRenderer
	{
	public:
		struct ConstBuffer
		{
			Matrix  projViewWorld;
			Float3  viewPoint;
			float	pointSize;
		};

		static const D3D11_INPUT_ELEMENT_DESC LayoutDesc[5];

	public:
		void SetConstantBuffer(const ConstBuffer* buffer);
		void SetRenderState(int i = 0, void* = nullptr);
		int GetNumOfRenderPass() const { return 2; }
		void GetVertexShaderBytecode(void const** pShaderByteCode, size_t* pByteCodeLength, void*);

		bool Initialize();
		void Release();

	private:
		ID3D11Device*           pd3dDevice = nullptr;
		ID3D11DeviceContext*    pd3dImmediateContext = nullptr;

		ID3DBlob* pVSBlob = nullptr;
		ID3D11VertexShader*     pVS = nullptr;
		ID3D11GeometryShader*   pGS = nullptr;
		ID3D11PixelShader*      pPS = nullptr;
		ID3D11Buffer*           pConstBuffer = nullptr;

		Matrix                  mlightProjViewWorld;
	};
}
