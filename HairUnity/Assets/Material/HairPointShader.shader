Shader "Custom/HairPointShader" {
	Properties{
		_Color("Color", Color) = (0,0,0,1)
		_StrokeWidth("Hair Stroke Width", Float) = 0.03
	}
	SubShader {
		Tags { 
			"RenderType"="Opaque" 
			"Queue" = "Geometry"
		}

		Cull Off

		Pass {
			CGPROGRAM

			#pragma vertex vert
			#pragma fragment frag
			#pragma geometry geo
			#include "UnityCG.cginc" 

			half4 _Color;
			float _StrokeWidth;

			struct VS_INPUT {
				float4 pos: POSITION;
				half4 color: COLOR;
			};

			struct GS_INPUT {
				float4 pos: POSITION;
				half4 color: COLOR0;
			};

			struct PS_INPUT {
				float4 pos: POSITION;
				half4 color: COLOR0;
			};

			GS_INPUT vert(VS_INPUT v) {
				GS_INPUT g;
				g.pos = mul(UNITY_MATRIX_MVP, v.pos);
				g.color = v.color;
				return g;
			}

			[maxvertexcount(6)]
			void geo(line GS_INPUT gs[2], inout TriangleStream<PS_INPUT> ps) {
				PS_INPUT p[4];
				p[0].pos = p[1].pos = gs[0].pos;
				p[0].color = p[1].color = gs[0].color;
				p[2].pos = p[3].pos = gs[1].pos;
				p[2].color = p[3].color = gs[1].color;
				p[0].pos.x += _StrokeWidth;
				p[1].pos.x -= _StrokeWidth;
				p[2].pos.x += _StrokeWidth;
				p[3].pos.x -= _StrokeWidth;

				ps.Append(p[0]);
				ps.Append(p[1]);
				ps.Append(p[2]);
				ps.RestartStrip();
				ps.Append(p[1]);
				ps.Append(p[2]);
				ps.Append(p[3]);
			}

			half4 frag(PS_INPUT p) : SV_TARGET{
				return p.color;
			}

			ENDCG
		}
	}
	FallBack "Diffuse"
}
