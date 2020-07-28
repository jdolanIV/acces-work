/*
    writeread.c - based on writeread.cpp
    [SOLVED] Serial Programming, Write-Read Issue - http://www.linuxquestions.org/questions/programming-9/serial-programming-write-read-issue-822980/

    build with: gcc -o writeread -lpthread -Wall -g writeread.c
*/

#include <stdio.h>
#include <string.h>
#include <stddef.h>

#include <stdlib.h>
#include <sys/time.h>

#include <pthread.h>
#include <semaphore.h>

#include "writeread.h"


int serport_fd;

//POSIX Threads Programming - https://computing.llnl.gov/tutorials/pthreads/#PassingArguments
struct write_thread_data{
   int  fd;
   char* comm; //string to send
   int bytesToSend;
   int writtenBytes;
   int iator; // Initiator.  0 = False, 1 = True
   pthread_cond_t *print;
   pthread_mutex_t *lock;
   sem_t *write;
};

struct print_thread_data {
    pthread_cond_t print;
    pthread_mutex_t lock;
    sem_t write;
    sem_t read;
    int terminate;
};

void usage(char **argv)
{
    fprintf(stdout, "Usage:\n"); 
    fprintf(stdout, "%s port baudrate file/string [-I]\n", argv[0]); 
    fprintf(stdout, "   The -I is for initiator.  Run on the remote side which "
                    "will wait, then start locally with -I which will initiate "
                    "the test.\n");
    fprintf(stdout, "Examples:\n");
    fprintf(stdout, "%s /dev/ttyUSB0 115200 /path/to/somefile.txt\n", argv[0]); 
    fprintf(stdout, "%s /dev/ttyUSB0 115200 \"some text test\"\n", argv[0]); 
}

void *print_thread_function(void *arg)
{
    struct print_thread_data *my_data;

    my_data = (struct print_thread_data *) arg;

    while (!my_data->terminate)
    {
        pthread_mutex_lock(&my_data->lock);
        pthread_cond_wait(&my_data->print, &my_data->lock);
        pthread_mutex_unlock(&my_data->lock);
        while (sem_trywait(&my_data->read) == 0)
        {
            fprintf(stdout, "R");
        }
        while (sem_trywait(&my_data->write) == 0)
        {
            fprintf(stdout, "W");
        }
        fflush(stdout);
    }
    return NULL;
}

// POSIX threads explained - http://www.ibm.com/developerworks/library/l-posix1.html
// instead of writeport
void *write_thread_function(void *arg) {
    int go = 0;
    int lastBytesWritten;
    struct write_thread_data *my_data;
    my_data = (struct write_thread_data *) arg;

    fprintf(stdout, "write_thread_function spawned\n");

    // Are we the initiator?
    if (my_data->iator == 1) {
        // We are the initiator, send the start command
        go = 0xde;
        write(my_data->fd, &go, 1);
    } else {
        // We wait for the initiator to send us the start command
        fprintf(stdout, "Waiting for initiator (start other end with -I)...\n");
        read(my_data->fd, &go, 1);
        if (go == 0xde) {
            fprintf(stdout, "Go!\n");
        } else {
            fprintf(stdout, "Error: Did not receive start command [0x%x]\n", go);
            return NULL;
        }
    }

    my_data->writtenBytes = 0; 
    while(my_data->writtenBytes < my_data->bytesToSend)
    {
        lastBytesWritten = write( my_data->fd, my_data->comm + my_data->writtenBytes, my_data->bytesToSend - my_data->writtenBytes );   
        my_data->writtenBytes += lastBytesWritten;  
        if ( lastBytesWritten < 0 ) 
        {
            fprintf(stdout, "write failed!\n");
            return 0;
        }
        //fprintf(stderr, "   write: %d - %d\n", lastBytesWritten, my_data->writtenBytes);
        pthread_mutex_lock(my_data->lock);
        sem_post(my_data->write);
        pthread_cond_signal(my_data->print);
        pthread_mutex_unlock(my_data->lock);
    }
    return NULL; //pthread_exit(NULL)
}

