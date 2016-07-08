struct VERTEX_IN {
	float3 pos : POSITION;	//����� ����
	float4 color : COLOR;
};

struct PIXEL_IN {
	float4 pos : SV_POSITION;	//��� + 0 or 1 ���ڸ��� �߰��Ǿ� ������ �������� �����Ѵ�
	float4 color : COLOR;
};

cbuffer MyConstantBuffer : register (b0)
{
	float4x4 world;
	float4x4 view;
	float4x4 proj;
};

PIXEL_IN main(VERTEX_IN inp) {
	PIXEL_IN ret;
	ret.color = inp.color;
	ret.pos = float4 (inp.pos, 1);
	ret.pos = mul(ret.pos, world);
	ret.pos = mul(ret.pos, view);
	ret.pos = mul(ret.pos, proj);
	return ret;
}