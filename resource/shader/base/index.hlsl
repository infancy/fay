struct VertexIn
{
	float3 mPos : POSITION;
	float3 mNor : NORMAL;
};

struct VertexOut
{
	float4 rPos : SV_POSITION;
    float3 rNor : NORMAL;
};

VertexOut vs_main(VertexIn vIn)
{
    VertexOut vOut;
    vOut.rPos = float4(vIn.mPos, 1.f);
    vOut.rNor = vIn.mNor;

    return vOut;
}

float4 ps_main(VertexOut vOut) : SV_TARGET
{
   return float4(vOut.rNor.x, vOut.rNor.y, vOut.rNor.z, 1.f);
   
   //return float4(0.f, 0.f, , 1.f);
}