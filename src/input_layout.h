#pragma once
#include <vulkan/vulkan.h>
namespace fgvk {
inline bool ReadableLayout(VkImageLayout l){
 return l==VK_IMAGE_LAYOUT_GENERAL || l==VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL ||
  l==VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL || l==VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL ||
  l==VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL;
}
// The bridge transitions and copies the source, then restores its observed
// layout. A source already in TRANSFER_SRC is valid for that operation, while
// native DLSS-G tags continue to require a shader-readable layout.
inline bool FGInputLayout(VkImageLayout layout,VkImageUsageFlags usage,bool copyTransport){
 if(!copyTransport)return ReadableLayout(layout);
 return (usage&VK_IMAGE_USAGE_TRANSFER_SRC_BIT) &&
  (ReadableLayout(layout) || layout==VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
}
}
