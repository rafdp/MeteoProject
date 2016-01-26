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
	float  Step;
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
	float4 worldPos: POSITION;
};

struct PS_INPUT1
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

float4 PShader (PS_INPUT1 input) : SV_TARGET
{
	return input.color;
}

float4 main () : SV_TARGET
{
	return float4(1.0f, 1.0f, 1.0f, 1.0f);
}


[maxvertexcount (6)]
void GShaderShuttle (point GS_INPUT input[1],
			  inout TriangleStream<PS_INPUT1> OutputStream)
{
	PS_INPUT1 output[6];

	float3 normal = normalize(CamPos.xyz - input[0].worldPos.xyz);
	float zPerpendicular = -normal.y / normal.z;
	float3 upAxis = normalize(float3 (0.0f, 1.0f, zPerpendicular));
	float3 rightAxis = normalize(cross(normal, upAxis));

	float d = 0.0015;
	float4x4 VP = View*Projection;

	for (int i = 0; i < 6; i++)
		output[i].color = input[0].color;
	output[1].position = mul(mul (float4 (input[0].worldPos.xyz + (upAxis - rightAxis) * d, 1.0f), View), Projection);
	output[0].position = mul(mul (float4 (input[0].worldPos.xyz + (upAxis + rightAxis) * d, 1.0f), View), Projection);
	output[2].position = mul(mul (float4 (input[0].worldPos.xyz + (-upAxis + rightAxis) * d, 1.0f), View), Projection);

	output[3].position = mul(mul (float4 (input[0].worldPos.xyz + (upAxis - rightAxis) * d, 1.0f), View), Projection);
	output[4].position = mul(mul (float4 (input[0].worldPos.xyz + (-upAxis - rightAxis) * d, 1.0f), View), Projection);
	output[5].position = mul(mul (float4 (input[0].worldPos.xyz + (-upAxis + rightAxis) * d, 1.0f), View), Projection);

	OutputStream.Append(output[1]);
	OutputStream.Append(output[0]);
	OutputStream.Append(output[2]);
	OutputStream.RestartStrip();
	OutputStream.Append(output[4]);
	OutputStream.Append(output[3]);
	OutputStream.Append(output[5]);
}


SamplerState FrontSamplerState : register (s2);
Texture3D	 FrontTexture      : register (t2);

SamplerState NoiseSamplerState : register (s1);
Texture2D	 NoiseTexture      : register (t1);


struct PS_INPUT_
{
	float4 position : SV_POSITION;
	float4 worldPos : POSITION;
};

PS_INPUT_ VShaderRM(float4 pos : POSITION, float4 color : COLOR)
{
	PS_INPUT_ out_;
	out_.worldPos = mul(pos, World);
	out_.position = mul(out_.worldPos, View);
	out_.position = mul(out_.position, Projection);
	return out_;
}

cbuffer Fronts : register(b2)
{
	float4x4 InverseWorld;
	float4 Size;
	float Shuttle;
}

float LengthSqr(float4 pt)
{
	return dot (pt.xyz, pt.xyz);
}

float LengthSqr3(float3 pt)
{
	return dot(pt, pt);
}
float4 Transform(float4 pt)
{
	float4 result = mul(pt, InverseProjection);
	result = mul(result, InverseView);
	result = mul(result, InverseWorld);
	result /= result.w;

	return result;

}

float4 SampleTexture(float3 pos)
{
	return  FrontTexture.Sample(FrontSamplerState, pos);
}

float4 GetRMColorAdding(float4 end)
{
	//const float step = 0.001f;

	float4 dir = mul(normalize(end - CamPos), InverseWorld);
	float s = NoiseTexture.Sample(NoiseSamplerState, 17.0f*dir.xz);

	float3 current = mul(end, InverseWorld) + dir * Step * s * 0.5f;
	float4 colorAdding = {0.0f, 0.0f, 0.0f, 0.0f};
	float newColor = 0.0f;


	float coeffSource = 0.6f;
	float coeffNew = 35.0f * Step;
	
	int iterations = 0;
	if (LengthSqr(Size) > LengthSqr(CamPos - end)) 
		iterations = sqrt(LengthSqr(CamPos - end)) / Step;
	else
		iterations = sqrt(LengthSqr(Size)) / Step;

	current -= dir*iterations*Step;

	int iterationsN = iterations;

	if (Shuttle > 0.0f) iterationsN /= 2;

	float k = 0.1f;

	bool inside = false;


	for (int i = 0; i < iterationsN; i++)
	{
		newColor = FrontTexture.SampleLevel(FrontSamplerState,
			float3 (current.x / Size.x + 0.5f,
					current.z / Size.y + 0.5f,
					current.y / Size.z + 0.5f), 0);
		current += dir*Step * (inside ? 0.5f : 1.0f);
		k += 1.0f / iterations;
		if (newColor < 0.1f) 
		{ 
			inside = false; 
			continue; 
		}
		if (!inside) {inside = true;}
		if (newColor)
		{
			if (newColor < 1.5f)
			{
				colorAdding += float4 (0.4f + 0.6f *      newColor,
					0.4f - 0.259375f * newColor,
					0.4f - 0.4f *      newColor,
					newColor) * coeffNew;
				/*if (newColor < 0.4f) colorAdding += float4 (0.5f, 0.5f, 0.5f, 1.0f);
				else
				if (newColor < 0.8f) colorAdding += float4 (0.75f, 0.3203125f, 0.25f, 1.0f);
				else
					colorAdding += float4 (1.0f, 0.140625f, 0.0f, 1.0f);*/
			}
				
			else
			if (newColor > 2.0f)
			{
				float d = 0.15625f * (newColor - 2.0f);
				colorAdding += float4 (0.703125f   - d, 
									   0.44921875f - d, 
									   0.78125f    - d, 
									   (newColor - 2.0f)) * coeffNew;
			}
				
		}
		//colorAdding += newColor*coeffNew;
		/*if (current.x / Size.x > 0.5f || current.x / Size.x < -0.5f ||
			current.z / Size.y > 0.5f || current.z / Size.y < -0.5f ||
			current.y / Size.z > 0.5f || current.y / Size.z < -0.5f) break;*/
	}
	return colorAdding;
}


float4 PShaderRM(PS_INPUT_ pos) : SV_TARGET
{
	//return float4 (1.0f, 1.0f, 1.0f, 0.5f);
	return GetRMColorAdding (pos.worldPos);
}

float4 PShaderRM_Map(PS_INPUT input) : SV_TARGET
{
	//Slow as hell, need to find workaround
	return input.color/* + GetRMColorAdding (input.worldPos)*/;
}                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  