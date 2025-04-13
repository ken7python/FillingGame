//c++ -std=c++11 -I. main.cpp libsockpp.a -lraylib -arch arm64 -o game && ./game localhost 12345
#include <iostream>
#include <sockpp/tcp_acceptor.h>
#include <sockpp/tcp_connector.h>
#include <pthread.h>
#include "raylib.h"
#define GRID_SIZE 5
#include <math.h>

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
    const char* server_name;
    const char* server_port;
    bool come;
    sockpp::tcp_connector* sock;
    pthread_mutex_t send_mtx;
    std::queue<packet> sendqueue;
    std::queue<packet> recvqueue;
    pthread_mutex_t recv_mtx;
    argument(){
        pthread_mutex_init(&send_mtx, nullptr);
        pthread_mutex_init(&recv_mtx, nullptr);
    }
    ~argument(){
        pthread_mutex_destroy(&send_mtx);
        pthread_mutex_destroy(&recv_mtx);
    }
};

void* sock_serverR(void* arg){
    
    sockpp::tcp_connector sock;
    sock.connect( sockpp::inet_address(((argument*)arg)->server_name, atoi(((argument*)arg)->server_port)) );

    int ready;
    sock.read(&ready, sizeof(ready));

    if(ready){
        ((argument*)arg)->come = true;
        ((argument*)arg)->sock = &sock;
        std::cout << "GO!!!!!!!!!!!!" << std::endl;
        while(1){
            packet p;
            sock.read(&p,sizeof(p));
            cout << p.row << "," << p.col << "," << ColorToInt(p.color) << endl;
            pthread_mutex_lock(&((argument*)arg)->recv_mtx);
            ((argument*)arg)->recvqueue.push(p);
            pthread_mutex_unlock(&((argument*)arg)->recv_mtx);
        }
    }

    return NULL;
}
void* sock_serverW(void* arg){
    while( !(((argument*)arg)->sock) ){
        usleep(1000*10);
    }

    sockpp::tcp_connector* sock = ((argument*)arg)->sock;

    while(1){
        pthread_mutex_lock(&((argument*)arg)->send_mtx);
        while( 0 < ((argument*)arg)->sendqueue.size()){
            auto p = ((argument*)arg)->sendqueue.front();
            ((argument*)arg)->sendqueue.pop();

            sock->write(&p,sizeof(p));
        }
        pthread_mutex_unlock(&((argument*)arg)->send_mtx);
        usleep(1000*10);
    }
    

    return NULL;
}

const int screenWidth = 800;
const int screenHeight = 450;

struct FullScreenFunc{
    int windowWidth;
    int windowHeight;
    float scale;
    int scaledW;
    int scaledH;
    int offsetX;
    int offsetY;

    void drawTarget(RenderTexture2D* target){
        BeginDrawing();
                ClearBackground(BLACK);
                windowWidth = GetScreenWidth();
                windowHeight = GetScreenHeight();
                scale = fminf((float)windowWidth / screenWidth, (float)windowHeight / screenHeight);
                scaledW = screenWidth * scale;
                scaledH = screenHeight * scale;
                offsetX = (windowWidth - scaledW) / 2;
                offsetY = (windowHeight - scaledH) / 2;

                DrawTexturePro(
                    target->texture,
                    (Rectangle){0.0f, 0.0f, (float)screenWidth, -(float)screenHeight},
                    (Rectangle){(float)offsetX, (float)offsetY, (float)scaledW, (float)scaledH},
                    (Vector2){0, 0},
                    0.0f,
                    WHITE
                );
        EndDrawing();
    }
    Vector2 GetGameMouse() {
        return (Vector2){
            (GetMouseX() - this->offsetX) / this->scale,
            (GetMouseY() - this->offsetY) / this->scale
        };
    }
};

