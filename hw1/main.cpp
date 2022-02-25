#include <pthread.h>
#include <iostream>
#include <string>
#include <cmath>
#include <map>
//file input redirect is complie then in term. ./main <infile.txt //reads file as input/cin
struct arguments{
    int dec_value; //decimal value of input
    std::string* bin_letters; //binary of letters will call new for this since done in threads
    char letter; //the char input
    int freq; // frequency

    std::string* coded; //should just be a ptr to the coded message
    int max;
   ~arguments(){
        delete bin_letters;
        //no new called for coded so
        coded=nullptr;
    }
};

struct arg2{ //This is for the m threads potion to solve a certain substr of the 
    std::map<std::string, char>* alph; //map holds input char and binary value, map ptr to save space from like 7+ copies of the same map
    std::string substring; //part of coded_message to decode
    char* ans; // char answer set to pos of decoded string
    ~arg2(){
        alph=nullptr;
    }
};

std::string decimal_to_binary(int x){
    std::string bin;
    while(x){
        x%2==0?bin='0'+bin:bin='1'+bin;
        x/=2;
    }
    return bin; //return the binary rep of the integer.
}

void* n_children(void* argum){
    arguments* args= (arguments*)argum;
    args->bin_letters=new std::string(decimal_to_binary(args->dec_value));
    //sets binary to the correct bit length
    while(args->bin_letters->length()!=args->max)
        *args->bin_letters='0'+*args->bin_letters;
    //makes sure freq is 0 to start counting the freq of the char's binary rep
    args->freq=0;
    for(int i=0;i<args->coded->length();i+=args->max) //look at each binary char and compares to current char to see if same, if same freq ++
        if(*args->bin_letters==args->coded->substr(i,args->max))
            args->freq++;
    return NULL;
}

void* m_children(void* argum){
    arg2* args= (arg2*)argum;
    //std::cout<<args->ans<<'\n';
    *args->ans=(*args->alph)[args->substring]; //plugs substring to map and get answer in same line
    return NULL;
}

int main(){
    std::string numInput, coded_message, decoded_message;
    int max_char=0;
    arguments* input; //arguments for n threads
    arg2* m_thread_args; //arguments for m threads
    pthread_t* threads; //the threads arr
    std::map<std::string,char>* alphebet;
    std::getline(std::cin, numInput); //used getline cuz I didnt want to deal with cin.ignore if I used both normal cin and getline(cin)

    input= new arguments[stoi(numInput)];
    threads= new pthread_t[stoi(numInput)];
    
    //takes input char + int
    for(int i=0;i<stoi(numInput);i++){
        std::string in;
        std::getline(std::cin, in);
        if(stoi(in.substr(2))>max_char)
            max_char=stoi(in.substr(2));
        input[i].letter=in[0];
        input[i].dec_value=stoi(in.substr(2));
    }

    std::getline(std::cin, coded_message);
    
    //if log2 is a digit do a +1 since 8 in binary is 4digits not 3
    max_char=(ceil(log2(max_char))==floor(log2(max_char))?ceil(log2(max_char))+1:ceil(log2(max_char)));// changes max char from largest integer to length of bitstring of largest number
    
    //n threads
    for(int i=0;i<stoi(numInput);i++){ //sets the coded message and bitlength to call elemetnts of arr
        input[i].coded=&coded_message;
        input[i].max=max_char;
    }
    for(int i=0;i<stoi(numInput);i++) //arr[i] so order of arr isnt affected only modifies a certain element of arr per thread
        pthread_create(&threads[i],NULL, n_children, &input[i]);
    for(int i=0;i<stoi(numInput);i++)
        pthread_join(threads[i],NULL);
    delete[] threads;

    //m threads
    threads= new pthread_t[coded_message.length()/max_char]; //coded_message.length()/mac_char represents the number of characters in the decoded message
    m_thread_args= new arg2[coded_message.length()/max_char];
    alphebet= new std::map<std::string,char>;
    
    //set map
    for(int i=0;i<stoi(numInput);i++)
        alphebet->insert(std::pair<std::string,char>(*input[i].bin_letters,input[i].letter));
    
    //set decoded string length w/out setting the chars yet
    decoded_message.resize(coded_message.length()/max_char);

    //set args for the m threads
    for(int i=0;i<coded_message.length()/max_char;i++){ //sets the map for all elements of the array
        m_thread_args[i].alph=alphebet;
        m_thread_args[i].ans=&decoded_message[i]; //sets ptr to specific element of string to modify
        m_thread_args[i].substring=coded_message.substr(i*max_char,max_char);
    }
    for(int i=0;i<coded_message.length()/max_char;i++)
        pthread_create(&threads[i],NULL,m_children,(void*)&m_thread_args[i]);
    for(int i=0;i<coded_message.length()/max_char;i++)
        pthread_join(threads[i],NULL);
    
    delete [] threads;
    delete alphebet;
    delete [] m_thread_args;

    //print
    std::cout<<"Alphabet:\n";
    for(int i=0;i<stoi(numInput);i++)
        std::cout<<"Character: "<<input[i].letter<<", Code: "<<*input[i].bin_letters<<", Frequency: "<<input[i].freq<<std::endl;
    std::cout<<"\nDecompressed message: "<<decoded_message<<std::endl;
    delete[] input;
    return 0;
}