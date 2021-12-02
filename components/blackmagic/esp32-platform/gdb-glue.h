#pragma once
#include <stdlib.h>
#include <stdint.h>

/**
 * Init gdb stream glue
 */
void gdb_glue_init(void);

/**
 * Get free size of rx stream
 * @return size_t 
 */
size_t gdb_glue_get_free_size(void);

/**
 * Put data to rx stream
 * @param buffer data
 * @param size data size
 */
void gdb_glue_receive(uint8_t* buffer, size_t size);

/**
 * 
 * @return bool 
 */
bool gdb_glue_can_receive();

/**
 * Get gdb packet size
 * @return size_t 
 */
size_t gdb_glue_get_packet_size();

/**
 * Get blackmagic version
 * @return const char* 
 */
const char* gdb_glue_get_bm_version();