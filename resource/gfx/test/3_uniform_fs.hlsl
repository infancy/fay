#include "test_render_struct.hlsli"

Texture2D gTex : register(t0);
SamplerState gSampler : register(s0);

cbuffer params : register(b0)
{   
    float4 window;
    int flag;
};

// @
float4 main(VertexOut vOut) : SV_TARGET
{
    float4 color;

   if(flag == 1)
        color = gTex.Sample(gSampler, vOut.rTex);
    else
    {
        float y = vOut.rPos.y / window.w;
        color = float4(y, y, y, 1.f);
    }

    if((vOut.rPos.x < window.z / 2) && (vOut.rPos.y < window.w / 2))
        color = float4(1.0, 0.0, 0.0, 1.f);

    return color;
}