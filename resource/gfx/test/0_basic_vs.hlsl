// @
static float2 positions[3] =
{
	float2(0.0, 0.0),
	float2(0.5, 1.0),
	float2(1.0, 0.5)
};

float4 main(uint vid : SV_VertexID) : SV_Position
{
	return float4(positions[vid], 0.0, 1.0);
}