#include <iostream>
#include <pthread.h>
//C++は小文字のnullptrを使うといい
//CにはnullptrがないのでNULLを使う
//C++ではstructの中に関数が書ける
//Cでは書けない
//ポインタを実態にしたら、次からは.でつなぐ

struct que_t{
    std::queue<int> que;
    pthread_mutex_t mtx;
    //#define NULL ((void*)0)
    que_t(){ //C/C++04:NULL, C++11:nulltprコンパイラが直接理解できる
        pthread_mutex_init(&mtx, nullptr);
    }
    ~que_t(){
        pthread_mutex_destroy(&mtx);
    }
};

void* func(void* arg){
    que_t* tsque = (que_t*)arg;

    //thread1 output
    int n = 0;
    while(n <= 9999){
        pthread_mutex_lock(&tsque->mtx);
        if (tsque->que.size()){
            n = tsque->que.front();
            tsque->que.pop();
            std::cout << "n: " << n << std::endl;
        }else{
            std::cout << "empty" << std::endl;
        }
        pthread_mutex_unlock(&tsque->mtx);
    }

    return nullptr;
}

int main(){
    que_t tsque;

    pthread_t th;
    pthread_create(&th, nullptr,func, &tsque);


    //thread0 - input
    int n = 0;
    pthread_mutex_lock(&tsque.mtx);
    while(n <= 9999 + 1){
        tsque.que.push(n);
        ++n;
    }
    pthread_mutex_unlock(&tsque.mtx);

    pthread_join(th, nullptr);

    return 0;
}