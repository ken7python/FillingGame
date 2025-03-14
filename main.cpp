//c++ -std=c++11 -I. main.cpp libsockpp.a -lraylib -arch arm64 && ./a.out
#include <iostream>
#include <sockpp/tcp_acceptor.h>
#include <sockpp/stream_socket.h>
#include <pthread.h>
#include "raylib.h"
#define GRID_SIZE 5

using namespace std;

class Player{
    public:
        Vector2 position;
        int r;

        Player(Color color,int r){
            this->color = color;
            this->r = r;
        }
        Color GetColor(){
            return this->color;
        }
        void Draw(){
            DrawCircleV(this->position, r, this->color);
        }
        void SetPosition(Vector2 pos){
            this->position = pos;
        }
        void move(){
            this->position = GetMousePosition();
        }
    private:
        Color color;
};

class CGrid{
    public:
        CGrid(){
            this->rec.width = GRID_SIZE;
            this->rec.height = GRID_SIZE;
            SetColor(WHITE);
        }
        Color GetColor(){
            return this->color;
        }
        void SetPosition(Vector2 pos){
            this->rec.x = pos.x;
            this->rec.y = pos.y;
        }
        void SetColor(Color color){
            this->color = color;
        }
        bool SameColor(Color color){
            return this->color.r == color.r && this->color.g == color.g && this->color.b == color.b && this->color.a == color.a;
        }
        void Draw(){
            DrawRectanglePro(this->rec,Vector2{this->rec.width / 2,this->rec.height / 2},0,this->color);
            //cout << "Drawing(" << rec.x << "," << rec.y << ")" << endl;
        }

        bool ColPlayer(Player player){
            if(!SameColor(player.GetColor()) && CheckCollisionCircleRec(player.position,player.r,rec)){
                //cout << "Collision" << endl;
                this->color = player.GetColor();
                return true;
            }
            return false;
        }
    private:
        Rectangle rec;
        Color color;
};

struct packet{
    int row;
    int col;
    Color color;
};

struct argument{
    bool come;
    std::queue<packet> sendqueue;
};

void* sock_server(void* arg){
    sockpp::tcp_acceptor acc(12345);
    sockpp::tcp_socket sock = acc.accept();

    std::cout << "player come.\n";

    ((argument*)arg)->come = true;
    char buf[1];
    while(0 < sock.read(buf,sizeof(buf))){
        std::cout << buf[0] << std::endl;
        std::cout << ((argument*)arg)->sendqueue.size() << std::endl;
    }

    return NULL;
}


int main(){
    const int screenWidth = 800;
    const int screenHeight = 450;

    float StartTime = GetTime();
    bool GameClear = false;
    float EndTime;

    InitWindow(screenWidth, screenHeight, "raylib [core] example - basic window");
    SetTargetFPS(60);

    argument arg;
    arg.come = false;
    pthread_t th;
    pthread_create(&th,NULL,sock_server,&arg);

    while(!WindowShouldClose()){
        if (arg.come){
            break;
        }
        BeginDrawing();
            ClearBackground(BLACK);
            DrawText("Waitting a player...",350,250,50,WHITE);
        EndDrawing();
    }

    int rows = screenHeight / GRID_SIZE;
    int cols = screenWidth / GRID_SIZE;
    CGrid grids[rows][cols];
    int i = 0;
    int j = 0;
    //cout << "rows: " << rows << endl;
    //cout << "cols: " << cols << endl;
    Player player(BLUE,30);

    while(j < rows){
        while(i < cols){
            //cout << "j: " << j << endl;
            //cout << "i: " << i << endl;
            grids[j][i].SetPosition(Vector2{ (float)(i * GRID_SIZE + GRID_SIZE / 2),(float)(j * GRID_SIZE + GRID_SIZE / 2)});
            grids[j][i].SetColor(BLACK);
            ++i;
        }
        i = 0;
        ++j;
    }

    float percent;
    int painted;

    while (!WindowShouldClose()){
        BeginDrawing();

        ClearBackground(RAYWHITE);

        painted = 0;
        i = 0;
        j = 0;
        while(j < rows){
            while(i < cols){
                bool changed = grids[j][i].ColPlayer(player);
                if(changed){
                    packet pack = {j,i,player.GetColor()};
                    arg.sendqueue.push(pack);
                }
                if (grids[j][i].SameColor(player.GetColor() )){
                    ++painted;
                }
                grids[j][i].Draw();
                ++i;
            }
            i = 0;
            ++j;
        }

        player.move();
        player.Draw();

        percent = (float)painted / (rows * cols) * 100;
        if (percent == 100 && !GameClear){
            EndTime = GetTime();
            GameClear = true;
        }

        DrawText(TextFormat("%d%%", (int)percent), 30, 30, 50, RED);

        if (GameClear){
            DrawText(TextFormat("Game Clear!"), 300, 100, 50, RED);
            DrawText(TextFormat("Time: %f", EndTime - StartTime), 350, 150, 50, RED);
        }
        
        //DrawFPS(10,10);

        EndDrawing();
    }
    return 0;
}