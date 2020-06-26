float4 vs_main(float4 Position : POSITION) : SV_POSITION
{
	return Position;
}

float4 ps_main(float4 Position : SV_POSITION) : SV_TARGET
{
	return float4(1, 0, 0, 1);
}