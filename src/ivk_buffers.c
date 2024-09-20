#include "ivk_buffers.h"
#include "ivk_util.h"

#include <string.h>

/********* { pos, pos }, { clr, clr, clr} ***********/
static VkVertexInputBindingDescription vert_2p3c_bind_desc[ IVK_2P3C_BIND_CNT ] =
	{
	[ 0 ].binding = 0,
	[ 0 ].stride = sizeof( ivk_2p3c_type ),
	[ 0 ].inputRate = VK_VERTEX_INPUT_RATE_VERTEX
	};

static VkVertexInputAttributeDescription vert_2p3c_attr_desc[ IVK_2P3C_ATTR_CNT ] = 
	{
	/* Position data */
	[ 0 ].binding = 0,
	[ 0 ].location = 0,
	[ 0 ].format = VK_FORMAT_R32G32_SFLOAT, /* Two elements */
	[ 0 ].offset = offsetof( ivk_2p3c_type, pos ),
	/* Color data */
	[ 1 ].binding = 0,
	[ 1 ].location = 1,
	[ 1 ].format = VK_FORMAT_R32G32B32_SFLOAT, /* Three elements */
	[ 1 ].offset = offsetof( ivk_2p3c_type, clr ),
	};

VkVertexInputBindingDescription* ivk_2p3c_get_bind_desc
	(
	void
	)
{
return &vert_2p3c_bind_desc[ 0 ];
}

VkVertexInputAttributeDescription* ivk_2p3c_get_attr_desc
	(
	void
	)
{
return &vert_2p3c_attr_desc[ 0 ];
}

/*
 * Creates a buffer based on the parameters provided.
 */
static void create_buffer
	(
	VkDevice				device,
	VkPhysicalDevice		gpu,
	VkDeviceSize			size,
	VkBufferUsageFlags		usage,
	VkMemoryPropertyFlags	properties,
	VkBuffer*				buffer,
	VkDeviceMemory*			buffer_memory
	);

/*
 * Records a memory transfer destination between buffers.
 */
static void copy_buffer
	(
	VkDevice		device,
	VkCommandPool	pool,
	VkQueue			queue,
	VkBuffer		dest,
	VkBuffer		src,
	VkDeviceSize	size
	);

/*
 * Returns the appropriate memory type from the GPU.
 */
static unsigned int find_memory_type
	(
	VkPhysicalDevice		gpu,
	unsigned int			type_filter,
	VkMemoryPropertyFlags	properties
	);


/* Vertex buffer create functions */
/* 
 * Creates a vertex function with:
 * - 2 position components ( x, y )
 * - 3 color components ( r, g, b )
 */
void ivk_buffer_create_vbo
	(
	VkDevice			device,
	VkPhysicalDevice	gpu,
	VkCommandPool		pool,
	VkQueue				queue,
	ivk_2p3c_type*		data,
	unsigned int		vert_cnt,
	VkBuffer*			buffer,
	VkDeviceMemory*		buffer_memory
	)
{
/* Local variables */
VkBuffer		_staging_buffer = { 0 };
VkDeviceMemory	_staging_buffer_mem = { 0 };
VkDeviceSize	_size = 0;
void*			_data = NULL;

_size = vert_cnt * sizeof( data[ 0 ] );

/* Create the staging buffer */
create_buffer
	( 
	device, 
	gpu, 
	_size, 
	VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
	VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
	&_staging_buffer, 
	&_staging_buffer_mem 
	);

/* Fill the buffer with the triangle data */
vkMapMemory( device, _staging_buffer_mem, 0, _size, 0, &_data );
memcpy( _data, data, _size );
vkUnmapMemory( device, _staging_buffer_mem );

/* Create the vertex buffer */
create_buffer
	( 
	device, 
	gpu, 
	_size, 
	VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, 
	VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
	buffer, 
	buffer_memory 
	);

/* Copy the data from the staging buffer to the actual vertex buffer */
copy_buffer
	(
	device,
	pool,
	queue,
	*buffer,
	_staging_buffer,
	_size
	);

/* Cleanup */
vkDestroyBuffer( device, _staging_buffer, NULL );
vkFreeMemory( device, _staging_buffer_mem, NULL );

}


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
	)
{
/* Local variables */
VkBuffer		_staging_buffer = { 0 };
VkDeviceMemory	_staging_buffer_mem = { 0 };
VkDeviceSize	_size = 0;
void*			_data = NULL;

_size = idx_cnt * sizeof( data[ 0 ] );

/* Create the staging buffer */
create_buffer
	( 
	device, 
	gpu, 
	_size, 
	VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
	VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
	&_staging_buffer, 
	&_staging_buffer_mem 
	);

/* Fill the buffer with the index data */
vkMapMemory( device, _staging_buffer_mem, 0, _size, 0, &_data );
memcpy( _data, data, _size );
vkUnmapMemory( device, _staging_buffer_mem );

/* Create the index buffer */
create_buffer
	( 
	device, 
	gpu, 
	_size, 
	VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, 
	VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
	buffer, 
	buffer_memory 
	);

/* Copy the data from the staging buffer to the actual vertex buffer */
copy_buffer
	(
	device,
	pool,
	queue,
	*buffer,
	_staging_buffer,
	_size
	);

/* Cleanup */
vkDestroyBuffer( device, _staging_buffer, NULL );
vkFreeMemory( device, _staging_buffer_mem, NULL );
}


