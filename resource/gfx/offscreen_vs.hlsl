#include "render_struct.hlsli"

cbuffer MVP : register(b0)
{
    matrix mvp;
};

#include "render_func.hlsli"

/*
cbuffer vp : register(b0) 
{
    matrix vp;
};

cbuffer model : register(b0) 
{
    matrix model;
};
*/

VertexOut main(VertexIn vIn)
{
    VertexOut vOut;
    //matrix mvp = vp * model;
    vOut.rPos = mul(mvp, float4(vIn.mPos, 1.f));
    vOut.rTex = vIn.mTex;

    return vOut;
}