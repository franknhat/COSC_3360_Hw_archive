//this should get modified soon just want to turn something in in-case I forget later. This SHOULD be a working solution I hope.

#include <pthread.h>
#include <iostream>
#include <string>
#include <cmath>
#include <map>
#include <vector>
static pthread_mutex_t sem;
//PA1 but with mutex


struct nArgs{
    std::map<std::string,char>* m;
    std::string* msg;
    
    pthread_mutex_t* sem;
    pthread_cond_t* waitTurn;
    int i=0;
    char c;   
    int* parIdx;
    int dec;
    int bitLength;
    void setArgs(pthread_mutex_t* s,pthread_cond_t* wT, std::map<std::string, char>* mappu,std::string* message, char curChar,int decIn,int idx, int* cur){
        sem=s;
        waitTurn=wT;
        msg=message;
        c=curChar;
        i=idx;
        parIdx=cur;
        dec=decIn;
        m=mappu;
    }
};

struct mArgs{
    pthread_mutex_t* sem;
    pthread_cond_t* waitTurn;
    std::map<std::string,char>* m;
    std::string submsg;
    int idx;
    int* parIdx;
    std::string* str;
    void setInital(pthread_mutex_t* daSem,pthread_cond_t* turn,std::string* s,std::map<std::string,char>* mappu, int* idxPar){
        sem=daSem;
        waitTurn=turn;
        m=mappu;
        parIdx=idxPar;
        str=s;
    }
};

void getInput(std::pair<char, int>*&,int& ,std::string&, int&);
int getFreq(std::string, std::string);

std::string decimal_to_binary(int x, int len){
    std::string bin;
    while(x){
        x%2==0?bin='0'+bin:bin='1'+bin;
        x/=2;
    }
    while(bin.length()!=len){
        bin='0'+bin;
    }
    return bin; //return the binary rep of the integer.
}

void* nThreadsFunction(void* argumen){
    
    nArgs* argu= (nArgs*) argumen;
    std::string temp= decimal_to_binary(argu->dec, argu->bitLength);
    (*argu->m).insert(std::pair<std::string,char>(temp ,argu->c));
    int freq=getFreq(*argu->msg, temp);
    
    pthread_mutex_unlock(argu->sem);

    std::cout<<"Character: "<<argu->c<<", Code: "<<temp<<", Frequency: "<<freq<<"\n";
    
    pthread_mutex_lock(argu->sem);
    (*argu->parIdx)++;
    //std::cout<<*argu->parIdx<<'\n';
    pthread_cond_broadcast(argu->waitTurn);
    pthread_mutex_unlock(argu->sem);
    
    return NULL;
}

void* mThreadsFunction(void* nArgs){
    mArgs* args= (mArgs*) nArgs;
    //std::cout<<" substr: "<<args->submsg<<'\t';
    char c=(*args->m)[args->submsg];
    //args->idx++;
    //*args->parIdx++;
    pthread_mutex_unlock(args->sem);
    std::cout<<c;
    pthread_mutex_lock(args->sem);
    (*args->parIdx)+=1;
    pthread_cond_broadcast(args->waitTurn);
    pthread_mutex_unlock(args->sem);
    
    return NULL;
}

int main(){
    std::pair<char, int>* inputArr;
    std::string inStr, decodedStr;
    pthread_t* threads;
    int numInput, bitlength=0, curIdx=0;
    std::map<std::string, char> inMap;


    getInput(inputArr,numInput ,inStr, bitlength);
    
    //create and set nthreads nArgs
    static pthread_mutex_t bsem;//= PTHREAD_MUTEX_INITIALIZER;
    static pthread_cond_t waitTurn;//= PTHREAD_COND_INITIALIZER;
    nArgs* nThreadArgs= new nArgs;
    nThreadArgs->bitLength=bitlength;
    //create and run n threads
    threads = new pthread_t[numInput];
    //std::cout<<inStr<<" the in str\n";
    std::cout<<"Alphabet:\n";

    //deadlock on moodle server here
    for(int i=0;i<numInput;i++){
        pthread_mutex_lock(&bsem); //sometimes deadlocks ;-;
        if(i!=curIdx){ //prob cuz initalize at 0 maybe
            pthread_cond_wait(&waitTurn,&bsem);
        }
        //std::cout<<"main thread calls for: "<<i<<std::endl;
        nThreadArgs->setArgs(&bsem,&waitTurn, &inMap,&inStr,inputArr[i].first,inputArr[i].second,i,&curIdx);
        pthread_create(&threads[i],NULL,nThreadsFunction, nThreadArgs);
    }
    for(int i=0;i<numInput;i++){
        pthread_join(threads[i],NULL);
    }
    delete [] threads;
    threads=nullptr;
    delete [] inputArr;
    mArgs mThreadsAgs;
    curIdx=0;

    mThreadsAgs.setInital(&bsem, &waitTurn,&decodedStr,&inMap, &curIdx);
    delete nThreadArgs;
    threads= new pthread_t[inStr.length()/bitlength];
    std::cout<<"\nDecompressed message: ";
    for(int i=0;i<inStr.length()/bitlength;i++){
        pthread_mutex_lock(&bsem);
        while(i>*mThreadsAgs.parIdx){
            //std::cout<<"in while\n";
            //std::cout<<"i: "<<i<<"\t parIdx: "<<*mThreadsAgs.parIdx<<"\n";
            pthread_cond_wait(&waitTurn, &bsem);
        }
        mThreadsAgs.submsg=inStr.substr(i*bitlength,bitlength);
        //std::cout<<"\nsubstr: "<<mThreadsAgs.submsg<<'\t';
        //mThreadsAgs.idx++;
        //pthread_mutex_unlock(&bsem);
        pthread_create(&threads[i],NULL,mThreadsFunction,&mThreadsAgs);
    }

    for(int i=0;i<inStr.length()/bitlength;i++){
        pthread_join(threads[i],NULL);
    }
    std::cout<<'\n';
    delete [] threads;
    return 0;
}

void getInput(std::pair<char, int>* &inputArr,int& size, std::string& s, int& len){
    std::cin>>size;//num of input
    std::cin.ignore();
    inputArr= new std::pair<char,int>[size];
    for(int i=0;i<size;i++){
        std::string temp;
        std::getline(std::cin,temp);
        if(len<stoi(temp.substr(2)))
            len=stoi(temp.substr(2));
        inputArr[i]=std::pair<char,int>(temp[0],stoi(temp.substr(2)));
    }
    len=ceil(log2(len+1));//find its bitlength
    std::getline(std::cin, s);
    return;
}

int getFreq(std::string str, std::string subStr){
    int freq=0;
    //std::cout<<"for substr: "<<subStr<<"\t";
    for(int j=0;j<str.length();j+=subStr.length()){
        //std::cout<<"j = "<<j<<" "<<str.substr(j,subStr.length())<<'\t';
        if(str.substr(j,subStr.length())==subStr)
            freq+=1;
    }
    //std::cout<<'\n';
    return freq;
}