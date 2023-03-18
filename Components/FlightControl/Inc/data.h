#pragma once

/* Structs containing data primitives */

/*
 * IMPORTANT NOTE:
 *  Despite being typed as int32 or uint32 these are actually not integers.
 *  They represent fixed point decimal numbers.
 *
 * The specified precision is not consistent across all instruments,
 * please see the design manual for more information.
 */


typedef struct
{
    osMutexId   mutex_;
    int32_t     pressure_;
} ServoData;

typedef struct
{
    ServoData* pressureTransducerData_;
} AllData;
