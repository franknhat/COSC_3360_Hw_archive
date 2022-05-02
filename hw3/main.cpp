#include <pthread.h>
#include <iostream>
#include <string>
#include <cmath>
#include <vector>
#include <unordered_map>

struct nArgs{
    pthread_mutex_t* sem;
    pthread_cond_t* waitTurn;
    std::unordered_map<std::string,char>* m; //used for inserting into the map
    std::string* msg;//coded message
    char c;//character finding binary of
    int parIdx=0;//int for synchronization
    int childID;
    int dec;//decimal value of char
    int bitLength;
    void setArgs(pthread_mutex_t* s,pthread_cond_t* wT, std::unordered_map<std::string, char>* mappu,std::string* message, char curChar,int decIn){
        sem=s;
        waitTurn=wT;
        msg=message;
        c=curChar;
        dec=decIn;
        m=mappu;
    }
    //no need for destructor since its pointers to addresses in main rather than memory allocation
};

struct mArgs{
    pthread_mutex_t* sem;
    pthread_cond_t* waitTurn;
    std::unordered_map<std::string,char>* m; //used for reading from the map
    std::string submsg; //substring to decode
    int parIdx=0;
    int curIdx;
    std::string* str; //string in main that holds the decompressed message
    void setInital(pthread_mutex_t* daSem,pthread_cond_t* turn,std::unordered_map<std::string,char>* mappu, std::string* input_str){
        sem=daSem;
        waitTurn=turn;
        str=input_str;
        m=mappu;
    }
    //no need for destructor since its pointers to addresses in main rather than memory allocation
};

void getInput(std::pair<char, int>*&,int& ,std::string&, int&);
int getFreq(std::string, std::string);
std::string decimal_to_binary(int, int);
void* nThreadsFunction(void* argumen);
void* mThreadsFunction(void* nArgs);

int main(){
    std::pair<char, int>* inputArr;
    std::string inStr, outputStr;
    pthread_t* threads;
    int numInput, bitlength=0;
    std::unordered_map<std::string, char> inMap;

    getInput(inputArr,numInput ,inStr, bitlength);
    
    //create and set nthreads nArgs
    static pthread_mutex_t bsem;
    static pthread_cond_t waitTurn;
    nArgs* nThreadArgs= new nArgs;
    nThreadArgs->bitLength=bitlength;
    //create and run n threads
    threads = new pthread_t[numInput];
    std::cout<<"Alphabet:\n";

    for(int i=0;i<numInput;i++){
        pthread_mutex_lock(&bsem); //sometimes deadlocks ;-;
        nThreadArgs->setArgs(&bsem,&waitTurn, &inMap,&inStr,inputArr[i].first,inputArr[i].second);
        nThreadArgs->childID=i;
        pthread_create(&threads[i],NULL,nThreadsFunction, nThreadArgs);
    }
    for(int i=0;i<numInput;i++)
        pthread_join(threads[i],NULL);

    //delete unnessary allocation from n thread section
    delete [] threads;
    threads=nullptr;
    delete [] inputArr; //not needed since we have the map now
    delete nThreadArgs;

    //creates and set the mthread args
    mArgs mThreadsAgs;
    mThreadsAgs.setInital(&bsem, &waitTurn,&inMap,&outputStr);
    threads= new pthread_t[inStr.length()/bitlength];

    //runs the m threads section
    for(int i=0;i<inStr.length()/bitlength;i++){
        pthread_mutex_lock(&bsem);
        mThreadsAgs.submsg=inStr.substr(i*bitlength,bitlength);
        mThreadsAgs.curIdx=i;
        pthread_create(&threads[i],NULL,mThreadsFunction,&mThreadsAgs);
    }

    for(int i=0;i<inStr.length()/bitlength;i++)
        pthread_join(threads[i],NULL);
    std::cout<<"\nDecompressed message: "<<outputStr<<'\n';
    delete [] threads;
    return 0;
}

void* nThreadsFunction(void* argumen){//first thread call
    nArgs* argu= (nArgs*) argumen;
    int decimal=argu->dec;
    char c=argu->c;
    int childIdentifier=argu->childID;
    pthread_mutex_unlock(argu->sem);

    //gets the substring of the current int and set length to bitlength
    std::string temp= decimal_to_binary(decimal, argu->bitLength);
    //insert into map from main to be used later
    (*argu->m).insert(std::pair<std::string,char>(temp ,c));
    int freq=getFreq(*argu->msg, temp);
    
    pthread_mutex_lock(argu->sem);
    while(childIdentifier!=argu->parIdx)
        pthread_cond_wait(argu->waitTurn, argu->sem);
    pthread_mutex_unlock(argu->sem);

    std::cout<<"Character: "<<c<<", Code: "<<temp<<", Frequency: "<<freq<<"\n";


    pthread_mutex_lock(argu->sem);
    (argu->parIdx)++;//iterates a main int, this is the synchronization
    pthread_cond_broadcast(argu->waitTurn);
    pthread_mutex_unlock(argu->sem);
    
    return NULL;
}

void* mThreadsFunction(void* nArgs){
    mArgs* args= (mArgs*) nArgs;
    int threadId=args->curIdx;
    std::string substr=args->submsg;
    pthread_mutex_unlock(args->sem);

    pthread_mutex_lock(args->sem);
    while(threadId!=args->parIdx)
        pthread_cond_wait(args->waitTurn, args->sem);
    pthread_mutex_unlock(args->sem);

    (*args->str)+=(*args->m)[substr];

    pthread_mutex_lock(args->sem);
    (args->parIdx)+=1;
    pthread_cond_broadcast(args->waitTurn);
    pthread_mutex_unlock(args->sem);
    
    return NULL;
}

void getInput(std::pair<char, int>* &inputArr,int& size, std::string& s, int& len){//all the cin/getline to get the input
    std::cin>>size;//num of input
    std::cin.ignore();
    inputArr= new std::pair<char,int>[size];
    for(int i=0;i<size;i++){
        std::string temp;
        std::getline(std::cin,temp);
        if(len<stoi(temp.substr(2)))// since we know its char space then int we can use substring 2 for idx 2 till end for the integer
            len=stoi(temp.substr(2));//setting len(bitlength to largest integer input)
        inputArr[i]=std::pair<char,int>(temp[0],stoi(temp.substr(2))); //input into the arr char then int
    }
    len=ceil(log2(len+1));//converting bitlength from being the largest integer to its own bitlength
    std::getline(std::cin, s);//get the coded string
    return;
}

int getFreq(std::string str, std::string subStr){//search the string the freq of substring
    int freq=0;
    for(int j=0;j<str.length();j+=subStr.length())
        if(str.substr(j,subStr.length())==subStr)//this runs in the for loop even without the curly brace
            freq+=1;// same comment as line above

    return freq;
}

std::string decimal_to_binary(int x, int len){//decimat to binary of bitlength len
    std::string bin;
    while(x){//the dec to bin part
        x%2==0?bin='0'+bin:bin='1'+bin;
        x/=2;
    }
    while(bin.length()!=len)//add 0 at front if not at bitlength
        bin='0'+bin;
    return bin; //return the binary rep of the integer.
}