/*
 * Creates a buffer based on the parameters provided.
 */
static void create_buffer
	(
	VkDevice				device,
	VkPhysicalDevice		gpu,
	VkDeviceSize			size,
	VkBufferUsageFlags		usage,
	VkMemoryPropertyFlags	properties,
	VkBuffer*				buffer,
	VkDeviceMemory*			buffer_memory
	)
{
/* Local variables */
VkBufferCreateInfo		_buffer_create_info = { 0 };
VkMemoryRequirements	_buffer_mem_requirements = { 0 };
VkMemoryAllocateInfo	_alloc_info = { 0 };
void*					_data = NULL;

_buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
_buffer_create_info.size = size;
_buffer_create_info.usage = usage;
_buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
__vk( vkCreateBuffer( device, &_buffer_create_info, NULL, buffer ) );

/* Get the memory requirements */
vkGetBufferMemoryRequirements( device, *buffer, &_buffer_mem_requirements );

/* Allocate the memory */
_alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
_alloc_info.allocationSize = _buffer_mem_requirements.size;
_alloc_info.memoryTypeIndex = find_memory_type
								( 
								gpu, 
								_buffer_mem_requirements.memoryTypeBits, 
								properties
								);
__vk( vkAllocateMemory
		( 
		device,
		&_alloc_info,
		NULL,
		buffer_memory
		) );

/* Bind the memory to the VBO */
__vk( vkBindBufferMemory( device, *buffer, *buffer_memory, 0 ) );

}


/*
 * Records a memory transfer destination between buffers.
 */
static void copy_buffer
	(
	VkDevice		device,
	VkCommandPool	pool,
	VkQueue			queue,
	VkBuffer		dest,
	VkBuffer		src,
	VkDeviceSize	size
	)
{
/* Local variables */
VkCommandBufferAllocateInfo _alloc_info = { 0 };
VkCommandBufferBeginInfo	_begin_info = { 0 };
VkCommandBuffer				_transfer_command_buffer = { 0 };
VkBufferCopy				_copy_region = { 0 };
VkSubmitInfo				_submit_info = { 0 };

/* Create a command buffer to be used for memory transfers */
_alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
_alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
_alloc_info.commandPool = pool;
_alloc_info.commandBufferCount = 1;
__vk( vkAllocateCommandBuffers( device, &_alloc_info, &_transfer_command_buffer ) );

/* Initialize the command buffer */
_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
_begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
_copy_region.size = size;

/* Initialize the transfer buffer submittal information */
_submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
_submit_info.commandBufferCount = 1;
_submit_info.pCommandBuffers = &_transfer_command_buffer;

/* Record the commands */
vkBeginCommandBuffer( _transfer_command_buffer, &_begin_info );
vkCmdCopyBuffer( _transfer_command_buffer, src, dest, 1, &_copy_region );
vkEndCommandBuffer( _transfer_command_buffer );

/* Submit the command buffer */
vkQueueSubmit( queue, 1, &_submit_info, VK_NULL_HANDLE );
vkQueueWaitIdle( queue );

/* Cleanup */
vkFreeCommandBuffers( device, pool, 1, &_transfer_command_buffer );

}


/*
 * Returns the appropriate memory type from the GPU.
 */
static unsigned int find_memory_type
	(
	VkPhysicalDevice		gpu,
	unsigned int			type_filter,
	VkMemoryPropertyFlags	properties
	)
{
/* Local variables */
VkPhysicalDeviceMemoryProperties _mem_properties = { 0 };

vkGetPhysicalDeviceMemoryProperties( gpu, &_mem_properties );
for( unsigned int i = 0; i < _mem_properties.memoryTypeCount; i++ )
	{
	if( ( type_filter & ( 1 << i ) ) &&
		( ( _mem_properties.memoryTypes[ i ].propertyFlags & properties ) == properties ) )
		{
		return i;
		}
	}
}
