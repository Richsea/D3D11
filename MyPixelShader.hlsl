struct PIXEL_IN {
	float4 pos : SV_POSITION;
	float4 color : COLOR;
};

float4 main(PIXEL_IN inp) : SV_TARGET{
	return inp.color;
}