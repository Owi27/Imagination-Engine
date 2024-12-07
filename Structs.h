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

struct UniformBufferOffscreen
{
	mat4 world, view, proj;
	float deltaTime;
};

struct UniformBufferFinal
{
	vec4 view;
};