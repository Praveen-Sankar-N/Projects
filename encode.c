#include <stdio.h>
#include <string.h>
#include "encode.h"
#include "types.h"
#include "common.h"


/* Function Definitions */

/* Get image size
 * Input: Image file ptr
 * Output: width * height * bytes per pixel (3 in our case)
 * Description: In BMP Image, width is stored in offset 18,
 * and height after that. size is 4 bytes
 */
uint get_image_size_for_bmp(FILE *fptr_image)
{
    uint width, height;
    // Seek to 18th byte
    fseek(fptr_image, 18, SEEK_SET);

    // Read the width (an int)
    fread(&width, sizeof(int), 1, fptr_image);

    // Read the height (an int)
    fread(&height, sizeof(int), 1, fptr_image);

    // Return image capacity
    return width * height * 3;
}

/* 
 * Get File pointers for i/p and o/p files
 * Inputs: Src Image file, Secret file and
 * Stego Image file
 * Output: FILE pointer for above files
 * Return Value: e_success or e_failure, on file errors
 */
Status open_files(EncodeInfo *encInfo)
{
    // Src Image file
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "r");
    // Do Error handling
    if (encInfo->fptr_src_image == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->src_image_fname);

    	return e_failure;
    }
    else
    {
        printf("INFO: Opened %s\n",encInfo ->src_image_fname);
    }

    // Secret file
    encInfo->fptr_secret = fopen(encInfo->secret_fname, "r");
    // Do Error handling
    if (encInfo->fptr_secret == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->secret_fname);

    	return e_failure;
    }
    else
    {
        printf("INFO: Opened %s\n",encInfo ->secret_fname);
    }

    // Stego Image file
    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "w");
    // Do Error handling
    if (encInfo->fptr_stego_image == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->stego_image_fname);

    	return e_failure;
    }
    else
    {
        printf("INFO: Opened %s\n",encInfo ->stego_image_fname);
    }

    // No failure return e_success
    return e_success;
}
/*
 * Get argv values from CLA and validating the file existence
 * And stroing it in those file names in respective structure members
 */
Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo)
{
    if(argv[2] != NULL && strcmp(strstr(argv[2],"."),".bmp") == 0)
    {
        encInfo -> src_image_fname = argv[2]; 
    }
    else
    {
        return e_failure;
    }
    if(argv[3] != NULL && strcmp(strstr(argv[3],"."),".txt") == 0)
    {
        encInfo -> secret_fname = argv[3];
    }
    else
    {
        return e_failure;
    }
    if(argv[4] != NULL)
    {
        encInfo -> stego_image_fname = argv[4];
    }
    else
    {
        encInfo -> stego_image_fname = "stego.bmp";
        printf("INFO: Output File Not Mentioned. Creating %s by default\n",encInfo->stego_image_fname);
    }
    return e_success;
}

/*
 * GEtting the data from input like secret file extension and secret file data
 * And encoding it into the beautiful.bmp file to produce stego.bmp
 */
Status encode_data_to_image(const char *data, int size, FILE *fptr_src_image, FILE *fptr_stego_image, EncodeInfo *encInfo)
{
    for (int i = 0; i < size; i++)
    {
        fread(encInfo ->image_data,sizeof(char),8,fptr_src_image);
        encode_byte_to_lsb(data[i],encInfo ->image_data);
        fwrite(encInfo ->image_data,sizeof(char),8,fptr_stego_image);
    }
    return e_success;
}
/*
 * Getting the image buffer and data from previous function. 
 * And encoding the data into stego.bmp
 */
Status encode_byte_to_lsb(const char data, char *image_buffer)
{
    for(int i = 0; i < 8 ; i++)
    {
        image_buffer[i] = (image_buffer[i] & 0xFE) | ((1 << (7 - i)) & data) >> (7 - i);
    }
    return e_success;
}

/*
 * Get the size of file or extension size and image buffer
 * And encode thos sizes in that image which will helpful in decoding stage
 */
