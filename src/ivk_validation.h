#pragma once
#include <stdbool.h>

#if defined( _DEBUG )
    #define VALIDATION_ENABLED  1
#else
    #define VALIDATION_ENABLED  0
#endif 
/*
 * Checks whether the validation layers are supported
 */
bool ivk_check_validation_layer_support
	(
	void
	);
