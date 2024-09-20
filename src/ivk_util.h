#pragma once
#include "cglm/cglm.h"

/* 
 * Debug macros
 */
#if defined( _DEBUG )
    #define __vk( stmt ) do {               \
            VkResult result = 0;            \
            result = stmt;                  \
            if( result != VK_SUCCESS )      \
                {                           \
                printf( "%s failed in %s at line %d", #stmt, __FILE__, __LINE__ ); \
                }                           \
        } while( 0 )
#else
    #define __vk( stmt )    stmt
#endif