Status encode_size_to_lsb(int size, char *image_buffer)
{
    char ch;
    for (int i = 0 ; i < 32 ; i++)
    {
        image_buffer[i] = (image_buffer[i] & 0xFE) | ((1 << (31 - i)) & size ) >> (31 - i);
    }
    return e_success;
}

/*
 * Main encoding function from where we will be calling every needed function to encode
 */
Status do_encoding(EncodeInfo *encInfo)
{
    // call rest of the functions from here

    // Files opening
    if(open_files(encInfo) == e_success)
    {
        printf("INFO: Done\n");
        printf("INFO: ## Encoding Procedure Started ##\n");
        //check the capacity (size of .bmp file
        if(check_capacity(encInfo) == e_success)
        {
            // Copying the header 54 bytes from input file to stego file
            if(copy_bmp_header(encInfo->fptr_src_image,encInfo->fptr_stego_image) == e_success)
            {
                printf("INFO: Done completed\n");
                // Encoding the magic string
                if(encode_magic_string(MAGIC_STRING,encInfo) == e_success)
                {
                    printf("INFO: Done Completed\n");
                    //Encode Secret file extension size 1st
                    if (encode_secret_extn_size(strlen(strstr(encInfo ->secret_fname,".")),encInfo ->fptr_src_image, encInfo ->fptr_stego_image) == e_success)
                    {
                            printf("INFO: Done Completed\n");
                            // Encode secret file extension ".txt" inside stego.bmp
                            if(encode_secret_file_extn(encInfo ->extn_secret_file, encInfo) == e_success)
                            {
                                printf("INFO: Done Completed\n");
                                // Encode Secret file size
                                if(encode_secret_file_size(encInfo ->size_secret_file,encInfo) == e_success)
                                {
                                    printf("INFO: Done Completed\n");
                                    // Encode secret file data now
                                    if(encode_secret_file_data(encInfo) == e_success)
                                    {
                                        printf("INFO: Done Completed\n");
                                        // Copying the remaining data as it is to stego.bmp
                                        if(copy_remaining_img_data(encInfo ->fptr_src_image, encInfo ->fptr_stego_image) == e_success)
                                        {
                                            printf("INFO: Done Completed\n");
                                        }
                                        else
                                        {
                                            printf("Failure: Copying the remaining data is successful\n");
                                            return e_failure;
                                        }
                                    }
                                    else
                                    {
                                        printf("Failure: Encoding the secret file data is not successful\n");
                                        return e_failure;
                                    }

                                }
                                else
                                {
                                    printf("Failure: Encoding secret file size is not successfull\n");
                                    return e_failure;
                                }
                            }
                            else
                            {
                                printf("Failure: Encoding secret file extension is not successful\n");
                                return e_failure;
                            }
                    }
                    else
                    {
                        printf("Failure: Encoding Secret file extension size\n");
                        return e_failure;
                    }
                }
                else
                {
                    printf("Failure: Encoding magic string\n");
                    return e_failure;
                }
            }
            else
            {
                printf("Failure: copying the header\n");
                return e_failure;
            }
        }
        else
        {
            printf("Encoding is not possible as .bmp image is not fit\n");
            return e_failure;
        }
    }
    else
    {
        printf("Failure: Opening files\n");
        return e_failure;
    }
    return e_success;
}
/*
 * Getting the structure members for checking size of secret file and input image file
 * Verifying the capacity of input image file whether it can accomodate the complete encoding process
 */
Status check_capacity(EncodeInfo *encInfo)
{   
    //Getting the secret file size
    encInfo ->size_secret_file = get_file_size(encInfo ->fptr_secret);
    if(encInfo ->size_secret_file > 0)
    {
        printf("INFO: Done. Not empty\n");
    }
    else
    {
        printf("INFO: Its empty\n");
    }
    //GEtting image size
    encInfo ->image_capacity = get_image_size_for_bmp(encInfo ->fptr_src_image);
    //We have to check the .bmp image file is capable of encoding the date.
    printf("INFO: Checking for %s capacity to handle %s\n",encInfo->src_image_fname, encInfo ->secret_fname);
    if(encInfo ->image_capacity > (54 + ((2 + 4 + 4 + 4 + encInfo ->size_secret_file ) * 8)))
    {
        printf("INFO: Done. Found OK\n");
        return e_success;
    }
    else
    {
        return e_failure;
    }
}

