#include "pch.h"
#include "FGCore.h"

FGTextureCreateInfo FGTextureCreateInfo::AsColorAttachment(const std::string& name, VkExtent2D extent, VkFormat format, VkSampleCountFlagBits samples)
{
    FGTextureCreateInfo info;
    info.name = name;
    info.extent = { extent.width, extent.height, 1 };
    info.format = format;
    info.samples = samples;
    // Your Texture constructor for depth/color implies usage, let's try to match:
    info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    info.aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
    info.imageTiling = VK_IMAGE_TILING_OPTIMAL;
    info.memoryPropertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    return info;
}

FGTextureCreateInfo FGTextureCreateInfo::AsDepthAttachment(const std::string& name, VkExtent2D extent, VkFormat format, VkSampleCountFlagBits samples)
{
    FGTextureCreateInfo info;
    info.name = name;
    info.extent = { extent.width, extent.height, 1 };
    info.format = format;
    info.samples = samples;
    info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    info.aspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT;
    if (format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT || format == VK_FORMAT_D16_UNORM_S8_UINT) {
        info.aspectFlags |= VK_IMAGE_ASPECT_STENCIL_BIT;
    }
    info.imageTiling = VK_IMAGE_TILING_OPTIMAL;
    info.memoryPropertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    return info;
}
