#ifndef MP3_H
#define MP3_H

typedef struct details
{
    char *source_fname;
    FILE *fptr_source_fname;

    char *duplicate_fname;
    FILE *fptr_duplicate_fname;

    char change_data[100];
    char tag[5];
}details;

typedef enum{
    failure,
    success
}Status;

#define RESET       "\x1b[0m"
#define RED         "\x1b[31m"
#define GREEN       "\x1b[32m"
#define YELLOW      "\x1b[33m"
#define BLUE        "\x1b[34m"
#define MAGENTA     "\x1b[35m"
#define CYAN        "\x1b[36m"


 Status validate_args(int argc,char *argv[],details *info);

 
/* view related functions */

/* to check the size of file(empty or not) */
Status v_check_size(FILE *fptr);

/* to view information from mpeg file */
Status view_info(details *info);

/* to edit the information in mpeg file */
Status edit_info(details *info);

/* to opening the files */
Status v_open_file(details *info);

/* to verify the version */
Status v_check_version(FILE *fptr_source_fname);

/* to read the tags */
Status v_read_tag(FILE *fptr_source_fname);

/* to read size of data */
Status v_read_size(FILE *fptr_source_fname);

/* to read data from file */
Status v_read_data(FILE *fptr_source_fname);


/* edit related functions */
/* to check size of file(empty or not)*/
Status e_check_size(FILE *fptr);

/* to open the files */
Status e_open_file(details *info);

/* to check the version */
Status e_check_version(FILE *fptr_source_fname);

/* to read the tags */
Status e_read_tag(details *info);

/* to read the size of data */
Status e_read_size(details *info);

/* to read the data from file */
Status e_read_data(details *info);

/* to copy remaining data from file */
Status e_copy_data(FILE *fptr_source, FILE *fptr_dest);

#endif