/*
 * Get file pointer of secret file to get the file size
 */
uint get_file_size(FILE *fptr_secret)
{
    printf("INFO: Checking for Secret.txt Size\n");
    //Finding the size of the secret file
    fseek(fptr_secret, 0, SEEK_END);
    return ftell(fptr_secret);
}

/*
 * Get source image file pointer and stego image file pointer to copy the 54 bytes header
 */
Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image)
{
    // Copying the 54 bytes of header from .bmp file to stego.bmp
    printf("INFO: Copying Image Header\n");
    char *str[54];
    fseek(fptr_src_image,0,SEEK_SET);
    // Using fread and fwrite because it is not human readable 
    fread(str,sizeof(char),54,fptr_src_image);
    fwrite(str,sizeof(char),54,fptr_dest_image);
    return e_success;
}
/*
 * Get magic string string and structure members for encoding the magic string
 */
Status encode_magic_string(const char *magic_string, EncodeInfo *encInfo)
{
    printf("INFO: Encoding Magic String Signature\n");
    encode_data_to_image(magic_string, strlen(magic_string), encInfo ->fptr_src_image, encInfo ->fptr_stego_image, encInfo);
    return e_success;
}

/*
 * Get size, source file pointer and stego file pointer for encoding size into stego file
 */
Status encode_secret_extn_size(int size, FILE *fptr_src_image, FILE *fptr_stego_image)
{
    printf("INFO: Encoding Secret File Extension Size\n");
    char str[32];
    fread(str,sizeof(char),32,fptr_src_image);
    encode_size_to_lsb(size,str);
    fwrite(str,sizeof(char),32,fptr_stego_image);
    return e_success;
}

/*
 * Get secret file extension and structure for encoding the file extension
 */
Status encode_secret_file_extn(const char *file_extn, EncodeInfo *encInfo)
{
    printf("INFO: Encoding %s File Extension\n", encInfo->secret_fname);
    file_extn = strstr(encInfo ->secret_fname,".");
    encode_data_to_image(file_extn, strlen(file_extn), encInfo ->fptr_src_image, encInfo ->fptr_stego_image, encInfo);
    return e_success;
}

/*
 * Get secret file size and encoding the secret file size
 */
Status encode_secret_file_size(long file_size, EncodeInfo *encInfo)
{
    printf("INFO: Encoding %s File Size\n", encInfo->secret_fname);
    char str[32];
    fread(str,sizeof(char),32,encInfo ->fptr_src_image);
    encode_size_to_lsb(file_size,str);
    fwrite(str,sizeof(char),32,encInfo ->fptr_stego_image);
    return e_success;
}

/*
 * Enccoding the secret file data using secret file pointer and stego file pointer
 */
Status encode_secret_file_data(EncodeInfo *encInfo)
{
    printf("INFO: Encoding %s File Data\n", encInfo->secret_fname);
    char str[encInfo ->size_secret_file];
    fseek(encInfo ->fptr_secret,0,SEEK_SET);
    fread(str,sizeof(char),encInfo->size_secret_file,encInfo ->fptr_secret);
    encode_data_to_image(str, encInfo ->size_secret_file, encInfo ->fptr_src_image,encInfo ->fptr_stego_image, encInfo);
    return e_success;
}

/*
 * Copying the remaining data from source image to stego.bmp
 */
Status copy_remaining_img_data(FILE *fptr_src_image, FILE *fptr_stego_image)
{
    printf("INFO: Copying Left Over Data\n");
    char ch;
    while(fread(&ch,1,1,fptr_src_image))
    {
        fwrite(&ch,1,1,fptr_stego_image);
    }
    return e_success;
}