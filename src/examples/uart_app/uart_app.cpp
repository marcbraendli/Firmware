
/**
 * @file uart_app.cpp
 * Testapplikation zur Inbetriebnahme der Serial4 Schnittstelle
 * mit der poll fkt
 *
 * @author Example User <mail@example.com>
 */

#include <px4_config.h>
#include <px4_posix.h>
#include <unistd.h>
#include <stdio.h>
#include <poll.h>
#include <string.h>
#include <assert.h>

#include <fcntl.h>
#include <termios.h>

#include <stdlib.h> //für fopen()!

//for readATfromSD
#define MAX_INPUT_LENGTH	20
#define MAX_INPUT_LENGTH	20
#define MAX_AT_COMMANDS		20
#define MAX_CHAR_PER_AT_COMMANDS 20
#define PATH                "/fs/microsd/toby/at-inits.txt"

typedef enum State
{
    Receive = 1,
    Send,
    Edit
}State;

extern "C" __EXPORT  int uart_app_main(int argc, char *argv[]);

void printAtCommands(char **atcommandbuffer, int atcommandbufferstand);
int  readATfromSD(char** atcommandbuffer);
int at_command_lenght(char *at_command);




int uart_app_main(int argc, char *argv[])
{

    PX4_INFO("This is a Uart Test App");

    int uart0_filestream =-1;
    uart0_filestream =open("/dev/ttyS6", (O_RDWR |O_NOCTTY | O_NONBLOCK));
    if(uart0_filestream == -1)
    {
       PX4_INFO("Unable to Open /dev/ttyS6");
    }
    PX4_INFO("open return value /dev/ttyS6: %d",uart0_filestream);

    struct termios options;
    tcgetattr(uart0_filestream, &options);
    options.c_iflag=0;
    options.c_iflag = IGNPAR;
    options.c_iflag &= ~(IGNBRK | BRKINT | ICRNL| INLCR | PARMRK | INPCK | ISTRIP | IXON);
    options.c_lflag=0;
    options.c_lflag &= ~(ECHO | ECHONL | ICANON | IEXTEN | ISIG);
    options.c_oflag=0;
    options.c_cflag=0;
    options.c_cflag &= ~(CSIZE | PARENB);
    options.c_cflag |= (CS8|CREAD|CLOCAL);

    cfsetispeed(&options,B57600);
    cfsetospeed(&options,B57600);

    tcflush(uart0_filestream, TCIFLUSH);

    if(tcsetattr(uart0_filestream, TCSANOW, &options)<0)
    {
        PX4_WARN("Wrong Options");
    }

    px4_pollfd_struct_t fds;
    fds.fd = uart0_filestream;
    fds.events = (POLLIN | POLLRDNORM);

    char    rx_buffer[100]  ="";
    char    output[100]     ="";
    char*   string_end      ='\0';
    int     received        =0;
    int     numberOfAT      =0;
    int     i               =0;
    int     lenght          =0;
    State   state           =Send;
    char   at_command_send[MAX_AT_COMMANDS][MAX_CHAR_PER_AT_COMMANDS];

    //Anpassung da malloc nicht funktioniert,
    //damit Funktionen weiterverwendet werden können
    char* at_command_sendp[MAX_AT_COMMANDS];
    for(int i=0; i < MAX_AT_COMMANDS; ++i)
        at_command_sendp[i] = &at_command_send[i][0];


    PX4_INFO("numberOfAT: %d",numberOfAT);
    PX4_INFO("i: %d",i);

    numberOfAT=readATfromSD(at_command_sendp);

    PX4_INFO("numberOfAT from return:: %d",numberOfAT);
    PX4_INFO("i: %d",i);

    while(numberOfAT > i)
    {
        switch(state)
        {
        case Send:
        {
            lenght= at_command_lenght(at_command_send[i]);

            PX4_INFO("Send: %.*s",lenght-1,at_command_send[i]);
            int count = write(uart0_filestream, at_command_send[i], lenght);

            if (count < 0)
            {
                PX4_INFO("UART TX error");
            }
            //usleep(0.1);

            state=Receive;
            break;
        }
        case Receive:
        {
            int poll_ret = px4_poll(&fds, 1, 500);

            if (poll_ret == 0)
            {
                PX4_INFO("Got no (more) datas!");
                if(received)
                {
                    state=Edit;
                }
            }else if(poll_ret <0){
                PX4_ERR("ERROR return value from poll(): %d", poll_ret);
            }else
            {
                if (fds.revents & (POLLIN | POLLRDNORM))
                {
                    int count = read(uart0_filestream, rx_buffer,100);
                    if (count < 0)
                    {
                        PX4_ERR("UART RX error");
                    }
                    strcat(output, rx_buffer);
                    strncpy(rx_buffer, string_end,100);
                    //PX4_INFO("RX-Buff check: %s", rx_buffer);
                    received =1;
                }
            }
            break;
        }
        case Edit://verarbeiten
        {
            PX4_INFO("Received: %s", output);
            i++;
            strncpy(output, string_end,100);


            if(i > numberOfAT){
                state=Receive;
            }
            else{
                state = Send;
            }

            received=0;
            break;
        }
        default:
            PX4_INFO("SWITCH STATE TROUBLE!!!");
            break;
        }
    }
    close(uart0_filestream);
    PX4_INFO("exit the Uart App");
    return 0;
}

