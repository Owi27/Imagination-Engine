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
	FrameBufferTexture position, normal, albedo, velocity, depth;
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
	mat4 world, view, proj;
	float deltaTime;
	mat4 prev, curr;
};

struct UniformBufferFinal
{
	Light lights[4];
	vec4 view;
};