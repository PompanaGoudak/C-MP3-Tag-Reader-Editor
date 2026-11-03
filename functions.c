#include "mp3_header.h"

int validate_cla(int argc, char *argv[], struct MP3 *mp3)
{
    if( argc == 1 )
    {
	printf("------------------------------------------------------------------------------\n");
	printf("ERROR: ./a.out : INVALID ARGUMENTS\n");
	printf("USAGE : To view please pass like: ./a.out -v mp3filename\n");
	printf("To edit please pass like: ./a.out -e -t/-a/-A/-m/-y/-c changing_text mp3filename\n");
	printf("To get help pass like : ./a.out --help\n");
	printf("------------------------------------------------------------------------------\n");
	return 0;
    }

    if( (strcmp(argv[1], "--help") == 0) )
    {
	printf("------------------------------->HELP<-----------------------------------------\n");
	printf("1. -v -> to view mp3 file contents\n");
	printf("2. -e -> to edit mp3 file contents\n");
	printf("\t\t2.1. -t -> to edit song title\n");
	printf("\t\t2.2. -a -> to edit artist name\n");
	printf("\t\t2.3. -A -> to edit album name\n");
	printf("\t\t2.4. -y -> to edit year\n");
	printf("\t\t2.5. -m -> to edit content\n");
	printf("\t\t2.6. -c -> to edit comment\n");
	printf("------------------------------------------------------------------------------\n");
	return 0; 
}
if(!strcmp(argv[1], "-v"))
{
    if(argc != 3)
    {
        printf("ERROR: Invalid arguments\n");
        return 0;
    }

    char *ext = strrchr(argv[2], '.');

    if(ext==NULL || strcmp(ext, ".mp3"))
    {
        printf("ERROR: Invalid file extension\n");
        return 0;
    }

    mp3->mp3_filename = argv[2];

    mp3->org_mp3_fptr = fopen(mp3->mp3_filename, "rb");

    if(mp3->org_mp3_fptr==NULL)
    {
        printf("ERROR: File not found\n");
        return 0;
    }

    return VIEW;
}
if(strcmp(argv[1], "-e") == 0)
    {
        /* Check number of arguments */
        if(argc != 5)
        {
            printf("ERROR: Invalid arguments\n");
            printf("Usage: ./mp3 -e -t/-a/-A/-y/-m/-c new_value filename.mp3\n");
            return 0;
        }

        /* Check file extension */
        char *ext = strrchr(argv[4], '.');

        if(ext == NULL || strcmp(ext, ".mp3") != 0)
        {
            printf("ERROR: Invalid file extension\n");
            return 0;
        }

        /* Store filename */
        mp3->mp3_filename = argv[4];

        /* Store new value */
        mp3->new_value = argv[3];

        /* Identify tag to modify */
        if(strcmp(argv[2], "-t") == 0)
        {
            strcpy(mp3->modified_tag, "TIT2");
        }
        else if(strcmp(argv[2], "-a") == 0)
        {
            strcpy(mp3->modified_tag, "TPE1");
        }
        else if(strcmp(argv[2], "-A") == 0)
        {
            strcpy(mp3->modified_tag, "TALB");
        }
        else if(strcmp(argv[2], "-y") == 0)
        {
            strcpy(mp3->modified_tag, "TYER");
        }
        else if(strcmp(argv[2], "-m") == 0)
        {
            strcpy(mp3->modified_tag, "TCON");
        }
        else if(strcmp(argv[2], "-c") == 0)
        {
            strcpy(mp3->modified_tag, "COMM");
        }
        else
        {
            printf("ERROR: Invalid edit option\n");
            return 0;
        }

        return EDIT;
    }

    /* Invalid operation */
    else
    {
        printf("ERROR: Invalid operation\n");
        printf("Use -v for view or -e for edit\n");
        return 0;
    }    
}


void toggle_endianess(struct MP3 *mp3, int *size)
{
    unsigned char *ptr = (unsigned char *)size;
    unsigned char temp;

    temp = ptr[0];
    ptr[0] = ptr[3];
    ptr[3] = temp;

    temp = ptr[1];
    ptr[1] = ptr[2];
    ptr[2] = temp;
}
void tag_to_name(char *tag)
{
    if(!strcmp(tag, "TIT2"))
    {
        printf("Song Title");
    }
    else if(!strcmp(tag, "TPE1"))
    {
        printf("Artist Name");
    }
    else if(!strcmp(tag, "TALB"))
    {
        printf("Album Name");
    }
    else if(!strcmp(tag, "TYER"))
    {
        printf("Year");
    }
    else if(!strcmp(tag, "TCON"))
    {
        printf("Content");
    }
    else if(!strcmp(tag, "COMM"))
    {
        printf("Comment");
    }
    else
    {
        printf("Unknown Tag");
    }
}
