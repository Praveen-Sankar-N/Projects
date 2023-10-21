#include <stdio.h>
#include "encode.h"
#include "types.h"
#include <string.h>
#include "decode.h"

/*
 * Entry point of the project STEGANOGRAPHY
 */
int main(int argc, char *argv[])
{
    uint img_size;
    // Check the command line argument passed or not
    if (argc > 1)
    {
        // Check the operation type -e or -d
        if (check_operation_type(argv) == e_encode)
        {
            //printf("INFO: ## Encoding Procedure Started ##\n");
            EncodeInfo encInfo;
            if (read_and_validate_encode_args(argv, &encInfo) == e_success)
            {
                //printf("INFO: Read and validation is successful\n");
                // Start encoding after validation and all
                if (do_encoding(&encInfo) == e_success)
                {
                    printf("INFO: ## Encoding Done Successfully ##\n");
                }
                else
                {
                    printf("Failure : Encodeing is not completed\n");
                }
            }
            else
            {
                printf("Read and validation is not successful\n");
            }
        }
        else if (check_operation_type(argv) == e_decode)
        {
            printf("INFO: ## Decoding Procedure Started ##\n");
            DecodeInfo decInfo;
            // Validation of file needed for decoding
            if(read_and_validate_decode(argv, &decInfo) == e_success)
            {
                printf("INFO: Read and validation is successful\n");
                //Start decoding from here
                if(do_decoding(&decInfo) == e_success)
                {
                    printf("INFO: ## Decoding Done Successfully #\n");
                }
                else
                {
                    printf("FAIL: Decoding is not completed\n");
                }
            }
            else
            {
                printf("FAIL: Validation of files is not successful\n");
            }
        }
        else
        {
            printf("Invalid Option\n*****Usage*****\n");
            printf("Encoding: ./a.out -e beautiful.bmp secret.txt stego.bmp\n");
            printf("Decoding: ./a.out -d stego.bmp\n");
        }
    }
    else
    {
        printf("Invalid Input\n*****Usage*****\n");
        printf("Encoding: ./a.out -e beautiful.bmp secret.txt stego.bmp\n");
        printf("Decoding: ./a.out -d stego.bmp\n");
    }
    return 0;
}

OperationType check_operation_type(char *argv[])
{
    if (strcmp(argv[1], "-e") == 0)
    {
        return e_encode;
    }
    else if (strcmp(argv[1], "-d") == 0)
    {
        return e_decode;
    }
    else
    {
        return e_unsupported;
    }
}
