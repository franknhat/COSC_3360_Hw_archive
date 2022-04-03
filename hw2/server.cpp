#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/wait.h>
#include <vector>
#include <string.h>
#include <map>
#include <cmath>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

//a lot of this code much like in client.cpp was dirived from rincons code from blackboard

void fireman(int){
    while (waitpid(-1, NULL, WNOHANG) > 0)
        continue;
}


//as the name implies it takes a decimal int and returns a binary string
std::string decimal_to_binary(int x){
    std::string bin;
    while (x)
    {
        x % 2 == 0 ? bin = '0' + bin : bin = '1' + bin;
        x /= 2;
    }
    return bin; // return the binary rep of the integer.
}


//This creates the map by taking in the input and filling it to a map
void setMap(std::map<std::string, char> &m, int len){
    int largest = 0;
    std::string *v = new std::string[len];//have to insert to an external array first due to bit length
    for (int i = 0; i < len; i++){
        std::getline(std::cin, v[i]);
        std::string temp=decimal_to_binary(stoi(v[i].substr(2)));//for get line its char white space then int so idx 2 to end is the int while index 0 is the char and index 1 is a white space
        if(stoi(v[i].substr(2))>largest)
            largest=stoi(v[i].substr(2));
        v[i]=v[i][0]+temp;
    }

    //taks the v arr and insert it into the map after formatting it
    for (int i = 0; i < len; i++){
        std::string t;
        t = v[i].substr(1);
        while (t.length() < ceil(log2(largest + 1)))//sets all to the same bitlength
            t = '0' + t;
        m.insert(std::pair<std::string, char>(t, v[i][0]));
    }
    delete[] v;
}

void error(char *msg){
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[]){
    //initializing vars needed to take in input to map
    std::map<std::string, char> m;
    std::string len;//number of chars to be inputed with their decimal values, using string so I dont use cin and just getline
    std::getline(std::cin, len);//didnt want to deal with cin.ignore later
    setMap(m, stoi(len));
    int bitlength = m.begin()->first.length();
    len = std::to_string(bitlength);

    //initial the vars needed to connect the server and client
    int sockfd, newsockfd, portno, clilen;

    char buffer[256];
    struct sockaddr_in serv_addr, cli_addr;
    int n;
    if (argc < 2){
        fprintf(stderr, "ERROR, no port provided\n");
        exit(1);
    }
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");
    bzero((char *)&serv_addr, sizeof(serv_addr));
    portno = atoi(argv[1]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR on binding");
    listen(sockfd, 5);
    clilen = sizeof(cli_addr);

    //created a new socket to allow me to send my bitlength over
    newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, (socklen_t *)&clilen);
    if (newsockfd < 0)
        error("ERROR on accept");
    bzero(buffer, 256);
    len.copy(buffer, sizeof(len.length()));
    n = write(newsockfd, buffer, 255);
    if (n < 0)
        error("ERROR writing to socket");

    //fireman code
    signal(SIGCHLD, fireman);
    while (true){
        //create a newsockfd to accept
        newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, (socklen_t *)&clilen);
        if (fork() == 0){
            if (newsockfd < 0)
                error("new socket wont open");
            //set buffer to 0 to read in the binary string from client
            bzero(buffer, 256);
            n = read(newsockfd, buffer, 255);
            if (n < 0)
                error("ERROR reading from socket");
            std::string temp=buffer;
            //set buffer to 0 to send over the decoded char to client
            bzero(buffer, 256);
            buffer[0] = m[temp];//why maps were used for the ease of use
            n = write(newsockfd, buffer, 255);
            if (n < 0)
                error("ERROR writing to socket");
            close(newsockfd);
            _exit(0);
        }
    }
    close(sockfd);
    return 0;
}