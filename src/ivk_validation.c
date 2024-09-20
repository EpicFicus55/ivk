#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <vulkan/vulkan.h>

#include "ivk_util.h"
#include "ivk_validation.h"

/* Validation layers */
const char* g_validation_layers[] =
    {
    "VK_LAYER_KHRONOS_validation"
    };
const unsigned int g_validation_layer_cnt = 1;

/*
 * Checks whether the validation layers are supported
 */
bool ivk_check_validation_layer_support
    (
    void
    )
{
/* Local variables */
unsigned int        _layer_count = 0;
VkLayerProperties*  _supported_layers = NULL;

/* Query the supported instance validation layers */
__vk( vkEnumerateInstanceLayerProperties( &_layer_count, NULL ) );
_supported_layers = ( VkLayerProperties* )calloc( _layer_count, sizeof( VkLayerProperties ) );
__vk( vkEnumerateInstanceLayerProperties( &_layer_count, _supported_layers ) );

/* Check each of the necessary validation layers */
for( size_t i = 0; i < g_validation_layer_cnt; i++ )
    {
    bool _is_layer_supported = false;
    for( size_t j = 0; j < _layer_count; j++ )
        {
        if( strcmp( g_validation_layers[ i ], _supported_layers[ j ].layerName ) == 0 )
            {
            _is_layer_supported = true;
            break;
            }
        }
    if( !_is_layer_supported )
        {
        printf( "Validation layer %s not supported.\n", g_validation_layers[ i ] );
        free( _supported_layers );
        return( false );
        }
    }

free( _supported_layers );
return( true );

}