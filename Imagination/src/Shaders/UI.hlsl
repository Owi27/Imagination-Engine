//in-game retained ui. orthographic pixel space, top-left origin.
//keep this file in sync with Imagination/src/Imgn/UI/UIShader.h
//compiled twice: vertex as-is, pixel with UI_STAGE_PIXEL defined.
//scene color is a push-descriptor sampled image (t2). widget textures
//index the existing streaming combined-sampler heap (t0, space1).
//do not write this pass into TAA history.

struct UIQuad
{
	float4 rect;
	float4 uv;
	float4 color;
	float4 extra;
};

StructuredBuffer<UIQuad> quads : register(t1, space0);

[[vk::push_constant]]
struct UIPC
{
	float2 targetSize;
	uint firstQuad;
	uint pad;
} pc;

struct VSOut
{
	float4 pos : SV_Position;
	float2 uv : TEXCOORD0;
	float4 color : COLOR0;
	float4 extra : TEXCOORD1;
};

#ifndef UI_STAGE_PIXEL
VSOut VSMain(uint vertexID : SV_VertexID, uint instanceID : SV_InstanceID)
{
	VSOut output;
	UIQuad quad = quads[pc.firstQuad + instanceID];

	float2 corners[6] =
	{
		float2(0.f, 0.f), float2(1.f, 0.f), float2(1.f, 1.f),
		float2(0.f, 0.f), float2(1.f, 1.f), float2(0.f, 1.f)
	};

	float2 corner = corners[vertexID];
	float2 local = (corner - 0.5f) * quad.rect.zw;
	float rotation = quad.extra.z;
	float sine, cosine;
	sincos(rotation, sine, cosine);
	float2 rotated = float2(local.x * cosine - local.y * sine, local.x * sine + local.y * cosine);
	float2 pixel = quad.rect.xy + quad.rect.zw * 0.5f + rotated;

	float2 ndc = pixel / pc.targetSize * 2.f - 1.f;
	output.pos = float4(ndc, 0.f, 1.f);
	output.uv = lerp(quad.uv.xy, quad.uv.zw, corner);
	output.color = quad.color;
	output.extra = quad.extra;
	return output;
}
#else
Texture2D sceneColor : register(t2, space0);
SamplerState sceneSampler : register(s4, space0);
//same pairing as the gbuffer shader: t0 + s0 in space1 is one combined sampler
Texture2D uiTextures[] : register(t0, space1);
SamplerState uiHeapSampler : register(s0, space1);

float4 PSMain(VSOut input) : SV_Target
{
	//extra.y == 1 samples the post-TAA scene. extra.x < 0 is a solid color.
	if (input.extra.y > 0.5f)
		return sceneColor.Sample(sceneSampler, input.uv) * input.color;

	if (input.extra.x < 0.f)
		return input.color;

	uint textureIndex = (uint)input.extra.x;
	float4 sampled = uiTextures[NonUniformResourceIndex(textureIndex)].Sample(uiHeapSampler, input.uv);
	return sampled * input.color;
}
#endif
