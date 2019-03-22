struct VertexIn
{
	float3 mPos : POSITION;
    float2 mTex : TEXCOORD;
};

struct VertexOut
{
	float4 rPos : SV_POSITION;
	float2 rTex : TEXCOORD;
};