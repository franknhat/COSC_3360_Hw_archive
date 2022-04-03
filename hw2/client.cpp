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

//similar to my server.cpp a lot of this code is dirived from Rincons code from blackboard

struct threadParams{
    std::string subMsg;
    int bitlength;
    char symbol;
    struct sockaddr_in server_addr;
    struct hostent *serv;
    int portnumber;
};

void *sendServerThreads(void *params){
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
    if (sockfd < 0){
        std::cout<<"ERROR opening socket\n";
        exit(0);
    }

    //connecting to server
    bzero((char *)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,
          (char *)&serv_addr.sin_addr.s_addr,
          server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){
        std::cout<<"Error connecting to server\n";
        exit(0);
    }

    bzero(buffer, args->bitlength + 2);
    args->subMsg.copy(buffer, sizeof(args->subMsg.length()));
    n = write(sockfd, buffer, args->bitlength + 1);
    if (n < 0){
        std::cout<<"Error writing the socket\n";
        exit(0);
    }
    bzero(buffer, args->bitlength + 2);
    n = read(sockfd, buffer, args->bitlength + 1);
    if (n < 0){
        std::cout<<"ERROR reading from socket\n";
    }

    args->symbol = buffer[0];

    close(sockfd);
    return NULL;
}

int main(int argc, char *argv[]){
    int sockfd, portno, n, bitlength, numthreads;
    std::string temp, encocedmsg, decodedmsg;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    char buffer[256];
    if (argc < 3){
        std::cout<<"too little arguments for client.cpp\n";
        return -1;
    }
    
    //create socket
    portno = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0){
        std::cout<<"ERROR opening socket\n";
        return -1;
    }

    //get server
    server = gethostbyname(argv[1]);
    if (server == NULL){
        fprintf(stderr, "ERROR, no such host\n");
        exit(0);
    }
    //connect to server
    bzero((char *)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,
          (char *)&serv_addr.sin_addr.s_addr,
          server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){
        std::cout<<"ERROR connecting to server\n";
        return -1;
    }

    // receives from server the bitlength
    bzero(buffer, 256);
    n = read(sockfd, buffer, 255);
    if (n < 0){
        std::cout<<"ERROR reading from socket\n";
    }
    close(sockfd); //close not needed till pthreads
    // read from buffer
    temp = buffer; //converts char[] to string
    bitlength = std::stoi(temp);//then to an int for bitlength

    // take in string encoded message
    std::cin >> encocedmsg;

    //creating threads
    numthreads = encocedmsg.length() / bitlength;
    pthread_t *threads = new pthread_t[numthreads];
    //sending vars that are needed for all threads
    threadParams *var = new threadParams[numthreads];
    for (int i = 0; i < numthreads; i++){
        var[i].bitlength = bitlength;
        var[i].subMsg = encocedmsg.substr(i * bitlength, bitlength);
        var[i].portnumber = portno;
        var[i].serv = server;
        var[i].server_addr=serv_addr;
    }
    // std::cout << "number of threads is: " << numthreads << '\n';
    for (int i = 0; i < numthreads; i++)
        pthread_create(&threads[i], NULL, sendServerThreads, &var[i]);
    
    for (int i = 0; i < numthreads; i++)
        pthread_join(threads[i], NULL);
    
    delete[] threads;

    for (int i = 0; i < numthreads; i++)
        decodedmsg += var[i].symbol;
    delete[] var;

    std::cout << "Decompressed message: " << decodedmsg << "\n";
    return 0;
}