// ./a.out 192.168.0.13 12345
// [  a0  ] [    a1    ] [ a2 ]
int main(int n,char* a[]){
    if (n < 3){
        cout << "./a.out 192.168.xx,xx yyyy" << endl;
        return 1;
    }

    //float StartTime = GetTime();
    //bool GameClear = false;
    //float EndTime;

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(screenWidth, screenHeight, "raylib [core] example - basic window");
    SetTargetFPS(60);

    FullScreenFunc fullScreen;
    RenderTexture2D target = LoadRenderTexture(screenWidth, screenHeight);

    argument arg;
    arg.server_name = a[1];
    arg.server_port = a[2];

    arg.come = false;
    arg.sock = nullptr;
    pthread_t thR;
    pthread_create(&thR,NULL,sock_serverR,&arg);

    pthread_t thW;
    pthread_create(&thW,NULL,sock_serverW,&arg);

    while(!WindowShouldClose()){
        if (arg.come){
            break;
        }
        BeginTextureMode(target);
            ClearBackground(BLACK);
            DrawText("Waitting a player...",350,250,50,WHITE);
        EndTextureMode();

        fullScreen.drawTarget(&target);
    }

    const int rows = screenHeight / GRID_SIZE;
    const int cols = screenWidth / GRID_SIZE;
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
            grids[j][i].SetPosition(fullScreen.GetGameMouse());
            grids[j][i].SetColor(BLACK);
            ++i;
        }
        i = 0;
        ++j;
    }

    float percent;
    float percent_enemy;

    int painted;
    int painted_enemy;
    Color enemy_color = WHITE;

    int GameTime = 60;
    int EndTime = GetTime() + GameTime;
    int ThisTime = GetTime();

    while (!WindowShouldClose()){
        pthread_mutex_lock(&arg.recv_mtx);
        while( arg.recvqueue.size() > 0){
            auto p = arg.recvqueue.front();
            arg.recvqueue.pop();
            grids[p.row][p.col].SetColor(enemy_color);
        }
        pthread_mutex_unlock(&arg.recv_mtx);

        ThisTime = GetTime();
        if (ThisTime > EndTime){
            break;
        }

        BeginTextureMode(target);
            ClearBackground(RAYWHITE);

            painted = 0;
            painted_enemy = 0;
            i = 0;
            j = 0;
            while(j < rows){
                while(i < cols){
                    bool changed = grids[j][i].ColPlayer(player);
                    if(changed){
                        packet pack = {j,i,player.GetColor()};
                        pthread_mutex_lock(&arg.send_mtx);
                        arg.sendqueue.push(pack);
                        pthread_mutex_unlock(&arg.send_mtx);
                    }
                    if (grids[j][i].SameColor(player.GetColor() )){
                        ++painted;
                    }
                    if (grids[j][i].SameColor(enemy_color) ){
                        ++painted_enemy;
                    }
                    grids[j][i].Draw();
                    ++i;
                }
                i = 0;
                ++j;
            }

            //player.move();
            player.position = fullScreen.GetGameMouse();
            player.Draw();

            percent = (float)painted / (rows * cols) * 100;
            percent_enemy = (float)painted_enemy / (rows * cols) * 100;
            /*
            if (percent == 100 && !GameClear){
                EndTime = GetTime();
                GameClear = true;
            }
                */

            DrawText(TextFormat("Player:%d%%", (int)percent), 500, 30, 50, RED);
            DrawText(TextFormat("Enemy:%d%%", (int)percent_enemy), 500, 90, 50, RED);
            DrawText(TextFormat("Time: %d", EndTime - ThisTime ), 30, 30, 50, RED);

            /*
            if (GameClear){
                DrawText(TextFormat("Game Clear!"), 300, 100, 50, RED);
                DrawText(TextFormat("Time: %f", EndTime - StartTime), 350, 150, 50, RED);
            }
                */
            
            //DrawFPS(10,10);
        EndTextureMode();
        fullScreen.drawTarget(&target);
    }

    while(!WindowShouldClose()){
        BeginTextureMode(target);
            ClearBackground(RAYWHITE);
            if (percent == percent_enemy){
                DrawText(TextFormat("Draw"), 300, 100, 50, RED);
            }else
            if (percent > percent_enemy){
                DrawText(TextFormat("You Win!"), 300, 100, 50, RED);
            }else
            if (percent < percent_enemy){
                DrawText(TextFormat("You Lose!"), 300, 100, 50, RED);
            }
        EndTextureMode();
        fullScreen.drawTarget(&target);
    }

    return 0;
}