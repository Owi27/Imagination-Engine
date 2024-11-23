#pragma once

struct Buffer
{
	VkBuffer buffer;
	VkDeviceMemory memory;
};

struct Texture
{
	Buffer texBuffer;
	VkImage texImage;
	VkImageView texImageView;
	VkSampler texSampler;
};

struct Matrices
{
	mat4 prevWorldViewProjection, currWorldViewProjection;
	float deltaTime;
};