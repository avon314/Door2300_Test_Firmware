/*****************************************************************************
*                       Copyright (c)
*              Avon Building Solutions Pvt Ltd
*                   All RIGTHS RESERVED.
*
* Module Name	: tls_cfg_guard.c
* Description	: Compiled into the mbedtls41 LIBRARY project (not the app).
*				  It defines a symbol whose name encodes the build configuration
*				  of the library. tls_client.c (in the app) references the symbol
*				  name matching ITS configuration, so linking a Debug app against a
*				  Release library (or the reverse) fails at link time instead of
*				  corrupting memory through different structure layouts.
*****************************************************************************/
#include "mbedtls/build_info.h"
#include "tls_cfg_guard.h"

const int MM_TLS_CFG_GUARD_SYMBOL = 1;
