cbuffer UniformBlock0 : register(b0)
{
	float4x4 mvp;
};

struct VSOutput {
	float4 Position : SV_POSITION;
	float2 TexCoord : TEXCOORD;
};

VSOutput VSMain(float4 Position : POSITION, float2 TexCoord : TEXCOORD0)
{
	VSOutput result;
	result.Position = mul(mvp, Position);
	result.TexCoord = TexCoord;
	return result;
}

Texture2D uTex0 : register(t1);
SamplerState uSampler0 : register(s2);

float4 PSMain(VSOutput input) : SV_TARGET
{
	return uTex0.Sample(uSampler0, input.TexCoord);
}



struct VertexIn
{
	float3 mPos : POSITION;
    float2 mTex : TEXCOORD0;
};

struct VertexOut
{
	float4 rPos : SV_POSITION;
    float2 rTex : TEXCOORD;
};

VertexOut vs_main(VertexIn vIn)
{
    VertexOut vOut;
    vOut.rPos = float4(vIn.mPos, 1.f);
    vOut.rTex = vIn.mTex;

    return vOut;
}

Texture2D gTex : register(t0);
SamplerState gSampler : register(s0);

float4 ps_main(VertexOut vOut) : SV_TARGET
{
   return gTex.Sample(gSampler, vOut.rTex);
}