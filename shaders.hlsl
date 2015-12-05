
cbuffer cbPerObject : register(b0)
{
	float4x4 World;
	float4x4 View;
	float4x4 Projection;

	float4x4 InverseView;
	float4x4 InverseProjection;
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


Texture3D	 FrontTexture      : register (t1);
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
	float4 Size;
}

float LengthSqr(float4 pt)
{
	return dot (pt.xyz, pt.xyz);
}

float4 Transform(float4 pt)
{
	float4 result = mul(pt, InverseProjection);
	result = mul(result, InverseView);
	result = mul(result, InverseWorld);
	result /= result.w;

	return result;

}

float4 PShaderRM(PS_INPUT_ pos) : SV_TARGET
{
	float4 near = {pos.pos.x,
				   pos.pos.y,
				   0.0f, 1.0f};
	float4 far = { pos.pos.x,
				   pos.pos.y,
				   1.0f, 1.0f };

	near = Transform(near);
	far  = Transform(far);

	float4 dir = normalize(far - near);

	float b = 2.0f * dot(near, dir);
	float c = 4.0f * LengthSqr(near) - LengthSqr(Size);

	float D = (b*b - c);

	clip (D);
	float sqrtDforth = sqrt(D);

	float t = 2.0f*(-b - sqrtDforth);
	float shift = 0.0f;

	float3 current = near.xyz + t*dir.xyz;
	near += t*dir;

	float4 color = {0.0f, 0.0f, 0.0f, 0.0f};
	float4 newColor = color;
	
	float iterations = 4.0f*sqrtDforth / 0.1f;

	float coeffSource = 9.0f;
	float coeffNew = 1.0f;

	coeffSource /= coeffSource + coeffNew;
	coeffNew    /= coeffSource + coeffNew;
	float i = 0;
	while (i < iterations)
	{
		//newColor = FrontTexture.Sample(FrontSamplerState, float3 (0.0f, 0.0f, 0.0f));
		//color = color*coeffSource + newColor*coeffNew;
		current += dir*0.1f;
		i += 1.0f;
	}
	color = float4(iterations / (50*length(Size)), 0.3f, 0.4f, 0.5f);
	return color;
}