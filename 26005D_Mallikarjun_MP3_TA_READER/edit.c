#include<stdio.h>
#include<string.h>
#include "mp3.h"

unsigned int e_data_size;
char temp_tag[50],data[500],flag[4];

/* edits the selected ID3v2.3 frame and rewrites the file through a duplicate */
Status edit_info(details *info)
{
    //printf("error\n");
    info->duplicate_fname = "duplicate.mp3";
    if(!(e_open_file(info)))
    {
        return failure;
    }
    if(!(e_check_size(info->fptr_source_fname)))
    {
        return failure;
    }
    if(!(e_check_version(info->fptr_source_fname)))
    {
        return failure;
    }
     printf(YELLOW "\n----------------- SELECTED EDIT DETAILS ----------------------\n\n" RESET);
    printf(MAGENTA "------------- SELECTED EDIT OPTION -------------\n" RESET);

    rewind(info->fptr_source_fname);
    fread(data,10,1,info->fptr_source_fname);
    fwrite(data,10,1,info->fptr_duplicate_fname); // ID3 header is copied as it is

    /* first 6 frames: TIT2, TPE1, TALB, TYER, TCON, COMM */
    for(int i=0;i<6;i++)
    {
        e_read_tag(info);
        fwrite(temp_tag, 4, 1, info->fptr_duplicate_fname );
        e_read_size(info);

        e_read_data(info);
         //printf("DEBUG: file tag=[%s] target=[%s]\n", temp_tag, info->tag);
        /* tag matched, so replace old data with the new data */
        if(strcmp(temp_tag,info->tag)==0)
        {
            strcpy(data,info->change_data);
            e_data_size = strlen(info->change_data);
            e_data_size = e_data_size+1; // +1 for the encoding byte

             /* based on user given command printing info by matching the tags*/
            if (strcmp(info->tag, "TIT2") == 0)
            {
                printf(CYAN "-------------- CHANGE THE TITLE --------------\n\n" RESET);
                printf("TITLE\t: %s\n", data);
                printf(MAGENTA "------------------------------------------------\n" RESET);
            }
            else if (strcmp(info->tag, "TPE1") == 0)
            {
                printf(CYAN"-------------- CHANGE THE ARTIST --------------\n\n"RESET);
                printf("ARTIST\t: %s\n", data);
                printf(MAGENTA"------------------------------------------------\n"RESET);
            }
            else if (strcmp(info->tag, "TALB") == 0)
            {
                printf(CYAN"-------------- CHANGE THE Album --------------\n\n"RESET);
                printf("ALBUM\t: %s\n", data);
                printf(MAGENTA"------------------------------------------------\n"RESET);
            }
            else if (strcmp(info->tag, "TYER") == 0)
            {
                printf(CYAN"-------------- CHANGE THE YEAR --------------\n\n"RESET);
                printf("YEAR\t: %s\n", data);
                printf(MAGENTA"------------------------------------------------\n"RESET);
            }
            else if (strcmp(info->tag, "TCON") == 0)
            {
                printf(CYAN"-------------- CHANGE THE MUSIC --------------\n\n"RESET);
                printf("MUSIC\t: %s\n", data);
                printf(MAGENTA"------------------------------------------------\n"RESET);
            }
            else if (strcmp(info->tag, "COMM") == 0)
            {
                printf(CYAN"-------------- CHANGE THE COMMENT --------------\n\n"RESET);
                printf("COMMENT\t: %s\n", data);
                printf(MAGENTA"------------------------------------------------\n"RESET);
            }
        }
         /* reversing the size */
        e_data_size = ((e_data_size & 0xFF000000) >> 24) | // Move byte 1 to byte 4
                      ((e_data_size & 0x00FF0000) >> 8) |  // Move byte 2 to byte 3
                      ((e_data_size & 0x0000FF00) << 8) |  // Move byte 3 to byte 2
                      ((e_data_size & 0x000000FF) << 24);  // Move byte 4 to byte 1


        fwrite(&e_data_size, 4, 1, info->fptr_duplicate_fname);
        fwrite(flag, 3, 1, info->fptr_duplicate_fname);

        /* reversing the size */
        e_data_size = ((e_data_size & 0xFF000000) >> 24) | // Move byte 1 to byte 4
                      ((e_data_size & 0x00FF0000) >> 8) |  // Move byte 2 to byte 3
                      ((e_data_size & 0x0000FF00) << 8) |  // Move byte 3 to byte 2
                      ((e_data_size & 0x000000FF) << 24);  // Move byte 4 to byte 1


        /* writing frame content (new data for the edited tag, old data for others) */
        for (int i = 0; i < e_data_size-1; i++)
        {
            fprintf(info->fptr_duplicate_fname, "%c", data[i]);
        }
    }
    e_copy_data(info->fptr_source_fname,info->fptr_duplicate_fname); //copying remaining data from the source file
   
    fclose(info->fptr_duplicate_fname); // closing the files
    fclose(info->fptr_source_fname);
    /*instead of renaming the duplicate file just copied the new contents back to original file  */
    info->fptr_duplicate_fname = fopen(info->duplicate_fname,"r");
    info->fptr_source_fname = fopen(info->source_fname,"w");

     e_copy_data(info->fptr_duplicate_fname,info->fptr_source_fname); //copying remaining data from the source file
   
    fclose(info->fptr_duplicate_fname); // closing the files
    fclose(info->fptr_source_fname);

}
/* opens source file in read mode and duplicate file in write mode */
Status e_open_file(details *info)
{
    info->fptr_duplicate_fname = fopen(info->duplicate_fname,"w");
      if(info->fptr_duplicate_fname == NULL)
      {
        printf("ERROR: %s opening file failed\n",info->duplicate_fname);
        return failure;
      }

   info->fptr_source_fname = fopen(info->source_fname,"r");
      if(info->fptr_source_fname == NULL)
      {
        printf("ERROR: %s opening file failed\n",info->source_fname);
        return failure;
      }
      return success;
}
/* file position at the end gives the file size, less than 1 means empty */
Status e_check_size(FILE *fptr)
{
    fseek(fptr,0,SEEK_END);
    if(ftell(fptr)<1)
    {
       printf("ERROR:file is empty\n");
       return failure;
    }
    rewind(fptr);
    return success;
}
/* checks for "ID3" header and version 3 (ID3v2.3) */
Status e_check_version(FILE *fptr_source_fname)
{
    char buf[4];
    fread(buf,3,1,fptr_source_fname);
    buf[3]='\0';
    if(strcmp(buf,"ID3")!= 0)
    {
        printf("%s is a wrong version\n",buf);
        return failure;
    }
    fread(&e_data_size,2,1,fptr_source_fname);
    if(e_data_size != 3)
    {
        printf("%s version is different\n",buf);
        return failure;
    }
    return success;
}
/* reads the 4 byte frame id */
Status e_read_tag(details *info)
{
    fread(temp_tag,4,1,info->fptr_source_fname);
    temp_tag[4]='\0';
    return success;
}

/* reads the frame size, stored in big endian so bytes are reversed */
Status e_read_size(details *info)
{
    fread(&e_data_size, 4, 1, info->fptr_source_fname);

    // reversing the data size
    e_data_size = ((e_data_size & 0xFF000000) >> 24) | // Move byte 1 to byte 4
                  ((e_data_size & 0x00FF0000) >> 8) |  // Move byte 2 to byte 3
                  ((e_data_size & 0x0000FF00) << 8) |  // Move byte 3 to byte 2
                  ((e_data_size & 0x000000FF) << 24);  // Move byte 4 to byte 1

    // printf("%x\t",data_size);

    return success;
}
/* function to read data from file */
Status e_read_data(details *info)
{
    fread(flag, 3, 1, info->fptr_source_fname); // reading 3 bytes of flag from file(skipping)

    fread(data, e_data_size-1, 1, info->fptr_source_fname); // size-1 as first byte is the encoding byte

    return success;
}

/* function to copy the data from file */
Status e_copy_data(FILE *fptr_source, FILE *fptr_dest)
{
    while (!feof(fptr_source))
    {
        fread(data, 50, 1, fptr_source);
        fwrite(data, 50, 1, fptr_dest);
    }
}