/**
 * @brief gibt die Anzahl AT Commands zurück
 *
 * #define MAX_AT_COMMANDS beachten !!!
 */
int readATfromSD(char **atcommandbuffer)
{
    FILE*   sd_stream               = fopen(PATH, "r");
    int 	atcommandbufferstand    = 0;
    int 	inputbufferstand        = 0;
    int 	c                       =-1;
    char 	string_end              ='\0';
    char 	inputbuffer[MAX_INPUT_LENGTH]="";

    if(sd_stream) {
        PX4_INFO("SD-Karte offen");
        do { // read all lines in file
            do{ // read one line until EOF
                c = fgetc(sd_stream);
                if(c != EOF){
                    //(EOF = End of File mit -1 im System definiert)
                    inputbuffer[inputbufferstand]=(char)c;
                    inputbufferstand++;
                }
            }while(c != EOF && c != '\n');

            inputbuffer[inputbufferstand]='\r';
            inputbufferstand++;
            inputbuffer[inputbufferstand]=string_end; //wirklich Notwendig ?

            //Speicherplatz konnte nicht mit malloc alloziert werden! Griff auf ungültigen Speicherbereich zu.
            //atcommandbuffer[atcommandbufferstand] =(char*) malloc(20);
            assert(atcommandbuffer[atcommandbufferstand]!=0);
            strcpy(atcommandbuffer[atcommandbufferstand],inputbuffer);

            inputbufferstand=0;
            atcommandbufferstand++;
        } while(c != EOF);
    }else{
        PX4_INFO("SD-Karte NICHT offen");
    }

    fclose(sd_stream);
    PX4_INFO("SD-Karte geschlossen");

    atcommandbufferstand--;

    printAtCommands(atcommandbuffer,atcommandbufferstand);

    //PX4_INFO("atcommandbufferstand in readATfromSD fkt: %d",atcommandbufferstand);


    return atcommandbufferstand;
}

/**
 * @brief gibt die länge des AT-Commands zurück
 *
 * länge bis und mit '\r'
 */
int at_command_lenght(char *at_command)
{
    int k=0;
    while(at_command[k] != '\n')
    {
        k++;
    }

    return k;
}

/**
 * @brief Gibt die AT-Commands als PX4_Info aus
 *
 *
 */
void printAtCommands(char **atcommandbuffer, int atcommandbufferstand)
{
    PX4_INFO("%s beinhaltet %d Zeilen",PATH,atcommandbufferstand);
    for(int j=0 ; j < atcommandbufferstand ; j++)
    {
        PX4_INFO("Inhalt %d %s",j ,atcommandbuffer[j]);
    }
}
