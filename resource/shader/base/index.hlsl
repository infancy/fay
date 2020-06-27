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

VertexOut vs_main(VertexIn vIn)
{
    VertexOut vOut;
    vOut.rPos = float4(vIn.mPos, 1.f);
    vOut.rTex = vIn.mTex;

    return vOut;
}

float4 ps_main(VertexOut vOut) : SV_TARGET
{
   return float4(vOut.rTex.x, vOut.rTex.y, 0.f, 1.f);
   
   //return float4(0.f, 0.f, , 1.f);
}