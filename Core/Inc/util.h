#ifndef _UTIL_H
#define _UTIL_H

#include "defs.h"

//-----------------------------------------------------------------------------
//
// Firmware Module Description
// ...
//
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//
// External Definitions
//
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//
// Interface Function Prototypes
//
//-----------------------------------------------------------------------------

void util_memcpy(UINT8* dest, UINT8* src, UINT32 len);
void util_memset(UINT8* dest, UINT8 val, UINT32 len);
UINT32 util_memcmp(UINT8* loc1, UINT8* loc2, UINT32 len);
INT32 util_max(INT32 num1, INT32 num2);
INT32 util_min(INT32 num1, INT32 num2);
INT32 util_bound(INT32 num, INT32 min, INT32 max);
INT32 util_linear(INT32 x, INT32 x0, INT32 x1, INT32 y0, INT32 y1);
UINT16 util_read_le16( UINT8 *p );
UINT32 util_read_le32( UINT8 *p );
void util_write_le16( UINT8 *p, UINT16 val );
void util_write_le32( UINT8 *p, UINT32 val );
UINT16 util_read_be16( UINT8 *p );
UINT32 util_read_be24( UINT8 *p );
UINT32 util_read_be32( UINT8 *p );
void util_write_be16( UINT8 *p, UINT16 val );
void util_write_be24( UINT8 *p, UINT32 val );
void util_write_be32( UINT8 *p, UINT32 val );
void util_init( void );
void util_main( void );


#endif