int main( int argc, char **argv ) 
{

    if( argc < 4 ) { 
        usage(argv);
        return 1; 
    }

    char *serport;
    char *serspeed;
    speed_t serspeed_t;
    char *serfstr;
    int serf_fd; // if < 0, then serfstr is a string
    int sentBytes; 
    int readChars;
    int recdBytes, totlBytes; 

    char* sResp;
    char* sRespTotal;

    struct timeval timeStart, timeEnd, timeDelta;
    float deltasec, expectBps, measReadBps, measWriteBps; 

    struct write_thread_data wrdata;
    pthread_t myWriteThread;

    struct print_thread_data print_data;
    pthread_t print_thread;

    pthread_cond_init(&print_data.print, NULL);
    pthread_mutex_init(&print_data.lock, NULL);
    sem_init(&print_data.read, 0, 0);
    sem_init(&print_data.write, 0, 0);
    print_data.terminate = 0;

    pthread_create(&print_thread, NULL, print_thread_function, &print_data);

    /* Re: connecting alternative output stream to terminal - 
    * http://coding.derkeiler.com/Archive/C_CPP/comp.lang.c/2009-01/msg01616.html 
    * send read output to file descriptor 3 if open, 
    * else just send to stdout
    */
    FILE *stdalt;
    if(dup2(3, 3) == -1) {
        fprintf(stdout, "stdalt not opened; ");
        stdalt = fopen("/dev/tty", "w");
    } else {
        fprintf(stdout, "stdalt opened; ");
        stdalt = fdopen(3, "w");
    }
    fprintf(stdout, "Alternative file descriptor: %d\n", fileno(stdalt));

    // Get the PORT name
    serport = argv[1];
    fprintf(stdout, "Opening port %s;\n", serport);

    // Get the baudrate
    serspeed = argv[2];
    serspeed_t = string_to_baud(serspeed);
    fprintf(stdout, "Got speed %s (%d/0x%x);\n", serspeed, serspeed_t, serspeed_t);

    //Get file or command;
    serfstr = argv[3];

    // Are we the initiator?
    if (argc == 5 &&
        strncmp(argv[4], "-I", 3) == 0 )
    {
        wrdata.iator = 1; // Initiator.  0 = False, 1 = True
    } else {
        wrdata.iator = 0; // Initiator.  0 = False, 1 = True
    }

    serf_fd = open( serfstr, O_RDONLY );
    fprintf(stdout, "Got file/string '%s'; ", serfstr);
    if (serf_fd < 0) {
        wrdata.bytesToSend = strlen(serfstr);
        wrdata.comm = serfstr; //pointer already defined 
        fprintf(stdout, "interpreting as string (%d).\n", wrdata.bytesToSend);
    } else {
        struct stat st;
        stat(serfstr, &st);
        wrdata.bytesToSend = st.st_size;
        wrdata.comm = (char *)calloc(wrdata.bytesToSend, sizeof(char));
        read(serf_fd, wrdata.comm, wrdata.bytesToSend);
        fprintf(stdout, "opened as file (%d).\n", wrdata.bytesToSend);
    }

    sResp = (char *)calloc(wrdata.bytesToSend, sizeof(char));
    sRespTotal = (char *)calloc(wrdata.bytesToSend, sizeof(char));

    // Open and Initialise port
    serport_fd = open( serport, O_RDWR | O_NOCTTY | O_NONBLOCK );
    if ( serport_fd < 0 ) { perror(serport); return 1; }
    initport( serport_fd, serspeed_t );

    wrdata.fd = serport_fd;

    sentBytes = 0; recdBytes = 0;

    gettimeofday( &timeStart, NULL );

    wrdata.write = &print_data.write;
    wrdata.lock = &print_data.lock;

    // start the thread for writing.. 
    if ( pthread_create( &myWriteThread, NULL, write_thread_function, (void *) &wrdata) ) {
        printf("error creating thread.");
        abort();
    }

    // run read loop 
    int loop_count =0;
    while ( recdBytes < wrdata.bytesToSend )
    {

        while ( wait_flag == TRUE );

        if ( (readChars = read( serport_fd, sResp, wrdata.bytesToSend)) >= 0 ) 
        {
            //~ fprintf(stdout, "InVAL: (%d) %s\n", readChars, sResp);
            // binary safe - add sResp chunk to sRespTotal
            memmove(sRespTotal+recdBytes, sResp+0, readChars*sizeof(char));
            /* // text safe, but not binary:
            sResp[readChars] = '\0'; D
            fprintf(stdalt, "%s", sResp);
            */
            recdBytes += readChars;
        } else {
            if ( errno == EAGAIN ) 
            {
                fprintf(stdout, "SERIAL EAGAIN ERROR\n");
                return 0;
            } 
            else 
            {
                fprintf(stdout, "SERIAL read error: %d = %s\n", errno , strerror(errno));
                return 0;
            }           
        }
        if (loop_count % 100 == 0)
        {
            //fprintf(stderr, "   read: %d\n", recdBytes);
            pthread_mutex_lock(&print_data.lock);
            sem_post(&print_data.read);
            pthread_cond_signal(&print_data.print);
            pthread_mutex_unlock(&print_data.lock);
        }
        loop_count++;

        wait_flag = TRUE; // was ==
        //~ usleep(50000);
    }

    pthread_mutex_lock(&print_data.lock);
    print_data.terminate = 1;
    pthread_cond_signal(&print_data.print);
    pthread_mutex_unlock(&print_data.lock);

    if ( pthread_join ( myWriteThread, NULL ) ) {
        printf("error joining write thread.");
        abort();
    }

    if ( pthread_join ( print_thread, NULL ) ) {
        printf("error joining print thread.");
        abort();
    }

    gettimeofday( &timeEnd, NULL );

    // binary safe - dump sRespTotal to stdalt
    fwrite(sRespTotal, sizeof(char), recdBytes, stdalt);

    // Close the open port
    close( serport_fd );
    if (!(serf_fd < 0)) { 
        close( serf_fd );
        free(wrdata.comm); 
    } 
    free(sResp);
    free(sRespTotal);

    fprintf(stdout, "\n+++DONE+++\n");

    sentBytes = wrdata.writtenBytes; 
    totlBytes = sentBytes + recdBytes;
    timeval_subtract(&timeDelta, &timeEnd, &timeStart);
    deltasec = timeDelta.tv_sec+timeDelta.tv_usec*1e-6;
    expectBps = atoi(serspeed)/10.0f; 
    measWriteBps = sentBytes/deltasec;
    measReadBps = recdBytes/deltasec;

    fprintf(stdout, "Wrote: %d bytes; Read: %d bytes; Total: %d bytes. \n", sentBytes, recdBytes, totlBytes);
    fprintf(stdout, "Start: %ld s %ld us; End: %ld s %ld us; Delta: %ld s %ld us. \n", timeStart.tv_sec, timeStart.tv_usec, timeEnd.tv_sec, timeEnd.tv_usec, timeDelta.tv_sec, timeDelta.tv_usec);
    fprintf(stdout, "%s baud for 8N1 is %d Bps (bytes/sec).\n", serspeed, (int)expectBps);
    if (!wrdata.iator ) fprintf(stdout, "NOT THE INITIATOR. Measured speed shouldn't be used.\n");
    fprintf(stdout, "Measured: write %.02f Bps (%.02f%%), read %.02f Bps (%.02f%%), total %.02f Bps.\n", measWriteBps, (measWriteBps/expectBps)*100, measReadBps, (measReadBps/expectBps)*100, totlBytes/deltasec);

    return 0;
}