//NAME: Rohan Rao
//EMAIL: raokrohan@gmail.com
//ID: 305339928

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
//incudes for open
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
//includes for pread
#include <unistd.h>
//for errno
#include <errno.h>
//for getopt_long
#include <getopt.h>
//for poll
#include <poll.h>
//for mraa library
#include <mraa.h>
//for time functions
#include <time.h>
//for detecting digits
#include <ctype.h>
//for log
#include <math.h>
//for socket
#include <sys/socket.h>
//for tcp
#include <netdb.h>
#include <netinet/in.h>

//some definitions
#define CHUNK 128000

#define CENTEGRADE 0 //celius is 0
#define FAHRENHEIT 1 //Farenheight is 1 (default)
#define INVALID_SCALE 3 //invalide scale provided

#define B 4275 //thermistor value 
#define R0 100000.0 //nominal base value

#define A0 1 //pin #1
#define GPIO_50 60 //pin #2

//GLOBAL VARS GO HERE:
time_t g_next_time = 0;
int g_rawtemp = 0;
int sockfd = -1;
char shutdownbuf[100];

int p_opt = 1, s_opt = FAHRENHEIT; //port, log, and compress options

//MANDATORY OPTIONS
int l_opt = 0, i_opt = 0, h_opt = 0, portnum = -1;


int g_report = 1; // 1 - report; 0 - don't
int g_shutdown = 0; // 1 - print last time; 0 - regular

static struct option long_options[] = {
    //{const char *name, int has_arg, int *flag, int val},
    {"period", required_argument, 0, 'p'},
    {"scale", required_argument, 0, 's'},
    {"log", required_argument, 0, 'l'},
    {"id", required_argument, 0, 'i'},
    {"host", required_argument, 0, 'h'},
    {0,0,0,0}
};

int scaleToVal(char c)
{
    if(c == 'C')
        return CENTEGRADE;
    if(c == 'F')
        return FAHRENHEIT;
    return INVALID_SCALE;
}

float rawToTemp(int raw)
{
    float R = 1023.0/((float) raw) - 1.0;
    R = R0*R;
    //C is the temperature in Celcious
    float C = 1.0/(log(R/R0)/B + 1/298.15) - 273.15;
    //F is the temperature in Fahrenheit
    float F = (C * 9)/5 + 32;
    if(s_opt == FAHRENHEIT)
        return F;
    else if (s_opt == CENTEGRADE)
        return C;
    else
        return -1;
}

void print_cur_report() //mode 1 prints temp, mode 0 doesn't
{
    struct timespec ts;
    struct tm * tm;
    clock_gettime(CLOCK_REALTIME, &ts);
    tm = localtime(&(ts.tv_sec));

    if(g_shutdown)
    {
        sprintf(shutdownbuf, "%02d:%02d:%02d", tm->tm_hour, tm->tm_min, tm->tm_sec);
        return;
    }

    if(g_report && (ts.tv_sec >= g_next_time))
    {
        //fprintf(stderr, "%02d:%02d:%02d %0.1f\n", tm->tm_hour, tm->tm_min, tm->tm_sec, rawToTemp(g_rawtemp));
        dprintf(l_opt, "%02d:%02d:%02d %0.1f\n", tm->tm_hour, tm->tm_min, tm->tm_sec, rawToTemp(g_rawtemp));
        dprintf(sockfd, "%02d:%02d:%02d %0.1f\n", tm->tm_hour, tm->tm_min, tm->tm_sec, rawToTemp(g_rawtemp));
        g_next_time = ts.tv_sec + (time_t)p_opt;
    }
}

void myshutdown()
{
    g_shutdown = 1;
    g_report = 1;
    print_cur_report();
    //fprintf(stderr, "%s SHUTDOWN\n", shutdownbuf);
    dprintf(l_opt, "%s SHUTDOWN\n", shutdownbuf);
    dprintf(sockfd, "%s SHUTDOWN\n", shutdownbuf);
    exit(0);
}

void process_command(char input[])
{   
    write(l_opt, input, strlen(input));
    //fprintf(stderr, "%s", input);
    //write(sockfd, input, strlen(input));
    input[strlen(input) - 1] = '\0';
    while(*input == ' ')
        input++;

    if(!strcmp(input, "SCALE=F")) {
        s_opt = FAHRENHEIT;
        return;
    } else if (!strcmp(input, "SCALE=C")) {
        s_opt = CENTEGRADE;
        return;
    } else if (!strcmp(input, "STOP")) {
        g_report = 0;
        return;
    } else if (!strcmp(input, "START")) {
        g_report = 1;
        return;
    } else if (!strcmp(input, "OFF")) {
        myshutdown();
        return;
    }
    
    /*
    if(input[0] == 'L' && input[1] == 'O' && input[2] == 'G')
    {
        //fprintf(stderr, "%s\n", input);
        dprintf(l_opt, "%s\n", input);
        return;
    }
    */
   
    char* pstart = strstr(input, "PERIOD=");
    if(pstart == NULL)
        return;
    pstart+=7;
    char pstring[CHUNK];
    int i;
    for(i = 0; i < CHUNK; i++)
        pstring[i] = '\0';
    int it = 0;
    while(isdigit(*pstart))
    {
        pstring[it] = *pstart;
        pstart++;
        it++;
    }
    if(it == 0)
        return;
    //printf("PERIOD: %d\n", atoi(pstring));
    p_opt = atoi(pstring);

}

