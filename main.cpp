//c++ -std=c++11 main.cpp -lraylib && ./a.out
#include <iostream>
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

        void ColPlayer(Player player){
            if(CheckCollisionCircleRec(player.position,player.r,rec)){
                //cout << "Collision" << endl;
                this->color = player.GetColor();
            }
        }
    private:
        Rectangle rec;
        Color color;
};


int main(){
    const int screenWidth = 800;
    const int screenHeight = 450;

    float StartTime = GetTime();
    bool GameClear = false;
    float EndTime;

    InitWindow(screenWidth, screenHeight, "raylib [core] example - basic window");
    SetTargetFPS(60);

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
                grids[j][i].ColPlayer(player);
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