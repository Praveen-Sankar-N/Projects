#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "decode.h"
#include "common.h"
#include "types.h"



/*
 * Function definitions for decoding are available here
 * Getting the argv and accessing structure members here to validate
 */

Status read_and_validate_decode(char *argv[], DecodeInfo *decInfo)
{
    if(argv[2] != NULL && strcmp(strstr(argv[2],"."),".bmp") == 0)
    {
        decInfo ->stego_file_name = argv[2];
    }
    else
    {
        return e_failure;
    }
    return e_success;
}

/*
 * Function for opening the stego file
 * Using file pointer and stego file name 
 */
Status open_stego_file_decode(DecodeInfo *decInfo)
{
    printf("INFO: Opening required files\n");
    decInfo ->fptr_stego = fopen(decInfo ->stego_file_name,"r");
    if (decInfo ->fptr_stego == NULL)
    {
        perror("fopen");
        printf("Error: Unable to open file %s\n",decInfo ->stego_file_name);
        return e_failure;
    }
    return e_success;
}

/*
 * Function for decoding the byte information from lsb of the stego.bmp
 * Byte by byte data is being sent to decode_byte_from_lsb2 function for decoding every byte
 */
Status decode_byte_from_lsb1(char *decoding_data, int size, FILE *fptr_stego, char *image_buffer_stego, DecodeInfo *decInfo)
{
    int i;
    for (i = 0; i < size; i++ )
    {
        fread(image_buffer_stego, sizeof(char),8,fptr_stego);
        decoding_data[i] = decode_byte_form_lsb2(decInfo, image_buffer_stego);
    }
    decInfo ->dec_magic_str[i] = '\0';

    return e_success;
}
/*
 *Function for decoding every byte of image data and getting the characters
 * Returning that characters to store in a string in previous lsb1 function
 */
char decode_byte_form_lsb2(DecodeInfo *decInfo, char *image_buffer)
{
    int i;
    char ch = 0x00;
    for (i = 0 ; i < 8 ; i++)
    {
        ch = (image_buffer[i] & 1) << (7 - i) | ch;
    }
    return ch;
}

/*
 * Function for decoding the size of extension and file size from stego.bmp
 */
uint decode_size_from_lsb(char *image_buffer_for_size_decoding)
{
    uint size = 0;
    for(int i = 0; i < 32; i++)
    {
        size = ((image_buffer_for_size_decoding[i] & 1) << (31 - i)) | size;
    }
    return size;
}

/*
 * Function for opening the output file decode.txt
 */
Status open_decode_file(DecodeInfo *decInfo)
{
    printf("INFO: Output File Not Mentioned. Creating decode.txt by default\n");
    strcpy(decInfo->decoded_file_name,"decode");
    strcat(decInfo->decoded_file_name,decInfo->secret_file_extn_data);
    decInfo->fptr_decode_file = fopen(decInfo->decoded_file_name,"w");
    printf("INFO: Opened %s\n",decInfo->decoded_file_name);
    return e_success;   
}

Status do_decoding(DecodeInfo *decInfo)
{
    if (open_stego_file_decode(decInfo) == e_success)
    {
        printf("INFO: Opened %s\n", decInfo->stego_file_name);
        //Decode Magic string
        if(decode_magic_string(decInfo) == e_success)
        {
            printf("INFO: Done Completed\n");
            // Comparing the decoded magic string and original magic string
            if (strcmp(decInfo ->dec_magic_str, MAGIC_STRING) == 0)
            {
                printf("INFO: Magic string is matched further decoding can be done\n");
                // Decoding the size of secret file extension
                if (decode_secret_file_extn_size(decInfo) == e_success)
                {
                    printf("INFO: Done Completed\n");
                    // Decode the scret file extension
                    if (decode_secret_file_extn(decInfo) == e_success)
                    {
                        printf("INFO: Done Completed\n");
                        // Create a decode output file in write with above found file extension
                        if(open_decode_file(decInfo) == e_success)
                        {
                            printf("INFO: Done. Opened all required files\n");
                            // Decode secret file size
                            if (decode_secret_file_size(decInfo) == e_success)
                            {
                                printf("INFO: Done Completed\n");
                                // Decoding secret file data
                                if(decode_secret_file_data(decInfo) == e_success)
                                {
                                    printf("INFO: Done Completed\n");
                                    // Write secret file data to file
                                    if(write_secret_message_to_file(decInfo) == e_success)
                                    {
                                        printf("INFO: Secret data retrieve is done\n");
                                    }
                                    else
                                    {
                                        printf("FAIL: Secret file data is not retrieved\n");
                                        return e_failure;
                                    }
                                }
                                else
                                {
                                    printf("FAIL: Secret file data is not decoded successfully\n");
                                    return e_failure;
                                }
                            }
                            else
                            {
                                printf("INFO: Secret file size is not decoded successfully\n");
                                return e_failure;
                            }
                        }
                        else
                        {
                            printf("FAIL: Output file can't be created successfully\n");
                            return e_failure;
                        }

                    }
                    else
                    {
                        printf("FAIL: Secret file extension has not been decoded successfully\n");
                        return e_failure;
                    }
                }
                else
                {
                    printf("FAIL: The secret file extension is not decode successfully\n");
                    return e_failure;
                }
            }
            else
            {
                printf("FAIL: Magic string is not matched further decoding can not be done\n");
                return e_failure;
            }
        }
        else
        {
            printf("FAIL: Magic string decoding is not successful\n");
            return e_failure;
        }
    }
    else
    {
        printf("FAIL: File opening is not successful\n");
        return e_failure;
    }
    return e_success;
}

