#pragma once
#include "vulkan/vulkan.h"
#include "cglm/cglm.h"

/*
 * Types
 */
typedef struct
	{
	mat4 model;
	mat4 view;
	mat4 proj;
	} ivk_mvp_type;

VkDescriptorSetLayoutBinding* ivk_ubo_mvp_get_binding
	(
	void
	);