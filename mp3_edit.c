#include "mp3_header.h"
#include <string.h>
#include <stdlib.h>

void edit(struct MP3 *mp3)
{
    mp3->org_mp3_fptr = fopen(mp3->mp3_filename, "rb");

    if(mp3->org_mp3_fptr == NULL)
    {
        printf("ERROR: Unable to open MP3 file\n");
        return;
    }

    mp3->dup_mp3_fptr = fopen("temp.mp3", "wb");

    if(mp3->dup_mp3_fptr == NULL)
    {
        printf("ERROR: Unable to create temporary file\n");
        fclose(mp3->org_mp3_fptr);
        return;
    }

    /* Copy ID3 header */

    char header[10];

    fread(header, 1, 10, mp3->org_mp3_fptr);

    fwrite(header, 1, 10, mp3->dup_mp3_fptr);

    /* Read frame ID and frame size */

    int i, size;
    int size_difference = 0;

    char tag[5];
    char flag[2];
    unsigned char size_bytes[4];
    int tag_found = 0;

    for(i = 0; i < 6; i++)
    {
        fread(tag, 1, 4, mp3->org_mp3_fptr);
        tag[4] = '\0';

        fread(size_bytes, 1, 4, mp3->org_mp3_fptr);

        memcpy(&size, size_bytes, 4);

        toggle_endianess(mp3, &size);

        fread(flag, 1, 2, mp3->org_mp3_fptr);

        char data[size];

        fread(data, 1, size, mp3->org_mp3_fptr);
         
if(strcmp(tag, mp3->modified_tag) == 0)
{
    tag_found = 1;
    printf("Tag found: %s\n", tag);

    int new_size;
    char *new_data;

    if(strcmp(tag, "COMM") == 0)
    {
        printf("Editing COMMENT frame...\n");

        if(size < 5 || data[0] != 0)
        {
            printf("ERROR: Invalid COMM frame or unsupported encoding\n");
            fclose(mp3->org_mp3_fptr);
            fclose(mp3->dup_mp3_fptr);
            remove("temp.mp3");
            return;
        }

        char *separator = memchr(data + 4, '\0', size - 4);

        if(separator == NULL)
        {
            printf("ERROR: Invalid COMM description\n");
            fclose(mp3->org_mp3_fptr);
            fclose(mp3->dup_mp3_fptr);
            remove("temp.mp3");
            return;
        }

        int prefix_size = (separator - data) + 1;

        new_size = prefix_size + strlen(mp3->new_value);

        new_data = malloc(new_size);

        if(new_data == NULL)
        {
            printf("Memory allocation failed\n");
            fclose(mp3->org_mp3_fptr);
            fclose(mp3->dup_mp3_fptr);
            remove("temp.mp3");
            return;
        }

        memcpy(new_data, data, prefix_size);

        memcpy(new_data + prefix_size,
               mp3->new_value,
               strlen(mp3->new_value));
    }
    else
    {
        new_size = strlen(mp3->new_value) + 1;

        new_data = malloc(new_size);

        if(new_data == NULL)
        {
            printf("Memory allocation failed\n");
            fclose(mp3->org_mp3_fptr);
            fclose(mp3->dup_mp3_fptr);
            remove("temp.mp3");
            return;
        }

        new_data[0] = 0;
        strcpy(new_data + 1, mp3->new_value);
    }

    size_difference = size_difference + (new_size - size);

    fwrite(tag, 1, 4, mp3->dup_mp3_fptr);

    int temp_size = new_size;
    toggle_endianess(mp3, &temp_size);

    fwrite(&temp_size, 1, 4, mp3->dup_mp3_fptr);
    fwrite(flag, 1, 2, mp3->dup_mp3_fptr);
    fwrite(new_data, 1, new_size, mp3->dup_mp3_fptr);

    free(new_data);

    printf("Tag updated successfully\n");
}

        
    else
    {
        fwrite(tag, 1, 4, mp3->dup_mp3_fptr);
        fwrite(size_bytes, 1, 4, mp3->dup_mp3_fptr);
        fwrite(flag, 1, 2, mp3->dup_mp3_fptr);
        fwrite(data, 1, size, mp3->dup_mp3_fptr);
    }
}

    if(tag_found == 0)
    {
        printf("ERROR: Requested tag not found\n");

        fclose(mp3->org_mp3_fptr);
        fclose(mp3->dup_mp3_fptr);

        remove("temp.mp3");

        return;
    }


    int ch;

    while((ch = fgetc(mp3->org_mp3_fptr)) != EOF)
    {
        fputc(ch, mp3->dup_mp3_fptr);
    }

    /* Update ID3 tag size */

    int old_tag_size;

    old_tag_size = (((unsigned char)header[6] & 0x7F) << 21) |
               (((unsigned char)header[7] & 0x7F) << 14) |
               (((unsigned char)header[8] & 0x7F) << 7)  |
               ((unsigned char)header[9] & 0x7F);

    int new_tag_size = old_tag_size + size_difference;

    header[6] = (new_tag_size >> 21) & 0x7F;
    header[7] = (new_tag_size >> 14) & 0x7F;
    header[8] = (new_tag_size >> 7) & 0x7F;
    header[9] = new_tag_size & 0x7F;

    fseek(mp3->dup_mp3_fptr, 0, SEEK_SET);

    fwrite(header, 1, 10, mp3->dup_mp3_fptr);

    fclose(mp3->org_mp3_fptr);
    fclose(mp3->dup_mp3_fptr);

    if(rename("temp.mp3", mp3->mp3_filename) != 0)
    {
        printf("ERROR: Unable to update MP3 file\n");
        return;
    }

    printf("MP3 file updated successfully\n");
}