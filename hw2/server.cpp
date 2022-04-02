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
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>

#define BUFFER_LENGTH 256

void fireman(int)
{
    while (waitpid(-1, NULL, WNOHANG) > 0)
        continue;
}

std::string decimal_to_binary(int x)
{
    std::string bin;
    while (x)
    {
        x % 2 == 0 ? bin = '0' + bin : bin = '1' + bin;
        x /= 2;
    }
    return bin; // return the binary rep of the integer.
}

void setMap(std::map<std::string, char> &m, int len)
{ // takes input and
    int largest = 0;
    std::string *v = new std::string[len];
    for (int i = 0; i < len; i++)
    {
        std::getline(std::cin, v[i]);
        for (int j = 0; j < v[i].length(); j++)
        {
            if (isdigit(v[i][j]) == true && j!=0)
            {
                if (stoi(v[i].substr(j)) > largest)
                    largest = stoi(v[i].substr(j));
                std::string t = decimal_to_binary(stoi(v[i].substr(j)));
                v[i] = v[i].substr(0, j - 1); // remove white space
                v[i] += t;
                break;
            }
        }
    }
    for (int i = 0; i < len; i++)
    {
        std::string t;
        t = v[i].substr(1);
        while (t.length() < ceil(log2(largest + 1)))
            t = '0' + t;
        m.insert(std::pair<std::string, char>(t, v[i][0]));
    }
    delete[] v;
}

void error(char *msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[])
{

    std::map<std::string, char> m;
    std::string len;
    std::getline(std::cin, len);
    setMap(m, stoi(len));
    int bitlength = m.begin()->first.length();
    len = std::to_string(bitlength);

    int sockfd, newsockfd, portno, clilen;

    char buffer[BUFFER_LENGTH];
    struct sockaddr_in serv_addr, cli_addr;
    int n;
    if (argc < 2)
    {
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
    if (bind(sockfd, (struct sockaddr *)&serv_addr,
             sizeof(serv_addr)) < 0)
        error("ERROR on binding");
    listen(sockfd, 5);
    clilen = sizeof(cli_addr);
    newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, (socklen_t *)&clilen);
    if (newsockfd < 0)
        error("ERROR on accept");
    bzero(buffer, BUFFER_LENGTH);
    len.copy(buffer, sizeof(len.length()));
    n = write(newsockfd, buffer, 255);
    if (n < 0)
        error("ERROR writing to socket");
    //close(newsockfd);

    signal(SIGCHLD, fireman);
    while (true)
    {
        newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, (socklen_t *)&clilen);
        if (fork() == 0)
        {
            //close(sockfd);
            if (newsockfd < 0)
                error("new socket wont open");

            bzero(buffer, BUFFER_LENGTH);
            n = read(newsockfd, buffer, 255);
            if (n < 0)
                error("ERROR reading from socket");
            std::string temp;
            for (int i = 0; i < 255; i++)
            {
                if (buffer[i] == '\0')
                    break;
                temp += buffer[i];
            }
            bzero(buffer, BUFFER_LENGTH);
            buffer[0] = m[temp];
            //std::cout << "the value is" << temp << '\n';
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