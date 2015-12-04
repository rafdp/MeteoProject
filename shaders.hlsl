
cbuffer cbPerObject : register(b0)
{
	float4x4 World;
	float4x4 View;
	float4x4 Projection;

	float4x4 InverseView;
};

cbuffer Cam : register(b1)
{
	float4 CamPos;
	float4 CamDir;
}

struct GS_INPUT
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
	float4 worldPos: POSITION;
};

struct PS_INPUT
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
};


GS_INPUT VShader (float4 inPos : POSITION, float4 inColor : COLOR)
{
	GS_INPUT output;
	output.worldPos = mul (inPos, World);
	output.position = mul (output.worldPos, View);
	output.position = mul (output.position, Projection);
	output.color = inColor;
	return output;
}

float4 PShader (PS_INPUT input) : SV_TARGET
{
	return input.color;
}

float4 main () : SV_TARGET
{
	return float4(1.0f, 1.0f, 1.0f, 1.0f);
}

[maxvertexcount (6)]
void GShader (point GS_INPUT input[1],
			  inout TriangleStream<PS_INPUT> OutputStream)
{
	PS_INPUT output[6];

	float3 normal = normalize (CamPos.xyz - input[0].worldPos.xyz);
	float zPerpendicular = -normal.y / normal.z;
	float3 upAxis = normalize (float3 (0.0f, 1.0f, zPerpendicular));
	float3 rightAxis = normalize (cross (normal, upAxis));

	float d = 0.001f;

	float4x4 VP = View*Projection;

	for (int i = 0; i < 6; i++)
		output[i].color = input[0].color;
	output[1].position = mul (float4 (input[0].worldPos.xyz + (upAxis - rightAxis) * d, 1.0f), VP);
	output[0].position = mul (float4 (input[0].worldPos.xyz + (upAxis + rightAxis) * d, 1.0f), VP);
	output[2].position = mul (float4 (input[0].worldPos.xyz + (-upAxis + rightAxis) * d, 1.0f), VP);

	output[3].position = mul (float4 (input[0].worldPos.xyz + (upAxis - rightAxis) * d, 1.0f), VP);
	output[4].position = mul (float4 (input[0].worldPos.xyz + (-upAxis - rightAxis) * d, 1.0f), VP);
	output[5].position = mul (float4 (input[0].worldPos.xyz + (-upAxis + rightAxis) * d, 1.0f), VP);
	
	OutputStream.Append (output[0]);
	OutputStream.Append (output[1]);
	OutputStream.Append (output[2]);
	OutputStream.RestartStrip (); 
	OutputStream.Append (output[3]);
	OutputStream.Append (output[4]);
	OutputStream.Append (output[5]);
}

[maxvertexcount (6)]
void GShaderShuttle (point GS_INPUT input[1],
			  inout TriangleStream<PS_INPUT> OutputStream)
{
	PS_INPUT output[6];

	float3 normal = normalize (CamPos.xyz - input[0].worldPos.xyz);
	float zPerpendicular = -normal.y / normal.z;
	float3 upAxis = normalize (float3 (0.0f, 1.0f, zPerpendicular));
	float3 rightAxis = normalize (cross (normal, upAxis));

	float d = 0.0015;
	float4x4 VP = View*Projection;

	for (int i = 0; i < 6; i++)
		output[i].color = input[0].color;
	output[1].position = mul (float4 (input[0].worldPos.xyz + (upAxis - rightAxis) * d, 1.0f), VP);
	output[0].position = mul (float4 (input[0].worldPos.xyz + (upAxis + rightAxis) * d, 1.0f), VP);
	output[2].position = mul (float4 (input[0].worldPos.xyz + (-upAxis + rightAxis) * d, 1.0f), VP);

	output[3].position = mul (float4 (input[0].worldPos.xyz + (upAxis - rightAxis) * d, 1.0f), VP);
	output[4].position = mul (float4 (input[0].worldPos.xyz + (-upAxis - rightAxis) * d, 1.0f), VP);
	output[5].position = mul (float4 (input[0].worldPos.xyz + (-upAxis + rightAxis) * d, 1.0f), VP);

	OutputStream.Append (output[0]);
	OutputStream.Append (output[1]);
	OutputStream.Append (output[2]);
	OutputStream.RestartStrip ();
	OutputStream.Append (output[3]);
	OutputStream.Append (output[4]);
	OutputStream.Append (output[5]);
}


Texture2D	 FrontTexture    : register (t1);
SamplerState FrontSamplerState : register (s1);


struct PS_INPUT_
{
	float4 position : SV_POSITION;
	float4 pos : TEXCOORD0;
};

PS_INPUT_ VShaderRM(float4 pos : POSITION, float4 color : COLOR)
{
	PS_INPUT_ out_ = {pos, pos};
	return out_;
}

cbuffer Fronts : register(b2)
{
	float4x4 InverseWorld;
}


float4 PShaderRM(PS_INPUT_ pos) : SV_TARGET
{
	float4 near = {pos.pos.x / Projection[0][0],
				   pos.pos.y / Projection[1][1],
				   0.0f, 1.0f};
	float4 far = { pos.pos.x / Projection[0][0],
				   pos.pos.y / Projection[1][1],
				   1.0f, 1.0f };

	near = mul(near, InverseView);
	near = mul(near, InverseWorld);
	far  = mul(far,  InverseView);
	far  = mul(far,  InverseWorld);

	float4 dir = normalize (far - near);

	const int minPoints = 16;

	float4 d = (far - near) / minPoints;

	for (int i = 0; i < minPoints; i++)
	{
		if 
	}




	return float4 (cos (3.14/2.0f*pos.pos.x), sin(3.14 / 2.0f*pos.pos.y), 0.0f, 0.5f);
}