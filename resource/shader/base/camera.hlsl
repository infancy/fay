cbuffer UniformBlock0 : register(b0)
{
	float4x4 mvp;
};

struct VertexIn
{
	float3 mPos : POSITION;
	float3 mNor : NORMAL;
    float2 mTex : TEXCOORD;
    float3 mTan : TANGENT;
    float3 mBit : BITANGENT;
};

struct VertexOut
{
	float4 rPos : SV_POSITION;
    float3 rNor : NORMAL;
    float2 rTex : TEXCOORD;
};

VertexOut vs_main(VertexIn vIn)
{
    VertexOut vOut;
    vOut.rPos = mul(mvp, float4(vIn.mPos, 1.f));
    vOut.rTex = vIn.mTex;

    return vOut;
}

SamplerState gSampler : register(s0);
Texture2D gTex : register(t0);

float4 ps_main(VertexOut vOut) : SV_TARGET
{
    return gTex.Sample(gSampler, vOut.rTex);
}