/* Decoding Magic string
 * Using file pointer of stego.bmp skipping the header bytes
 * From Here calling the decode_byte_from_lsb1 function
 */
Status decode_magic_string(DecodeInfo *decInfo)
{
    printf("INFO: Decoding Magic String Signature\n");
    fseek(decInfo ->fptr_stego,54,SEEK_SET);
    // Calling the decoding function
    decode_byte_from_lsb1(decInfo ->dec_magic_str, strlen(MAGIC_STRING), decInfo ->fptr_stego, decInfo ->stego_image_buffer, decInfo);
    return e_success;
}
/*
 * Function for decoding the file extension size
 * Using secret_file_extn_size which is structure member.
 */
Status decode_secret_file_extn_size(DecodeInfo *decInfo)
{
    printf("INFO: Decoding Secret File Extension Size\n");
    char str[32];
    fread(str,sizeof(char),32,decInfo ->fptr_stego);
    decInfo ->secret_file_extn_size =  decode_size_from_lsb(str);
    return e_success;
}

/*
 * Function for decoding secret file extension data ".txt"
 * Using Secret file extension size
 */
Status decode_secret_file_extn(DecodeInfo *decInfo)
{
    printf("INFO: Decoding Secret File Extension\n");
    decInfo->secret_file_extn_data = (char *) malloc(sizeof(char) * (decInfo->secret_file_extn_size + 1));
    decode_byte_from_lsb1(decInfo->secret_file_extn_data, decInfo->secret_file_extn_size, decInfo->fptr_stego, decInfo->stego_image_buffer, decInfo);
    return e_success;
}
/*
 * Function for decoding secret file size from stego.bmp
 * Storing the size in a structure member
 */
Status decode_secret_file_size(DecodeInfo *decInfo)
{
    printf("INFO: Decoding %s File Size\n",decInfo->decoded_file_name);
    int size = sizeof(int) * 8;
    char str[32];
    fread(str,sizeof(char),32, decInfo->fptr_stego);
    decInfo ->secret_file_size = decode_size_from_lsb(str);
    return e_success;
}
/*
 * Function for decoding the secret file data
 * Using secret file size and stroing it in a string which is structure member
 */
Status decode_secret_file_data(DecodeInfo *decInfo)
{
    printf("INFO: Decoding %s File Data\n",decInfo->decoded_file_name);
    decInfo ->secret_file_data = (char *) malloc(sizeof(char) * (decInfo ->secret_file_size + 1));
    decode_byte_from_lsb1(decInfo ->secret_file_data, decInfo ->secret_file_size, decInfo ->fptr_stego, decInfo->stego_image_buffer, decInfo);
    return e_success;
}
/*
 * Function for writing the extracted secret file data to decode.txt
 * Using string where we stored the secret file data
 */
Status write_secret_message_to_file(DecodeInfo *decInfo)
{
    fseek(decInfo ->fptr_decode_file,0,SEEK_SET);
    for(int i = 0 ; i < decInfo -> secret_file_size; i++)
    {
        fwrite(decInfo->secret_file_data, 1,1,decInfo ->fptr_decode_file);
        decInfo->secret_file_data++;
    }
    return e_success;
}

