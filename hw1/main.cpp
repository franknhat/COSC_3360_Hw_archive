#include <pthread.h>
#include <iostream>
#include <string>
#include <cmath>
#include <map>
#include <vector>
//file input redirect is complie then in term. ./main <infile.txt //reads file as input/cin
struct arguments{
    std::string bin_letters;
    char letter;
    int freq;

    std::string coded;
    int max;
    std::string decoded;
};

struct arg2{
    std::map<std::string, char> alph;
    std::string substring;
    char ans;
};

std::string decimal_to_binary(int x){
    std::string bin;
    while(x){
        x%2==0?bin='0'+bin:bin='1'+bin;
        x/=2;
    }
    return bin;
}

void* n_children(void* argum){
    arguments* args= (arguments*)argum;
    args->bin_letters=decimal_to_binary(args->freq);
    while(args->bin_letters.length()!=args->max)
        args->bin_letters='0'+args->bin_letters;
    args->freq=0;
    for(int i=0;i<args->coded.length();i+=args->max)
        if(args->bin_letters==args->coded.substr(i,args->max))
            args->freq++;
    return NULL;
}

void* m_children(void* argum){
    arg2* args= (arg2*)argum;
    args->ans=args->alph[args->substring];
    return NULL;
}

int main(){
    std::string numInput, coded_message, decoded_message;
    int max_char=0;

    std::getline(std::cin, numInput);

    arguments* input= new arguments[stoi(numInput)];
    pthread_t* threads= new pthread_t[stoi(numInput)];
    
    for(int i=0;i<stoi(numInput);i++){
        std::string in;
        std::getline(std::cin, in);
        if(stoi(in.substr(2))>max_char)
            max_char=stoi(in.substr(2));
        input[i].letter=in[0];
        input[i].freq=stoi(in.substr(2)); //temp using freq to store decimal value
    }

    std::getline(std::cin, coded_message);
    //if log2 exist do a +1 since 8 in binary is 4digits not 3
    max_char=(ceil(log2(max_char))==floor(log2(max_char))?ceil(log2(max_char))+1:ceil(log2(max_char)));
    //n threads
    for(int i=0;i<stoi(numInput);i++){
        input[i].coded=coded_message;
        input[i].max=max_char;
    }
    for(int i=0;i<stoi(numInput);i++)
        pthread_create(&threads[i],NULL, n_children, &input[i]);
    for(int i=0;i<stoi(numInput);i++)
        pthread_join(threads[i],NULL);
    delete[] threads;
    
    //m threads
    threads= new pthread_t[coded_message.length()/max_char];
    std::vector<arg2> m_thread_args(coded_message.length()/max_char);
    for(int i=0;i<m_thread_args.size();i++){
        for(int j=0;j<stoi(numInput);j++){
            m_thread_args[i].alph.insert(std::pair<std::string,char>(input[j].bin_letters,input[j].letter));
        }
    }
    for(int i=0;i<coded_message.length();i+=max_char){
        m_thread_args[i/max_char].substring=coded_message.substr(i,max_char);
        pthread_create(&threads[i/max_char],NULL,m_children,&m_thread_args[i/max_char]);
    }
    for(int i=0;i<coded_message.length()/max_char;i++)
        pthread_join(threads[i],NULL);
    delete [] threads;

    //decompress message
    for(int i=0;i<m_thread_args.size();i++)
        decoded_message+=m_thread_args[i].ans;
    
    //print
    std::cout<<"Alphabet:\n";
    for(int i=0;i<stoi(numInput);i++)
        std::cout<<"Character: "<<input[i].letter<<", Code: "<<input[i].bin_letters<<", Frequency: "<<input[i].freq<<std::endl;
    std::cout<<"\nDecompressed message: "<<decoded_message<<std::endl;
    delete[] input;
    return 0;
}