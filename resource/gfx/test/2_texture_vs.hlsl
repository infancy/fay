#include "test_render_struct.hlsli"

// @
VertexOut main(VertexIn vIn)
{
    VertexOut vOut;
    vOut.rPos = float4(vIn.mPos, 1.f);
    vOut.rTex = vIn.mTex;

    return vOut;
}