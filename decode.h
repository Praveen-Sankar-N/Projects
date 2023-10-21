#ifndef DECODE_H
#define DECODE_H

#include "types.h"

#define MAX_SECRET_BUF_SIZE 1
#define MAX_IMAGE_BUF_SIZE (MAX_SECRET_BUF_SIZE * 8)
#define MAX_FILE_SUFFIX 4
#define MAGIC_STRING_SIZE 2
/* 
 * Structure to store information required for
 * decoding secret file to source Image
 * Info about output and intermediate data is
 * also stored
 */

typedef struct _DecodeInfo
{
    /*Stego file info*/
    char *stego_file_name;
    FILE *fptr_stego;
    char stego_image_buffer[MAX_IMAGE_BUF_SIZE];
    uint secret_file_extn_size;
    char *secret_file_extn_data;
    uint secret_file_size;
    char *secret_file_data;

    /*Decoded file output info*/
    char decoded_file_name[15];
    FILE *fptr_decode_file;
    char dec_magic_str[MAGIC_STRING_SIZE + 1]; 

}DecodeInfo;


// Validating the args passed while decoding
Status read_and_validate_decode(char *argv[],DecodeInfo *decInfo);

// Validating the file existence and open the file
Status open_stego_file_decode(DecodeInfo *decInfo);

// Do decoding every information
Status do_decoding(DecodeInfo *decInfo);

// Deocding magic string
Status decode_magic_string(DecodeInfo *decInfo);

// Decoding secret file extension size
Status decode_secret_file_extn_size(DecodeInfo *decInfo);

// Decoding secret file extension data
Status decode_secret_file_extn(DecodeInfo *decInfo);

// Opening the decode.txt output file
Status open_decode_file(DecodeInfo *decInfo);

// Decode secret file size
Status decode_secret_file_size(DecodeInfo *decInfo);

// Decode secret file data
Status decode_secret_file_data(DecodeInfo *decInfo);

// Decode every byte from lsb
Status decode_byte_from_lsb1(char *decoding_data, int size, FILE *fptr_stego, char *image_buffer_stego, DecodeInfo *decInfo);

// Decode every byte from lsb and returning that character to lsb1 function for stroing
char decode_byte_form_lsb2(DecodeInfo *decInfo, char *image_buffer);

// Decoding the size
uint decode_size_from_lsb(char *image_buffer_for_size_decoding);

// Writing the secret message to deocde.txt output file
Status write_secret_message_to_file(DecodeInfo *decInfo);

#endif