#include<stdio.h>
#include<string.h>
#include "mp3.h"

unsigned int data_size;

/* reads and prints the first 6 frames (title, artist, album, year, music, comment) of the ID3v2.3 tag */
Status view_info(details *info)
{
    if(!(v_open_file(info)))
    {
        return failure;
    }
    if(!(v_check_size(info->fptr_source_fname)))
    {
        return failure;
    }
    if(!(v_check_version(info->fptr_source_fname)))
    {
        return failure;
    }

    
    printf(YELLOW "\n----------------- SELECTED VIEW DETAILS ----------------------\n\n" RESET);
    printf(MAGENTA "--------------------------------------------------------------\n" RESET);
    printf(CYAN "\t MP3 TAG READER AND EDITOR FOR ID3v2\n" RESET);
    printf(MAGENTA "--------------------------------------------------------------\n" RESET);

    fseek(info->fptr_source_fname,10,SEEK_SET); // skipping the 10 byte ID3 header, frames start after it
    for(int i=0;i<6;i++)
    {
        if(!(v_read_tag(info->fptr_source_fname))) //reading tag from file
        {
           return failure;
        }
        if(!(v_read_size(info->fptr_source_fname)))//reading data size for file
        {
        return failure;
        }
        if(!(v_read_data(info->fptr_source_fname)))// reading data from the file
        {
            return failure;
        }
    }
      printf(MAGENTA "--------------------------------------------------------------\n" RESET);
      return success;
}
   /* opens the mp3 file in read mode */
   Status v_open_file(details *info)
   {
      info->fptr_source_fname = fopen(info->source_fname,"r");
      if(info->fptr_source_fname == NULL)
      {
        printf("ERROR: %s opening file failed\n",info->source_fname);
        return failure;
      }

      return success;
   }

/* file position at the end gives the file size, less than 1 means empty */
Status v_check_size(FILE *fptr)
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
Status v_check_version(FILE *fptr_source_fname)
{
    char buf[4];
    fread(buf,3,1,fptr_source_fname);
    buf[3]='\0';
    if(strcmp(buf,"ID3")!= 0)
    {
        printf("%s is a wrong version\n",buf);
        return failure;
    }
    fread(&data_size,2,1,fptr_source_fname);
    if(data_size != 3)
    {
        printf("%s version is different\n",buf);
        return failure;
    }
}
/* reads the 4 byte frame id and prints the matching label */
Status v_read_tag(FILE *fptr_source_fname)
{
    char buf[5];

    fread(buf, 4, 1, fptr_source_fname);
    buf[4] = '\0';

    if (strcmp(buf, "TIT2") == 0)
        printf("TITLE\t:\t");
    else if (strcmp(buf, "TPE1") == 0)
        printf("ARTIST\t:\t");
    else if (strcmp(buf, "TALB") == 0)
        printf("Album\t:\t");
    else if (strcmp(buf, "TYER") == 0)
        printf("YEAR\t:\t");
    else if (strcmp(buf, "TCON") == 0)
        printf("MUSIC\t:\t");
    else if (strcmp(buf, "COMM") == 0)
        printf("COMMENT\t:\t");
    else
    {
        buf[4] = '\0';
        printf("INFO: ERROR: %s tag not matched\n", buf);
        return failure;
    }

    return success;
}
/* function to read the size of data */
Status v_read_size(FILE *fptr_source_fname)
{
    fread(&data_size, 4, 1, fptr_source_fname);

    // reversing the data size
    data_size = ((data_size & 0xFF000000) >> 24) | // Move byte 1 to byte 4
                  ((data_size & 0x00FF0000) >> 8) |  // Move byte 2 to byte 3
                  ((data_size & 0x0000FF00) << 8) |  // Move byte 3 to byte 2
                  ((data_size & 0x000000FF) << 24);  // Move byte 4 to byte 1

    // printf("%x\t",data_size);

    return success;
}
/* function to read data based on size*/
Status v_read_data(FILE *fptr_souce_fname)
{
    fseek(fptr_souce_fname, 3, SEEK_CUR); // skipping the 3 bytes flag
    char buf[data_size];
    fread(buf, data_size - 1, 1, fptr_souce_fname); // size-1 as first byte is the encoding byte
    buf[data_size - 1] = '\0';

    for (int i = 0; i < data_size-1; i++)
    {
        printf("%c", buf[i]);
    }

    printf("\n");

    return success;
}