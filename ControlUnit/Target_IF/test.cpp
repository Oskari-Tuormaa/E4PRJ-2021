#include "Target_IF.hpp"
#include <iostream>
#include <fstream>
#include <pthread.h>

using namespace std;  

void *funcUART_rx(void *arg)
{
    int fd;
    fd = open("/dev/ttyACM0", O_RDWR, S_IRUSR | S_IWUSR);
    if( fd == -1)
        printf("Failed to open with err: %s", strerror(errno));
    
    //ifstream MyFile;
    //MyFile.open("/dev/ttyACM0", ios::in);

    while(1)
    {
        //printf("Reading\n");
        char rx_buffer[BUF_SIZE] = "";
        read(fd, rx_buffer, BUF_SIZE);
        //MyFile.read(rx_buffer, 1);
        //if(strlen(rx_buffer) != 0)
        printf("%s\n", rx_buffer);
    }

    //MyFile.close();
    pthread_exit(NULL);
}

int main()
{
    pthread_t UART_rx;
    pthread_create(&UART_rx, NULL, funcUART_rx, NULL);

    printf("Program started\n");

    Target_IF testObj;    

    sleep(1);
    testObj.startDetection(2);
    sleep(1);
    testObj.stopDetection();

    sleep(1);
    //pthread_join( UART_rx, NULL );

    return 0;
}