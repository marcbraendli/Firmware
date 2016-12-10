
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

#include <fcntl.h>
#include <termios.h>

#include <stdlib.h> //für fopen()!

#include <./drivers/toby/tobyDevice.h>

//for readATfromSD
#define MAX_INPUT_LENGTH	20
#define MAX_LINE_NUMBER		10
#define PATH                "/fs/microsd/toby/at-inits.txt"

typedef enum State
{
    Receive = 1,
    Send,
    Edit
}State;

extern "C" __EXPORT  int uart_app_main(int argc, char *argv[]);

void readATfromSD();
int at_command_lenght(const char* at_command);

unsigned char at_buffer[2][MAX_LINE_NUMBER][MAX_INPUT_LENGTH]={};

int uart_app_main(int argc, char *argv[])
{

    TobyDevice* myToby =new TobyDevice();


    PX4_INFO("This is a Uart Test App");


    //int uart0_filestream =-1;

    /*uart0_filestream =open("/dev/ttyS6", (O_RDWR |O_NOCTTY | O_NONBLOCK));

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
    }*/

    //px4_pollfd_struct_t fds;
    //fds.fd = uart0_filestream;
    //fds.events = (POLLIN | POLLRDNORM);

    char rx_buffer[100]="";
    char output[100]="";
    char* string_end ='\0';
    int received =0;


#define NUMBER_OF_AT_COMMANDS 18

    const char *at_command_send[NUMBER_OF_AT_COMMANDS]={"ATE0\r",
                                                        "AT+IPR =57600\r",
                                                        "AT+CMEE=2\r",
                                                        "AT+CGMR\r",
                                                        "ATI9\r",
                                                        "AT+CPIN=\"4465\"\r",
                                                        "AT+CLCK=\"SC\",2\r",
                                                        "AT+CREG=2\r",
                                                        "AT+CREG=0\r",
                                                        "AT+CSQ\r",
                                                        "AT+UREG?\r",
                                                        "AT+CPIN=\"4465\"\r",
                                                        "AT+CLCK=\"SC\",2\r",
                                                        "AT+CGMR\r",
                                                        "ATI9\r",
                                                        "at+upsd=0,100,3\r",
                                                        "at+upsda=0,3\r",
                                                        "at+uping=\"www.google.ch\"\r",

                                                       };

    //int at_command_send_size[]={5,8,8,5,8,9,15,15,16,13};

    //const char *at_command_send[5]={"ATE0\r","AT+CPIN=\"4465\"\r","AT+UPSD=0,100,4\r","AT+UPSDA=0,3\r","AT+UPING=\"www.google.com\"\r"};
    //int at_command_send_size[5]={5,15,16,13,26};

    int i =0;
    int lenght =0;
    State state =Send;
    while(i<NUMBER_OF_AT_COMMANDS)
    {

        switch(state)
        {
        case Send:
        {
            lenght= at_command_lenght(at_command_send[i]);
            //PX4_INFO("Sendsize: %i vs. length %i", at_command_send_size[i],lenght);

            //PX4_INFO("Send: %.*s",at_command_send_size[i]-1,at_command_send[i]);
            //int count = write(uart0_filestream, at_command_send[i], at_command_send_size[i]);

             PX4_INFO("Send: %.*s",lenght-1,at_command_send[i]);
            int count = myToby->write(at_command_send[i], lenght);



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
            int poll_ret = myToby->poll(0);

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
                if (poll_ret > 0)
                {
                    int count = myToby->read(rx_buffer,100);

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
            //PX4_INFO("Output check: %s", output);
            if(i > NUMBER_OF_AT_COMMANDS){
               state=Receive;
            }
            else{
                state = Send;
            }

            received=0;
            break;
        }
        default:
            break;
        }
    }
    delete myToby;
    PX4_INFO("exiting the Uart App");

    return 0;
}

/*void readATfromSD()
{
    FILE *f = fopen(PATH, "r");
    int pos = 	0;
    int line=	0;
    int buf=	0;
    int c;         //Charakter Zwischenspeicher muss negativ werden können
                    //return von fgetc() ist -1 für das ende der Zeile
    unsigned char buffer[2][MAX_LINE_NUMBER][MAX_INPUT_LENGTH]={};

    if(f) {
        PX4_INFO("SD-Karte offen");
        //std::cout<<"SD-Karte offen"<<std::endl;
        do { // read all lines in file
            pos = 0;
            do{ // read one line until EOF
                c = fgetc(f);
                if(c ==':')
                {
                    //std::cout<<"Bufferchange to 1 because of :"<<std::endl;
                    buf=1;
                }else if(c != EOF){
                    //std::cout<<"c="<<c<<std::endl;
                    buffer[buf][line][pos] = (char)c;
                    pos++;
                }else{}
            }while(c != EOF && c != '\n');
            line++;
            //std::cout<<"Bufferchange back to 0"<<std::endl;
            buf=0;
        } while(c != EOF);
    }else{
        PX4_INFO("SD-Karte NICHT offen");
        //std::cout<<"SD-Karte NICHT offen"<<std::endl;
    }
    fclose(f);

    //std::cout<<"myfile  beinhaltet "<<line-1<<" Zeilen"<<std::endl;
    PX4_INFO("myfile  beinhaltet %d Zeilen", (line-1));
    //std::cout<<"Anfragen:"<<std::endl;
    PX4_INFO("Anfragen");

    for(int i=0; i <line ; i++)
    {
        for(int j=0; j < MAX_INPUT_LENGTH; j++)
        {
            //std::cout << buffer[0][i][j];
            PX4_INFO("%c",buffer[0][i][j]);
        }
        //std::cout<<std::endl;
    }
    //std::cout<<"Antworten:"<<std::endl;

    for(int i=0; i <line ; i++)
    {
        for(int j=0; j < MAX_INPUT_LENGTH; j++)
        {
            //std::cout << buffer[1][i][j];
            PX4_INFO("%c",buffer[1][i][j]);
        }
    }
}*/

int at_command_lenght(const char* at_command)
{
   int k=0;
   while(at_command[k] != '\r')
    {
        k++;
    }

    return k+1;
}








