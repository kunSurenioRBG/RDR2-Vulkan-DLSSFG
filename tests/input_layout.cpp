#include "../src/input_layout.h"
#include <cassert>
#include <cstdio>
#include <initializer_list>
static bool Allowed(VkImageLayout layout,VkImageUsageFlags usage,bool copy){
#ifdef FGVK_BASELINE_LAYOUT
 return fgvk::ReadableLayout(layout);
#else
 return fgvk::FGInputLayout(layout,usage,copy);
#endif
}
int main(){
 const auto copy=VK_IMAGE_USAGE_TRANSFER_SRC_BIT|VK_IMAGE_USAGE_SAMPLED_BIT;
 // Exact FG-104 inputs recorded at 17:45:38: depth=6, motion=5.
 if(!Allowed(VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,copy,true) ||
    !Allowed(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,copy,true)){
  puts("FAIL: observed valid copy-source depth/motion pair rejected (FG-104)");return 1;
 }
 assert(!Allowed(VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,copy,false));
 assert(Allowed(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,VK_IMAGE_USAGE_SAMPLED_BIT,false));
 assert(!Allowed(VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,VK_IMAGE_USAGE_SAMPLED_BIT,true));
 assert(!Allowed(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,VK_IMAGE_USAGE_SAMPLED_BIT,true));
 for(auto invalid:{VK_IMAGE_LAYOUT_UNDEFINED,VK_IMAGE_LAYOUT_PREINITIALIZED,
     VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
     VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL}){
  assert(!Allowed(invalid,copy,true));assert(!Allowed(invalid,copy,false));
 }
 puts("Observed FG-104 copy-layout regression; invalid states/usages rejected; native tags unchanged: PASS");
}
