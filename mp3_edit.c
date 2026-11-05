
#include "mp3_header.h"
#include <string.h>
#include <stdlib.h>

void edit(struct MP3 *mp3)
{
    char header[10];
    char tag[5];
    char flag[2];
    unsigned char size_bytes[4];

    int i;
    int size;
    int new_size;
    int size_difference = 0;
    int tag_found = 0;
    int failed = 0;
    int ch;

    char *data = NULL;
    char *new_data = NULL;
    printf("\n========== EDIT FUNCTION STARTED ==========\n");
     /* Open original MP3 file */
    mp3->org_mp3_fptr = fopen(mp3->mp3_filename, "rb");

    if(mp3->org_mp3_fptr == NULL)
    {
        printf("ERROR: Unable to open MP3 file\n");
        return;
    }
     /* Create temporary file */
    mp3->dup_mp3_fptr = fopen("temp.mp3", "wb");

    if(mp3->dup_mp3_fptr == NULL)
    {
        printf("ERROR: Unable to create temporary file\n");
        fclose(mp3->org_mp3_fptr);
        mp3->org_mp3_fptr = NULL;
        return;
    }
    /* Read ID3 header */
    if(fread(header, 1, 10, mp3->org_mp3_fptr) != 10)
    {
        printf("ERROR: Unable to read ID3 header\n");
        failed = 1;
        goto cleanup;
    }
    /* Copy ID3 header to temporary file */
    if(fwrite(header, 1, 10, mp3->dup_mp3_fptr) != 10)
    {
        printf("ERROR: Unable to write ID3 header\n");
        failed = 1;
        goto cleanup;
    }
    /* Process six frames */
    for(i = 0; i < 6; i++)
    {
        /* Read frame ID */
        if(fread(tag, 1, 4, mp3->org_mp3_fptr) != 4)
        {
            printf("ERROR: Unable to read frame ID\n");
            failed = 1;
            break;
        }
        tag[4] = '\0';
        /* Read frame size */
        if(fread(size_bytes, 1, 4, mp3->org_mp3_fptr) != 4)
        {
            printf("ERROR: Unable to read frame size\n");
            failed = 1;
            break;
        }

        memcpy(&size, size_bytes, 4);
        toggle_endianess(mp3, &size);
        /* Read frame flags */
        if(fread(flag, 1, 2, mp3->org_mp3_fptr) != 2)
        {
            printf("ERROR: Unable to read frame flags\n");
            failed = 1;
            break;
        }
        if(size < 0)
        {
            printf("ERROR: Invalid frame size\n");
            failed = 1;
            break;
        }
        /* Allocate memory for frame data */
        data = malloc(size > 0 ? size : 1);

        if(data == NULL)
        {
            printf("ERROR: Memory allocation failed\n");
            failed = 1;
            break;
        }

        /* Read frame data */
        if(fread(data, 1, size, mp3->org_mp3_fptr) != (size_t)size)
        {
            printf("ERROR: Unable to read frame data\n");
            free(data);
            data = NULL;
            failed = 1;
            break;
        }
        /* Check whether this is the frame to modify */
        if(strcmp(tag, mp3->modified_tag) == 0)
        {
            tag_found = 1;
            printf("Tag found: %s\n", tag);
            printf("Old value is being replaced...\n");
            if(strcmp(tag, "COMM") == 0)
            {
                printf("Editing COMMENT frame...\n");

                /* COMM data must contain encoding, language
                   and a null-terminated description */
                if(size < 5 || data[0] != 0)
                {
                    printf("ERROR: Unsupported COMMENT frame format\n");
                    free(data);
                    data = NULL;
                    failed = 1;
                    break;
                }

                char *separator = memchr(data + 4, '\0', size - 4);

                if(separator == NULL)
                {
                    printf("ERROR: COMMENT description not found\n");
                    free(data);
                    data = NULL;
                    failed = 1;
                    break;
                }

                /* Preserve encoding, language and description */
                int prefix_size = (separator - data) + 1;

                new_size = prefix_size + strlen(mp3->new_value);

                new_data = malloc(new_size > 0 ? new_size : 1);

                if(new_data == NULL)
                {
                    printf("ERROR: Memory allocation failed\n");
                    free(data);
                    data = NULL;
                    failed = 1;
                    break;
                }
                memcpy(new_data, data, prefix_size);
                memcpy(new_data + prefix_size,mp3->new_value,strlen(mp3->new_value));
            }
            else
            {
                /* Normal text frame */
                new_size = strlen(mp3->new_value) + 1;
                new_data = malloc(new_size);
                if(new_data == NULL)
                {
                    printf("ERROR: Memory allocation failed\n");
                    free(data);
                    data = NULL;
                    failed = 1;
                    break;
                }
                /* First byte is text encoding */
                new_data[0] = 0;
                strcpy(new_data + 1, mp3->new_value);
            }
            size_difference += new_size - size;
            /* Write frame ID */
            if(fwrite(tag, 1, 4, mp3->dup_mp3_fptr) != 4)
            {
                printf("ERROR: Unable to write frame ID\n");
                free(data);
                free(new_data);
                data = NULL;
                new_data = NULL;
                failed = 1;
                break;
            }
            /* Convert new frame size to big-endian */
            int temp_size = new_size;
            toggle_endianess(mp3, &temp_size);
            if(fwrite(&temp_size, 1, 4, mp3->dup_mp3_fptr) != 4 ||
               fwrite(flag, 1, 2, mp3->dup_mp3_fptr) != 2 ||
               fwrite(new_data, 1, new_size, mp3->dup_mp3_fptr) != (size_t)new_size)
            {
                printf("ERROR: Unable to write modified frame\n");
                free(data);
                free(new_data);
                data = NULL;
                new_data = NULL;
                failed = 1;
                break;
            }
             printf("\nTag       : %s\n", tag);
            printf("New value : %s\n", mp3->new_value);
            free(new_data);
            new_data = NULL;

            
        }
        else
        {
            /* Copy unchanged frame */
            if(fwrite(tag, 1, 4, mp3->dup_mp3_fptr) != 4 ||
               fwrite(size_bytes, 1, 4, mp3->dup_mp3_fptr) != 4 ||
               fwrite(flag, 1, 2, mp3->dup_mp3_fptr) != 2 ||
               fwrite(data, 1, size, mp3->dup_mp3_fptr) != (size_t)size)
            {
                printf("ERROR: Unable to copy frame\n");
                free(data);
                data = NULL;
                failed = 1;
                break;
            }
        }

        /* Release memory after processing each frame */
        free(data);
        data = NULL;
    }
    if(failed == 1)
        goto cleanup;
    if(tag_found == 0)
    {
        printf("ERROR: Requested tag not found\n");
        failed = 1;
        goto cleanup;
    }
    /* Copy remaining MP3 audio data */
    while((ch = fgetc(mp3->org_mp3_fptr)) != EOF)
    {
        if(fputc(ch, mp3->dup_mp3_fptr) == EOF)
        {
            printf("ERROR: Unable to copy remaining MP3 data\n");
            failed = 1;
            goto cleanup;
        }
    }

    if(ferror(mp3->org_mp3_fptr))
    {
        printf("ERROR: Error while reading MP3 data\n");
        failed = 1;
        goto cleanup;
    }
    /* Calculate original ID3 tag size */
    int old_tag_size =
        (((unsigned char)header[6] & 0x7F) << 21) |
        (((unsigned char)header[7] & 0x7F) << 14) |
        (((unsigned char)header[8] & 0x7F) << 7) |
        ((unsigned char)header[9] & 0x7F);
    /* Calculate new ID3 tag size */
    int new_tag_size = old_tag_size + size_difference;
    if(new_tag_size < 0 || new_tag_size > 0x0FFFFFFF)
    {
        printf("ERROR: Invalid updated ID3 tag size\n");
        failed = 1;
        goto cleanup;
    }

    /* Store updated size in syncsafe format */
    header[6] = (new_tag_size >> 21) & 0x7F;
    header[7] = (new_tag_size >> 14) & 0x7F;
    header[8] = (new_tag_size >> 7) & 0x7F;
    header[9] = new_tag_size & 0x7F;

    /* Update ID3 header */
    if(fseek(mp3->dup_mp3_fptr, 0, SEEK_SET) != 0 ||
       fwrite(header, 1, 10, mp3->dup_mp3_fptr) != 10)
    {
        printf("ERROR: Unable to update ID3 header\n");
        failed = 1;
        goto cleanup;
    }
     /* Display original and updated ID3 tag sizes */
    printf("\nOriginal ID3 tag size : %d bytes\n", old_tag_size);
    printf("Updated ID3 tag size  : %d bytes\n", new_tag_size);

cleanup:

    if(data != NULL)
        free(data);

    if(new_data != NULL)
        free(new_data);

    if(mp3->org_mp3_fptr != NULL)
    {
        fclose(mp3->org_mp3_fptr);
        mp3->org_mp3_fptr = NULL;
    }

    if(mp3->dup_mp3_fptr != NULL)
    {
        if(fclose(mp3->dup_mp3_fptr) != 0)
        {
            printf("ERROR: Unable to close temporary file\n");
            failed = 1;
        }

        mp3->dup_mp3_fptr = NULL;
    }

    if(failed == 1)
    {
        remove("temp.mp3");
        return;
    }

    /* Replace original file with updated file */
    printf("\nReplacing original MP3 file...\n");
    if(rename("temp.mp3", mp3->mp3_filename) != 0)
    {
        printf("ERROR: Unable to replace original MP3 file\n");
        remove("temp.mp3");
        return;
    }

    printf("MP3 file updated successfully\n");
     printf("========== EDIT FUNCTION COMPLETED ==========\n");
}