//Discussion 1B Week9

int client_connect(char * host_name, unsigned int port)
//e.g. host_name:”lever.cs.ucla.edu”, port:18000, return the socket for subsequent communication
{
    struct sockaddr_in serv_addr; //encode the ip address and the port for the remote server
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    // AF_INET: IPv4, SOCK_STREAM: TCP connection
    struct hostent *server = gethostbyname(host_name);
    // convert host_name to IP addr
    memset(&serv_addr, 0, sizeof(struct sockaddr_in));
    serv_addr.sin_family = AF_INET; //address is Ipv4
    memcpy(&serv_addr.sin_addr.s_addr, server->h_addr, server->h_length);
    //copy ip address from server to serv_addr
    serv_addr.sin_port = htons(port); //setup the port
    connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)); //initiate the connection to server
    return sockfd;
}



int main(int argc, char* argv[])
{
    //printf("MRAA VERSION: %s\n", mraa_get_version());
int opt = -1; //default if there are no options
int option_index = 0; //needed for getopt_long
char *periodarg = "", *logfilename = "", *scale = "", *hostname = ""; //port number and logging filename

//-----------------------------
//begin arg parse

while(1)
{
    opt = getopt_long(argc, argv, "", long_options, &option_index);
    if(opt == -1)
        break;
    switch(opt)
    {
        case 0:
            printf("0 was called\n");
            break;
        case 'p':
            periodarg = optarg;
            p_opt = atoi(periodarg);
            if(p_opt <= 0)
            {
                fprintf(stderr, "Period must be a positive, nonnegative int\n");
                exit(1);
            }
            break;
        case 's':
            scale = optarg;
            if (strlen(scale) > 1 || scaleToVal(scale[0]) == INVALID_SCALE)
            {
                fprintf(stderr, "Scale must be C or F\n");
                exit(1);
            }
            s_opt = scaleToVal(scale[0]);
            break;
        case 'l':
            logfilename = optarg;
            l_opt = open(logfilename, O_RDWR | O_CREAT | O_TRUNC, 0777);
            if(l_opt == -1)
            {
                    fprintf(stderr, "Problem with opening/creating '%s' for logging\n%s\n", logfilename, strerror(errno));
                    exit(1);
            }
            break;
        case 'h':
            h_opt = 1;
            hostname = optarg;
            break;
        case 'i':
            i_opt = atoi(optarg);
            break;
        case '?':
            fprintf(stderr, "Usage: %s --id=9-digit-number --host=name-or-address --log=filename portnumber --period=# --scale=''\n", argv[0]);
            exit(1);
            break;
        default:
            printf("default was called\n");
    }
}


if (optind < argc) 
{
    portnum = atoi(argv[optind]);
    if (portnum < 1) 
    {
        fprintf(stderr, "Port invalid\n");
        exit(1);
    }
}

if(!l_opt || !h_opt || i_opt < pow(10, 8) || portnum < 1)
{
    fprintf(stderr, "Mandatory use of command line options:\n--id=9-digit-number --host=name-or-address --log=filename portnumber\n");
    exit(1);
}
//end arg parse
//-----------------------------

//fprintf(stderr, "Attempting to establish connection...\n");
sockfd = client_connect(hostname, portnum);
//fprintf(stderr, "Socketfd: '%d'\n", sockfd);
dprintf(sockfd, "ID=%d\n", i_opt);
dprintf(l_opt, "ID=%d\n", i_opt);

FILE *tempstream = fdopen(sockfd, "r");
if (tempstream == NULL)
{
    fprintf(stderr, "null stream\n");
    exit(1);
}

//Initializing sensors

//init button
mraa_gpio_context button = mraa_gpio_init(GPIO_50);
mraa_gpio_dir(button, MRAA_GPIO_IN);
mraa_gpio_isr(button, MRAA_GPIO_EDGE_RISING, &myshutdown, NULL);

//init thermistor
mraa_aio_context thermistor = mraa_aio_init(A0);

struct pollfd command;
command.fd = sockfd;
command.events = POLLIN;

char pollbuf[CHUNK];
while(1)
{
    int ret = poll(&command, 1, 1000);
    if(ret < 0)
        break;
    g_rawtemp = mraa_aio_read(thermistor);
    print_cur_report();
    if(ret >= 1)
    {
        fgets(pollbuf, CHUNK, tempstream);
        process_command(pollbuf);
    }
}

//close devices
mraa_gpio_close(button);
mraa_aio_close(thermistor);
    exit(0);
}