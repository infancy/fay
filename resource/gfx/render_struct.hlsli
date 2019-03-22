struct VertexIn
{
	float3 mPos : POSITION;
	float3 mNor : NORMAL;
    float2 mTex : TEXTURECOORD0;
    float3 mTan : TANGENT;
    float3 mBit : BITANGENT;
};

struct VertexOut
{
	float4 rPos : SV_POSITION;
	float4 rNor : NORMAL;
};