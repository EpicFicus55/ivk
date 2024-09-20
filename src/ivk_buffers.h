#pragma once
#include "vulkan/vulkan.h"
#include "cglm/cglm.h"

/* 
 * Vertex format bind / attribute counts
 */
#define IVK_2P3C_BIND_CNT   1
#define IVK_2P3C_ATTR_CNT   2

/*
 * Types
 */
typedef struct
    {
    vec2    pos;
    vec3    clr;
    } ivk_2p3c_type;


/********* { pos, pos }, { clr, clr, clr} ***********/
VkVertexInputBindingDescription* ivk_2p3c_get_bind_desc
    (
    void
    );

VkVertexInputAttributeDescription* ivk_2p3c_get_attr_desc
    (
    void
    );


/* Buffer creation functions */
/* 
 * Creates a vertex buffer with:
 * - 2 position components ( x, y )
 * - 3 color components ( r, g, b )
 */
void ivk_buffer_create_vbo
    (
    VkDevice            device,
    VkPhysicalDevice	gpu,
    VkCommandPool		pool,
	VkQueue				queue,
    ivk_2p3c_type*      data,
    unsigned int        vert_cnt,
    VkBuffer*           buffer,
    VkDeviceMemory*		buffer_memory
    );

/* 
 * Creates an index buffer
 */
void ivk_buffer_create_ibo
    (
    VkDevice            device,
    VkPhysicalDevice	gpu,
    VkCommandPool		pool,
	VkQueue				queue,
    unsigned int*       data,
    unsigned int        idx_cnt,
    VkBuffer*           buffer,
    VkDeviceMemory*		buffer_memory
    );
