#pragma once
#include <fstream>
#include <DirectXMath.h>
#include "XRwy_h.h"
#include "xmath.h"
#include "HairStructs.h"

namespace XRwy
{
	class ZhouHairLoader :
		public HairLoader
	{
	public:
		bool loadFile(const char* fileName, HairGeometry * geom)
		{
			std::ifstream file(fileName, std::ios::binary);
			if (file)
			{
				int factor = std::stoi(g_paramDict["particleperstrand"]);
				char* cbuffer = new char[sizeof(float) * 3 * std::stoi(g_paramDict["particleperstrand"])];

				file.read(cbuffer, sizeof(int));
				int n_particles = *reinterpret_cast<int*>(cbuffer);

				file.seekg(3 * n_particles * sizeof(float) + sizeof(int));
				file.read(cbuffer, sizeof(int));
				int n_strands = *reinterpret_cast<int*>(cbuffer);

				int sampleRatio = std::stoi(g_paramDict["hairsample"]);
				geom->nStrand = n_strands / sampleRatio;
				geom->nParticle = geom->nStrand * factor;

				wchar_t ch[256];
				wchar_t wc[256];
				std::mbstowcs(wc, fileName, std::strlen(fileName) + 1);
				wsprintf(ch, L"Loading %ls, total particles: %d, total strands: %d\n", wc, n_particles, n_strands);
				OutputDebugString(ch);

				file.seekg(sizeof(int));
				
				geom->position = new XMFLOAT3[geom->nParticle];
				geom->direction = new XMFLOAT3[geom->nParticle];
				DirectX::XMStoreFloat4x4(reinterpret_cast<DirectX::XMFLOAT4X4*>(
					geom->rigidTrans), DirectX::XMMatrixIdentity());
				geom->particlePerStrand = factor;

				DirectX::XMStoreFloat4x4(&geom->worldMatrix, DirectX::XMMatrixIdentity());
				int hairRendererVersion = std::stoi(g_paramDict["hairrendversion"]);
				for (int i = 0; i < geom->nStrand; i++)
				{
					file.read(cbuffer, sizeof(float) * 3 * factor);
					if (file.eof())
						throw std::exception("unexpected end of file");

					memcpy(geom->position + i*factor, cbuffer, sizeof(float)*factor * 3);

					if (hairRendererVersion == 1)
					{
						vec3 dir;
						float* pos1, *pos2;
						for (int j = 1; j < factor; j++)
						{
							pos1 = reinterpret_cast<float*>(geom->position + i*factor + j);
							pos2 = reinterpret_cast<float*>(geom->position + i*factor + j + 1);
							vec3_sub(dir, pos2, pos1);
							float norm = vec3_len(dir);
							vec3_scale(dir, dir, 1 / norm);
							memcpy(geom->direction + i*factor + j, dir, sizeof(vec3));
						}
						memcpy(geom->direction + i*factor, geom->direction + i*factor + 1, sizeof(vec3));
					}

					file.seekg(sizeof(int) + (sampleRatio * i + static_cast<int>(sampleRatio * randf()))
						* sizeof(float) * 3 * factor);
				}

				XMFLOAT3 scale = { 0.05f, -0.05f, 0.05f };
				for (int i = 0; i < geom->nParticle; i++)
				{
					XMFLOAT3 *ptr = geom->position + i;
					ptr->x *= scale.x;
					ptr->y *= scale.y;
					ptr->z *= scale.z;
				}

				file.close();
				delete[] cbuffer;
			}

			return true;
		}

	};

}
