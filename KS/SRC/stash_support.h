/* stash_support.h
 *
 * support functions to prepare and fixup memory images of various types
 */

#ifndef STASH_SUPPORT_HEADER
#define STASH_SUPPORT_HEADER

bool prepare_tga_memory_image(unsigned char *&file_data, unsigned &file_length, 
       unsigned &pal_offset, unsigned short &width, unsigned short &height,
       unsigned char &bit_depth, unsigned char &palettized_flag);

#endif