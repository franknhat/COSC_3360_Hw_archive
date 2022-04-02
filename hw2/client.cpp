/*
This program receives the number of bits of the fixed-length codes from the server program (using sockets). Then, your solution creates m child threads (where m is the number of characters in the decompressed message). Each child thread will use the parallel solution from assignment 1 to store the decompressed message into a memory location accessible by the main thread.

Each child thread determines a character of the decompressed message following the steps presented below:

Create a socket to communicate with the server program.
Send the binary code of the symbol to decompress to the server program using sockets.
Wait for the decompressed character from the server program.
Write the received information into a memory location accessible by the main thread.
Finish its execution.

tldr: cin/getline binary message, receive bit leng from server,
to do: send server spliced messege substr, receive decoded char, cout message
*/

// most of the code here has been taken from Rincon and his lecture/blackboard
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string>
#include <iostream>

void error(char *msg)
{
    perror(msg);
    exit(0);
}

struct threadParams
{
    std::string subMsg;
    int bitlength; //?
    char symbol;
    struct sockaddr_in server_addr;
    struct hostent *serv;
    int portnumber;
};

void *sendServerThreads(void *params)
{
    threadParams *args = (threadParams *)params;

    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    char *buffer = new char[args->bitlength + 2]; // you are sending a string of length bit length and receiving a char this should be plenty for a buffer

    portno = args->portnumber;
    server = args->serv;
    serv_addr=args->server_addr;

    // make a new socket per thread
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        error("ERROR opening socket");
    }
    bzero((char *)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,
          (char *)&serv_addr.sin_addr.s_addr,
          server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR connecting");

    bzero(buffer, args->bitlength + 2);
    args->subMsg.copy(buffer, sizeof(args->subMsg.length()));
    n = write(sockfd, buffer, args->bitlength + 1);
    if (n < 0)
    {
        error("Error writing the socket");
    }
    bzero(buffer, args->bitlength + 2);
    n = read(sockfd, buffer, args->bitlength + 1);
    if (n < 0)
        error("ERROR reading from socket");
    args->symbol = buffer[0];

    close(sockfd);
    return NULL;
}

int main(int argc, char *argv[])
{
    int sockfd, portno, n, bitlength, numthreads;
    std::string temp, encocedmsg, decodedmsg;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    char buffer[256];
    if (argc < 3)
    {
        fprintf(stderr, "usage %s hostname port\n", argv[0]);
        exit(0);
    }
    portno = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");
    server = gethostbyname(argv[1]);
    if (server == NULL)
    {
        fprintf(stderr, "ERROR, no such host\n");
        exit(0);
    }
    bzero((char *)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,
          (char *)&serv_addr.sin_addr.s_addr,
          server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR connecting");
    bzero(buffer, 256);
    // receives from server the bitlength
    n = read(sockfd, buffer, 255);
    if (n < 0)
        error("ERROR reading from socket");
    close(sockfd);
    // read from buffer
    temp = "";
    for (int i = 0; i < 255; i++)
    {
        if (buffer[i] == '\0')
            break;
        temp += buffer[i];
    }
    bitlength = std::stoi(temp);
    //std::cout << "yo the bitlenth is: " << bitlength << '\n';

    // take in string
    std::cin >> encocedmsg;
    numthreads = encocedmsg.length() / bitlength;
    pthread_t *threads = new pthread_t[numthreads];

    threadParams *var = new threadParams[numthreads];
    for (int i = 0; i < numthreads; i++)
    {
        var[i].bitlength = bitlength;
        var[i].subMsg = encocedmsg.substr(i * bitlength, bitlength);
        var[i].portnumber = portno;
        var[i].serv = server;
        var[i].server_addr=serv_addr;
    }
    // std::cout << "number of threads is: " << numthreads << '\n';
    for (int i = 0; i < numthreads; i++)
    {
        pthread_create(&threads[i], NULL, sendServerThreads, &var[i]);
    }
    for (int i = 0; i < numthreads; i++)
    {
        pthread_join(threads[i], NULL);
    }
    delete[] threads;

    for (int i = 0; i < numthreads; i++)
    {
        decodedmsg += var[i].symbol;
        // std::cout << "the char for index" << i << " is " << var[i].symbol << '\n';
    }
    std::cout << "Decompressed message: " << decodedmsg << "\n";
    delete[] var;
    return 0;
}