#include<stdio.h>
#include<string.h>
#include "mp3.h"

int main(int argc,char *argv[])
{
    details info;

    /* validating the command line arguments before doing anything */
    if(!(validate_args(argc,argv,&info)))
    {
        printf("error\n");
        return failure;
    }
    else
    {
        /* -v : view the tags of the mp3 file */
        if(strcmp(argv[1], "-v")==0)
        {
             if(argc>2)
             {
                info.source_fname = argv[2];
             }
             else
             {
                info.source_fname = "sample.mp3"; // default file when no file name is given
             }
              if (view_info(&info) == failure)
            {
               return failure;
            }
        }
        /* -e : edit a tag, option decides which frame id is to be changed */
        else if(strcmp(argv[1], "-e")==0)
        {
            if(strcmp(argv[2],"-t") == 0)
            {
                strcpy(info.tag,"TIT2");
                strcpy(info.change_data,argv[3]);
            }
            else if(strcmp(argv[2],"-a") == 0)
            {
                strcpy(info.tag,"TPE1");
                strcpy(info.change_data,argv[3]);
            }
            else if(strcmp(argv[2],"-A") == 0)
            {
                strcpy(info.tag,"TALB");
                strcpy(info.change_data,argv[3]);
            }
            else if(strcmp(argv[2],"-y") == 0)
            {
                strcpy(info.tag,"TYER");
                strcpy(info.change_data,argv[3]);
            }
            else if(strcmp(argv[2],"-m") == 0)
            {
                strcpy(info.tag,"TCON");
                strcpy(info.change_data,argv[3]);
            }
            else if(strcmp(argv[2],"-c") == 0)
            {
                strcpy(info.tag,"COMM");
                strcpy(info.change_data,argv[3]);
            }
            if (edit_info(&info) == failure)
            {
               return failure;
            }
        }
        else
        {
            printf("ERROR: %s is unsupported command\n", argv[1]);
            return failure;
        }
         
    }
}

/* checks the command line arguments and the mp3 file name for view and edit */
Status validate_args(int argc,char *argv[],details *info)
{
    /* no arguments given, print the usage */
    if(argc < 2)
    {
        printf("\n---------------------------------------------\n");
        printf("ERROR: ./a.out :invalid arguments\n");
        printf("usage:\n");
        printf("To view pass : ./a.out -v <mp3_fname>\n");
        printf("To edit  pass : ./a.out -e -t/-a/-A/-y/-m/-c changing_text <mp3_fname>\n");
        printf("for any help pass : ./a.out --help\n");
        printf("\n----------------------------------------------\n");
        return failure;
    }
    if (strcmp(argv[1], "--help") == 0)  // --help command menu
    {
        printf("\n--------------------HELP MENU--------------------------\n\n");
        printf("1. -v -> to view mp3 file contents\n");
        printf("2. -e -> to edit mp3 file contents\n");
        printf("\t2.1. -t -> to edit song title\n");
        printf("\t2.2. -a -> to edit artist name\n");
        printf("\t2.3. -A -> to edit album name\n");
        printf("\t2.4. -y -> to edit year\n");
        printf("\t2.5. -m -> to edit content\n");
        printf("\t2.6. -c -> to edit comment\n");
        printf("-----------------------------------------------------\n\n" );

        return failure;
    }
    /* view : ./a.out -v <mp3_fname> */
    if(strcmp(argv[1],"-v") == 0)
    {
        if(argc<3)
        {
            printf("Error: ./a.out -v <mp3_fname>\n");
            return failure;
        }
        info->source_fname=strstr(argv[2],".mp3"); // finding ".mp3" in the file name
        if(info->source_fname == NULL)
        {
            printf("error: %s is  not a <mp3file>\n",argv[2]);
            return failure;
        }
        /* ".mp3" must be at the end of the name, otherwise it is not a valid mp3 file */
        if(strcmp(info->source_fname,".mp3")== 0)
        {
            info->source_fname = argv[2];
        }
       
        else
        {
             printf("error: %s is  not a <mp3file>\n",argv[2]);
            return failure;
        }
        return success;
     }

     /* edit : ./a.out -e -t/-a/-A/-y/-m/-c <changing_text> <mp3_fname> */
     if(strcmp(argv[1],"-e")==0)
    {
        if(argc<5)
        {
             printf("EDIT: ./a.out -e -t/-a/-A/-y/-m/-c <changing_text> <mp3 file_name>\n");
            return failure;
        }
        else
        {
        info->source_fname=strstr(argv[4],".mp3");
        if(info->source_fname == NULL)
        {
            printf("error: %s is  not a <mp3file>\n",argv[4]);
            return failure;
        }
        if(strcmp(info->source_fname,".mp3")== 0)
        {
            info->source_fname = argv[4];
        }
        else
        {
             printf("error: %s is  not a <mp3file>\n",argv[4]);
            return failure;
        }
        }
        /* validating the edit option */
        if(!((strcmp(argv[2],"-t"))|| (strcmp(argv[2],"-a"))||(strcmp(argv[2],"-A"))||(strcmp(argv[2],"-y"))||(strcmp(argv[2],"-m"))||(strcmp(argv[2],"-c"))))
        {
            printf("%s is a invalid command\n",argv[2]);
            return failure;
        }
        return success;
     }
}