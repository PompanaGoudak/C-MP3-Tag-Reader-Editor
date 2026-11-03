#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "mp3_header.h"

void edit(struct MP3 *mp3)
{
    unsigned char header[10];
    unsigned char size_bytes[4];
    unsigned char flags[2];
    unsigned char encoding;

    char tag[5];

    int frame_size;
    int new_frame_size;
    int write_size;

    unsigned int total_tag_size;
    unsigned int new_tag_size;
    unsigned int remaining;

    unsigned char buffer[1024];

    int found = 0;
    int bytes;
    long tag_end;
    long current_position;

    printf("\n========== EDIT MP3 STARTED ==========\n");
    if(strcmp(mp3->modified_tag, "COMM") == 0)
   {
    printf("COMM tag needs separate handling\n");
    return;
    }


    // Open original MP3 file
    printf("\nOpening original MP3 file: %s\n", mp3->mp3_filename);

    mp3->org_mp3_fptr = fopen(mp3->mp3_filename, "rb");

    if(mp3->org_mp3_fptr == NULL)
    {
        printf("Unable to open original MP3 file\n");
        return;
    }
    printf("\nCreating temp.mp3 file\n");

    // Open temporary file
    mp3->dup_mp3_fptr = fopen("temp.mp3", "wb");

    if(mp3->dup_mp3_fptr == NULL)
    {
        printf("Unable to open temp.mp3\n");
        fclose(mp3->org_mp3_fptr);
        return;
    }

    // Read 10 bytes of ID3 header
    printf("\nReading 10 bytes of ID3 header...\n");
    if(fread(header, 1, 10, mp3->org_mp3_fptr) != 10)
        goto error;
    
    // Validate ID3 header
    if(memcmp(header, "ID3", 3) != 0 || header[3] != 3)
    {
        printf("ERROR: Only ID3v2.3 MP3 files are supported\n");
        goto error;
    }
    if((header[6] | header[7] | header[8] | header[9]) & 0x80)
    {
    printf("ERROR: Invalid ID3 tag size\n");
    goto error;
    }


    // Calculate total ID3 tag size
    total_tag_size = ((unsigned int)header[6] << 21) |
                     ((unsigned int)header[7] << 14) |
                     ((unsigned int)header[8] << 7) |
                     header[9];

    new_tag_size = total_tag_size;
    tag_end = 10 + total_tag_size;
    // Check actual MP3 file size
if(fseek(mp3->org_mp3_fptr, 0, SEEK_END) != 0)
    goto error;

if(ftell(mp3->org_mp3_fptr) < tag_end)
{
    printf("ERROR: ID3 tag size exceeds file size\n");
    goto error;
}

// Return to the beginning of frame data
if(fseek(mp3->org_mp3_fptr, 10, SEEK_SET) != 0)
    goto error;
    printf("Original ID3 tag size = %u bytes\n", total_tag_size);

    // Copy header to temp file
    if(fwrite(header, 1, 10, mp3->dup_mp3_fptr) != 10)
        goto error;

    printf("ID3 header copied successfully\n");

    // Read frames until the end of ID3 tag
    printf("\n========== READING FRAMES ==========\n");
    while(ftell(mp3->org_mp3_fptr) < tag_end)
    {
        current_position = ftell(mp3->org_mp3_fptr);

    if(tag_end - current_position < 10)
    {
        printf("ERROR: Incomplete frame header\n");
        goto error;
    }
        // Read 4 bytes of frame tag
        if(fread(tag, 1, 4, mp3->org_mp3_fptr) != 4)
            goto error;

        tag[4] = '\0';
        // Check whether padding has started
        if(tag[0] == '\0')
        {
            printf("ID3 padding detected\n");
            break;
        }
        printf("Frame tag found: %s\n", tag);
        // Read 4 bytes of frame size
        if(fread(size_bytes, 1, 4, mp3->org_mp3_fptr) != 4)
            goto error;

        // Convert big endian to little endian
        memcpy(&frame_size, size_bytes, 4);
        toggle_endianess(mp3, &frame_size);

        printf("Frame size = %d bytes\n", frame_size);

        // Validate frame size
        if(frame_size < 1 || frame_size > tag_end - ftell(mp3->org_mp3_fptr) - 2)
        {
            printf("ERROR: Invalid frame size\n");
            goto error;
        }


        // Check whether the tag matches
        if(strcmp(tag, mp3->modified_tag) == 0)
        {
            found = 1;
            printf("\nMATCH FOUND!\n");
            printf("Tag to edit: %s\n", tag);

            // Read 4 bytes of frame size

            printf("Old frame size = %d bytes\n", frame_size);


            // Read 2 bytes of flags
            if(fread(flags, 1, 2, mp3->org_mp3_fptr) != 2)
                goto error;

            // Read encoding byte
            if(fread(&encoding, 1, 1, mp3->org_mp3_fptr) != 1)
                goto error;
            printf("Text encoding byte = %u\n", encoding);
            if(encoding != 0)
            {
                printf("ERROR: Only encoding 0 is supported\n");
                goto error;
            }

            // Skip old frame content
           if(fseek(mp3->org_mp3_fptr, frame_size - 1, SEEK_CUR) != 0)
           goto error;

           printf("Skipping old frame content...\n"); 
            // Calculate new frame size
            if(strlen(mp3->new_value) > 1024 * 1024)
            {
                printf("ERROR: New text is too large\n");
                goto error;
            }
            new_frame_size = strlen(mp3->new_value) + 1;
            printf("\nNew value = %s\n", mp3->new_value);
            printf("New frame size = %d bytes\n", new_frame_size);


            // Write frame tag
           if(fwrite(tag, 1, 4, mp3->dup_mp3_fptr) != 4)
            goto error;

            // Make a copy of new frame size
            write_size = new_frame_size;

            // Convert little endian to big endian
            toggle_endianess(mp3, &write_size);

            // Write new frame size
            if(fwrite(&write_size, 1, 4, mp3->dup_mp3_fptr) != 4)
             goto error;
             printf("New frame size written to temp.mp3\n");
            // Write flags
            if(fwrite(flags, 1, 2, mp3->dup_mp3_fptr) != 2)
             goto error;

            if(fwrite(&encoding, 1, 1, mp3->dup_mp3_fptr) != 1)
                goto error;

            // Write new content
            if(fwrite(mp3->new_value, 1, strlen(mp3->new_value),mp3->dup_mp3_fptr) != strlen(mp3->new_value))
    goto error;
            printf("New text content written successfully\n");

            // Update total tag size
            new_tag_size = new_tag_size - frame_size + new_frame_size;
            printf("\nUpdated ID3 tag size = %u bytes\n", new_tag_size);
            

            // Stop loop after editing
            printf("Editing completed for this frame\n");
            break;
        }
        else
        {
             printf("Tag does not match. Copying original frame...\n");
            // Frame size is already read above
            printf("Copying unmatched frame: %s\n", tag);
            // Read 2 bytes of flags
            if(fread(flags, 1, 2, mp3->org_mp3_fptr) != 2)
                goto error;

            // Write frame tag
            if(fwrite(tag, 1, 4, mp3->dup_mp3_fptr) != 4)
           goto error;

            // Write original size bytes
            if(fwrite(size_bytes, 1, 4, mp3->dup_mp3_fptr) != 4)
                goto error;

            // Write flags
            if(fwrite(flags, 1, 2, mp3->dup_mp3_fptr) != 2)
                goto error;

            // Read and copy frame content
            remaining = frame_size;
            printf("Copying frame content...\n");

            while(remaining > 0)
            {
                if(remaining > 1024)
                    bytes = 1024;
                else
                    bytes = remaining;

                if(fread(buffer, 1, bytes,
                         mp3->org_mp3_fptr) != bytes)
                    goto error;

                if(fwrite(buffer, 1, bytes, mp3->dup_mp3_fptr) != bytes)
                goto error;

                remaining = remaining - bytes;
            }
        }
    }

    if(found == 0)
    {
        printf("Tag not found\n");
        goto error;
    }

    // Copy remaining MP3 data
     printf("\n========== COPYING REMAINING MP3 DATA ==========\n");

    while((bytes = fread(buffer, 1, 1024, mp3->org_mp3_fptr)) > 0)
    {
        if(fwrite(buffer, 1, bytes, mp3->dup_mp3_fptr) != bytes)
            goto error;
    }

    if(ferror(mp3->org_mp3_fptr))
    goto error;

    // Update ID3 header size
    header[6] = (new_tag_size >> 21) & 0x7F;
    header[7] = (new_tag_size >> 14) & 0x7F;
    header[8] = (new_tag_size >> 7) & 0x7F;
    header[9] = new_tag_size & 0x7F;

    printf("New ID3 tag size = %u bytes\n", new_tag_size);

    // Move to beginning of temp file
    if(fseek(mp3->dup_mp3_fptr, 0, SEEK_SET) != 0)
    goto error;

    // Write updated header
   if(fwrite(header, 1, 10, mp3->dup_mp3_fptr) != 10)
    goto error;

    // Close files
    fclose(mp3->org_mp3_fptr);
    fclose(mp3->dup_mp3_fptr);
    printf("\nBoth files closed successfully\n");

    // Replace original file
    printf("Replacing original MP3 file...\n");
    if(rename("temp.mp3", mp3->mp3_filename) != 0)
    {
        printf("Unable to rename file\n");
        remove("temp.mp3");
        return;
    }

    printf("Tag edited successfully\n");
    return;

error:

    printf("Editing failed\n");

    fclose(mp3->org_mp3_fptr);
    fclose(mp3->dup_mp3_fptr);

    remove("temp.mp3");
}