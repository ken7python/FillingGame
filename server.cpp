//% c++ -std=c++11 -I. server.cpp libsockpp.a -lraylib -o server && ./server
#include <iostream>
#include <sockpp/tcp_acceptor.h>
#include <pthread.h>
#include "raylib.h"

struct packet{
    int row;
    int col;
    Color color;
};

struct passer_socks{
    sockpp::tcp_socket* self;
    sockpp::tcp_socket* other0;
};

void* passer(void* arg){
    passer_socks* socks = (passer_socks*)arg;

    while(1){
        packet p;
        socks->self->read(&p,sizeof(p));
        socks->other0->write(&p,sizeof(p));
    }
    return NULL;
}

int main(){
    sockpp::tcp_acceptor acc(12345);
    sockpp::tcp_socket sock[2];
    sock[0] = acc.accept();
    sock[1] = acc.accept();

    std::cout << "players come.\n";

    int ready = 1;
    sock[0].write(&ready,sizeof(ready));
    sock[1].write(&ready,sizeof(ready));

    passer_socks forp0{
        &sock[0],
        &sock[1]
    };
    passer_socks forp1{
        &sock[1],
        &sock[0]
    };

    pthread_t th0;
    pthread_create(&th0,NULL,passer, &forp0);
    pthread_create(&th0,NULL,passer, &forp1);
    pthread_t th1;

    while(1){
        usleep(1000 * 10);
    }

    return 0;
}