#include "TFTP.h"

int main()
{
    int choice;
    char ip[50];
    int port_num;
    char mode[MODE_SIZE] = "Normal";
    char filename[FILENAME_SIZE];
    do
    {
        printf("-----------------------------------------------------------\n");
        printf("--------------------------MENU------------------------------\n");
        printf("1.Connect\n");
        printf("2.Put\n");
        printf("3.Get\n");
        printf("4.Mode(Normal/octet/Net-ascii)\n");
        printf("5.exit\n");
        printf("--------------------------------------------------------------\n");
        scanf("%d",&choice);
        switch(choice)
        {
            case 1://connect function call
            printf("Enter IP adderss\n");
            scanf("%s",ip);
            printf("Enter the port number\n");
            scanf("%d",&port_num);
            connect_to_server(ip,port_num);
            break;

            case 2: //put function call
                 printf("Enter file name\n");
                 scanf("%s", filename);
                 put_file(filename,mode);
            break;

            case 3: //get function call
                  printf("Enter file name\n");
                  scanf("%s", filename);
                  get_file(filename,mode);
            break;

            case 4: //mode function call
            set_mode(mode);
            break;

            case 5: //exit
            break;

            default:
            break;
        }
    }while(choice != 5);

    return 0;
}