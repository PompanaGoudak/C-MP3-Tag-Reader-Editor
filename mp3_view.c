#include <string.h>
#include <stdlib.h>
#include "mp3_header.h"

void view(struct MP3 *mp3)
{
    char tag[5];
    int frame_size;
    int i;
    printf("\n========== VIEW FUNCTION STARTED ==========\n");


    // Skip 10 bytes of ID3 header
    if(fseek(mp3->org_mp3_fptr, 10, SEEK_SET) != 0)
    {
        printf("ERROR: Unable to skip ID3 header\n");
        goto cleanup;
    }


    printf("\n----------- MP3 DETAILS -----------\n");

    for(i = 0; i < 6; i++)
    {
        // Read 4 bytes of tag ID
        if(fread(tag, 1, 4, mp3->org_mp3_fptr) != 4)
        {
            printf("ERROR: Failed to read tag ID\n");
            break;
        }

        tag[4] = '\0';
        
       

        // Read 4 bytes of frame size
        if(fread(&frame_size, 4, 1, mp3->org_mp3_fptr) != 1)
        {
            printf("ERROR: Failed to read frame size\n");
            break;
        }
        
        // Convert endianess
        toggle_endianess(mp3, &frame_size);
        

        // Skip 2 bytes of flags
        if(fseek(mp3->org_mp3_fptr, 2, SEEK_CUR) != 0)
        {
            printf("ERROR: Failed to skip flags\n");
            break;
        }
         
        // Display tag name
        tag_to_name(tag);
        printf(": ");
        
        // Display tag data
        print_tag_data(mp3->org_mp3_fptr, frame_size, tag);
        printf("\n");

        
    }
   cleanup:

    if(mp3->org_mp3_fptr != NULL)
    {
        fclose(mp3->org_mp3_fptr);
        mp3->org_mp3_fptr = NULL;
    }
    printf("\n========== VIEW FUNCTION ENDED ==========\n");
}



void print_tag_data(FILE *org_mp3_fptr, int frame_size, char *tag)
{
    char data[frame_size + 1];

    if(fread(data, 1, frame_size, org_mp3_fptr) != (size_t)frame_size)
    {
        printf("ERROR: Failed to read tag data\n");
        return;
    }

    data[frame_size] = '\0';

    if(strcmp(tag, "COMM") == 0)
    {
        if(frame_size < 5 || data[0] != 0)
        {
            printf("Invalid or unsupported COMM frame\n");
            return;
        }

        char *separator = memchr(data + 4, '\0', frame_size - 4);

        if(separator == NULL)
        {
            printf("Invalid COMM description\n");
            return;
        }

        printf("%s", separator + 1);
    }
    else
    {
        printf("%s", data + 1);
    }
}
