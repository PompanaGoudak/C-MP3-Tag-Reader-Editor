#include "mp3_header.h"

void view(struct MP3 *mp3)
{
    char tag[5];
    int frame_size;
    int i;
    printf("\n========== VIEW FUNCTION STARTED ==========\n");


    // Skip 10 bytes of ID3 header
    fseek(mp3->org_mp3_fptr, 10, SEEK_SET);

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
        printf("Tag ID: %s\n", tag);

        // Read 4 bytes of frame size
        if(fread(&frame_size, 4, 1, mp3->org_mp3_fptr) != 1)
        {
            printf("ERROR: Failed to read frame size\n");
            break;
        }
        printf("Frame size before endian conversion: %d\n", frame_size);

        // Convert endianess
        toggle_endianess(mp3, &frame_size);
        printf("Frame size after endian conversion: %d\n", frame_size);


        // Skip 2 bytes of flags
        if(fseek(mp3->org_mp3_fptr, 2, SEEK_CUR) != 0)
        {
            printf("ERROR: Failed to skip flags\n");
            break;
        }
         printf("Tag Name: ");
        // Display tag name
        tag_to_name(tag);
        printf("\nTag Content: ");

        // Display tag data
        print_tag_data(mp3->org_mp3_fptr, frame_size);

        printf("\n");
    }

    fclose(mp3->org_mp3_fptr);
    printf("\n========== VIEW FUNCTION ENDED ==========\n");
}


void print_tag_data(FILE *org_mp3_fptr, int frame_size)
{
    char data[frame_size + 1];

    if(fread(data, 1, frame_size, org_mp3_fptr) != frame_size)
    {
        printf("ERROR: Failed to read tag data\n");
        return;
    }

    data[frame_size] = '\0';

    // Skip text encoding byte
    printf("%s", data + 1);
}