#pragma once

struct Buffer
{
	VkBuffer buffer;
	VkDeviceMemory memory;
};

struct Image
{
	VkImage image;
	VkDeviceMemory memory;
};

struct Texture
{
	Image texImage;
	VkImageView texImageView;
	VkSampler texSampler;
};

struct FrameBufferTexture
{
	Texture texture;
	VkFormat format;
};

struct FrameBuffer
{
	VkFramebuffer frameBuffer;
	FrameBufferTexture position, normal, albedo, depth;
	VkRenderPass renderPass;
};

struct Light
{
	vec4 pos;
	vec3 col;
	float radius;
};

struct UniformBufferOffscreen
{
	mat4 world, view, proj, inverse/*hlsl wouldnt invert*/;
	float deltaTime;
};

struct UniformBufferFinal
{
	Light lights[4];
	vec4 view;
};

struct PrimData
{
	unsigned int firstIndex = 0;
	unsigned int indexCount = 0;
	unsigned int vertexOffset = 0;
	unsigned int vertexCount = 0;
	int materialIndex = 0;
};