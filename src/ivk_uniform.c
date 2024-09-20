#include "ivk_uniform.h"

/********* MVP matrices ***********/
static VkDescriptorSetLayoutBinding ubo_mvp_layout_binding =
	{
	.binding = 0,
	.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
	.descriptorCount = 1,
	.stageFlags = VK_SHADER_STAGE_VERTEX_BIT
	};

VkDescriptorSetLayoutBinding* ivk_ubo_mvp_get_binding
	(
	void
	)
{
return &ubo_mvp_layout_binding;
}