#include <iostream>
#include <raylib.h>
#include <deque>
#include <vector>
#include <cmath>
#include <raymath.h>

using namespace std;

static bool allowMove = false;
Color green = {173, 204, 96, 255};
Color darkGreen = {43, 51, 24, 255};
Color wallGreen = {90, 110, 50, 255};
Color poisonRed = {200, 40, 40, 255};

int cellSize = 30;
int cellCount = 25;
int offset = 75;

double lastUpdateTime = 0;

bool ElementInDeque(Vector2 element, deque<Vector2> deque){
    for (unsigned int i = 0; i < deque.size(); i++){
        if (Vector2Equals(deque[i], element)){
            return true;
        }
    }
    return false;
}

bool ElementInVector(Vector2 element, const vector<Vector2>& vec){
    for (unsigned int i = 0; i < vec.size(); i++){
        if (Vector2Equals(vec[i], element)){
            return true;
        }
    }
    return false;
}

bool EventTriggered(double interval){
    double currentTime = GetTime();
    if (currentTime - lastUpdateTime >= interval){
        lastUpdateTime = currentTime;
        return true;
    }
    return false;
}

enum class ScreenState{
    Menu,
    Playing
};

class Snake{
public:
    deque<Vector2> body = {Vector2{6, 9}, Vector2{5, 9}, Vector2{4, 9}};
    Vector2 direction = {1, 0};
    bool addSegment = false;
    Texture2D headUp;
    Texture2D headDown;
    Texture2D headLeft;
    Texture2D headRight;
    Texture2D tailUp;
    Texture2D tailDown;
    Texture2D tailLeft;
    Texture2D tailRight;
    Texture2D bodyHorizontal;
    Texture2D bodyVertical;
    Texture2D bodyTopLeft;
    Texture2D bodyTopRight;
    Texture2D bodyBottomLeft;
    Texture2D bodyBottomRight;

    void DrawScaledTexture(Texture2D texture, int drawX, int drawY){
        Rectangle source = Rectangle{0.0f, 0.0f, (float)texture.width, (float)texture.height};
        Rectangle dest = Rectangle{(float)drawX, (float)drawY, (float)cellSize, (float)cellSize};
        Vector2 origin = Vector2{0.0f, 0.0f};
        DrawTexturePro(texture, source, dest, origin, 0.0f, WHITE);
    }

    Snake(){
        headUp = LoadTexture("Graphics/head_up.png");
        headDown = LoadTexture("Graphics/head_down.png");
        headLeft = LoadTexture("Graphics/head_left.png");
        headRight = LoadTexture("Graphics/head_right.png");
        tailUp = LoadTexture("Graphics/tail_up.png");
        tailDown = LoadTexture("Graphics/tail_down.png");
        tailLeft = LoadTexture("Graphics/tail_left.png");
        tailRight = LoadTexture("Graphics/tail_right.png");
        bodyHorizontal = LoadTexture("Graphics/body_horizontal.png");
        bodyVertical = LoadTexture("Graphics/body_vertical.png");
        bodyTopLeft = LoadTexture("Graphics/body_tl.png");
        bodyTopRight = LoadTexture("Graphics/body_tr.png");
        bodyBottomLeft = LoadTexture("Graphics/body_bl.png");
        bodyBottomRight = LoadTexture("Graphics/body_br.png");
    }

    ~Snake(){
        UnloadTexture(headUp);
        UnloadTexture(headDown);
        UnloadTexture(headLeft);
        UnloadTexture(headRight);
        UnloadTexture(tailUp);
        UnloadTexture(tailDown);
        UnloadTexture(tailLeft);
        UnloadTexture(tailRight);
        UnloadTexture(bodyHorizontal);
        UnloadTexture(bodyVertical);
        UnloadTexture(bodyTopLeft);
        UnloadTexture(bodyTopRight);
        UnloadTexture(bodyBottomLeft);
        UnloadTexture(bodyBottomRight);
    }

    Texture2D GetHeadTexture(){
        if (direction.x == 1) return headRight;
        if (direction.x == -1) return headLeft;
        if (direction.y == 1) return headDown;
        return headUp;
    }

    Texture2D GetTailTexture(){
        Vector2 tail = body[body.size() - 1];
        Vector2 beforeTail = body[body.size() - 2];
        Vector2 diff = Vector2Subtract(tail, beforeTail);
        if (diff.x == 1) return tailRight;
        if (diff.x == -1) return tailLeft;
        if (diff.y == 1) return tailDown;
        return tailUp;
    }

    Texture2D GetBodyTexture(Vector2 prev, Vector2 current, Vector2 next){
        int prevX = (int)prev.x;
        int prevY = (int)prev.y;
        int currentX = (int)current.x;
        int currentY = (int)current.y;
        int nextX = (int)next.x;
        int nextY = (int)next.y;

        if (prevX == nextX) return bodyVertical;
        if (prevY == nextY) return bodyHorizontal;

        int dirPrevX = prevX - currentX;
        int dirPrevY = prevY - currentY;
        int dirNextX = nextX - currentX;
        int dirNextY = nextY - currentY;

        bool up = (dirPrevY == -1) || (dirNextY == -1);
        bool down = (dirPrevY == 1) || (dirNextY == 1);
        bool left = (dirPrevX == -1) || (dirNextX == -1);
        bool right = (dirPrevX == 1) || (dirNextX == 1);

        if (up && left) return bodyTopLeft;
        if (up && right) return bodyTopRight;
        if (down && left) return bodyBottomLeft;
        return bodyBottomRight;
    }

    void Draw(){
        for (unsigned int i = 0; i < body.size(); i++){
            float x = body[i].x;
            float y = body[i].y;
            int drawX = offset + (int)(x * cellSize);
            int drawY = offset + (int)(y * cellSize);

            if (i == 0){
                DrawScaledTexture(GetHeadTexture(), drawX, drawY);
            }
            else if (i == body.size() - 1){
                DrawScaledTexture(GetTailTexture(), drawX, drawY);
            }
            else{
                Texture2D segmentTexture = GetBodyTexture(body[i - 1], body[i], body[i + 1]);
                DrawScaledTexture(segmentTexture, drawX, drawY);
            }
        }
    }

    void Update(){
        body.push_front(Vector2Add(body[0], direction));
        if (addSegment == true){
            addSegment = false;
        }
        else{
            body.pop_back();
        }
    }

    void Reset(){
        body = {Vector2{6, 9}, Vector2{5, 9}, Vector2{4, 9}};
        direction = {1, 0};
    }
};

class Food{

public:
    Vector2 position;
    Texture2D texture;

    Food(deque<Vector2> snakeBody){
        Image image = LoadImage("Graphics/food.png");
        texture = LoadTextureFromImage(image);
        UnloadImage(image);
        position = GenerateRandomPos(snakeBody);
    }

    ~Food(){
        UnloadTexture(texture);
    }

    void Draw(){
        DrawTexture(texture, offset + position.x * cellSize, offset + position.y * cellSize, WHITE);
    }

    Vector2 GenerateRandomCell(){
        float x = GetRandomValue(0, cellCount - 1);
        float y = GetRandomValue(0, cellCount - 1);
        return Vector2{x, y};
    }

    Vector2 GenerateRandomPos(deque<Vector2> snakeBody){
        Vector2 position = GenerateRandomCell();
        while (ElementInDeque(position, snakeBody)){
            position = GenerateRandomCell();
        }
        return position;
    }
};

class Game{
public:
    Snake snake = Snake();
    Food food = Food(snake.body);
    bool running = true;
    int score = 0;
    int stage = 1;
    int applesThisStage = 0;
    int stageGoals[3] = {5, 5, 5};
    double moveSpeed = 0.2;
    vector<Vector2> walls;
    bool poisonActive = false;
    Vector2 poisonPos = {0, 0};
    Sound eatSound;
    Sound wallSound;

    Game(){
        InitAudioDevice();
        eatSound = LoadSound("Sounds/eat.mp3");
        wallSound = LoadSound("Sounds/wall.mp3");
    }

    ~Game(){
        UnloadSound(eatSound);
        UnloadSound(wallSound);
        CloseAudioDevice();
    }

    void Draw(){
        food.Draw();
        DrawWalls();
        DrawPoison();
        snake.Draw();
    }

    void Update(){
        if (running){
            snake.Update();
            CheckCollisionWithFood();
            CheckCollisionWithEdges();
            CheckCollisionWithWalls();
            CheckCollisionWithTail();
        }
    }

    bool IsCellBlockedForMove(Vector2 pos){
        if (pos.x < 0 || pos.x >= cellCount || pos.y < 0 || pos.y >= cellCount) return true;
        deque<Vector2> bodyToCheck = snake.body;
        if (!snake.addSegment && !bodyToCheck.empty()){
            bodyToCheck.pop_back();
        }
        if (ElementInDeque(pos, bodyToCheck)) return true;
        if (ElementInVector(pos, walls)) return true;
        if (poisonActive && Vector2Equals(pos, poisonPos)) return true;
        return false;
    }

    Vector2 ChooseBotDirection(){
        Vector2 options[4] = {Vector2{1, 0}, Vector2{-1, 0}, Vector2{0, 1}, Vector2{0, -1}};
        Vector2 bestDir = snake.direction;
        int bestScore = 1000000;

        for (int i = 0; i < 4; i++){
            Vector2 dir = options[i];
            if (Vector2Equals(dir, Vector2Scale(snake.direction, -1))) continue;

            Vector2 nextPos = Vector2Add(snake.body[0], dir);
            if (IsCellBlockedForMove(nextPos)) continue;

            int score = (int)(abs((int)(food.position.x - nextPos.x)) + abs((int)(food.position.y - nextPos.y)));
            if (score < bestScore){
                bestScore = score;
                bestDir = dir;
            }
        }

        return bestDir;
    }

    int CurrentStageGoal() const {
        int index = stage - 1;
        if (index < 0) index = 0;
        if (index > 2) index = 2;
        return stageGoals[index];
    }

    Vector2 GenerateRandomCell(){
        float x = GetRandomValue(0, cellCount - 1);
        float y = GetRandomValue(0, cellCount - 1);
        return Vector2{x, y};
    }

    bool IsCellBlocked(Vector2 pos, const deque<Vector2>& snakeBody, bool checkFood, bool checkPoison){
        if (ElementInDeque(pos, snakeBody)) return true;
        if (ElementInVector(pos, walls)) return true;
        if (checkFood && Vector2Equals(pos, food.position)) return true;
        if (checkPoison && poisonActive && Vector2Equals(pos, poisonPos)) return true;
        return false;
    }

    Vector2 GenerateRandomPos(const deque<Vector2>& snakeBody, bool checkFood, bool checkPoison){
        Vector2 position = GenerateRandomCell();
        while (IsCellBlocked(position, snakeBody, checkFood, checkPoison)){
            position = GenerateRandomCell();
        }
        return position;
    }

    void RelocateFood(){
        food.position = food.GenerateRandomPos(snake.body);
        while (ElementInVector(food.position, walls) || (poisonActive && Vector2Equals(food.position, poisonPos))){
            food.position = food.GenerateRandomPos(snake.body);
        }
    }

    void BuildStage(){
        if (stage < 2){
            walls.clear();
        }
        if (stage >= 2){
            walls.clear();
            int wallCount = 8 + (stage - 2) * 4;
            for (int i = 0; i < wallCount; i++){
                Vector2 pos = GenerateRandomPos(snake.body, true, true);
                walls.push_back(pos);
            }
        }

        if (stage >= 3){
            poisonActive = true;
            poisonPos = GenerateRandomPos(snake.body, true, false);
        }
        else{
            poisonActive = false;
        }

        RelocateFood();
    }

    void CheckCollisionWithFood(){
        if (Vector2Equals(snake.body[0], food.position)){
            RelocateFood();
            snake.addSegment = true;
            score++;
            applesThisStage++;
            PlaySound(eatSound);
        }
        if(applesThisStage >= CurrentStageGoal()){
            stage++;
            applesThisStage = 0;
            moveSpeed = max(0.06, moveSpeed * 0.9);
            score = 0;
            BuildStage();
        }
    }

    void CheckCollisionWithEdges(){
        if (snake.body[0].x == cellCount || snake.body[0].x == -1){
            GameOver();
        }
        if (snake.body[0].y == cellCount || snake.body[0].y == -1){
            GameOver();
        }
    }

    void CheckCollisionWithWalls(){
        if (stage >= 2 && ElementInVector(snake.body[0], walls)){
            GameOver();
        }
        if (poisonActive && Vector2Equals(snake.body[0], poisonPos)){
            GameOver();
        }
    }

    void GameOver(){
        snake.Reset();
        stage = 1;
        applesThisStage = 0;
        moveSpeed = 0.2;
        walls.clear();
        poisonActive = false;
        food.position = food.GenerateRandomPos(snake.body);
        running = false;
        score = 0;
        PlaySound(wallSound);
    }

    void CheckCollisionWithTail(){
        deque<Vector2> headlessBody = snake.body;
        headlessBody.pop_front();
        if (ElementInDeque(snake.body[0], headlessBody)){
            GameOver();
        }
    }

    void DrawWalls(){
        for (unsigned int i = 0; i < walls.size(); i++){
            int drawX = offset + (int)(walls[i].x * cellSize);
            int drawY = offset + (int)(walls[i].y * cellSize);
            DrawRectangle(drawX, drawY, cellSize, cellSize, wallGreen);
        }
    }

    void DrawPoison(){
        if (!poisonActive) return;
        int drawX = offset + (int)(poisonPos.x * cellSize);
        int drawY = offset + (int)(poisonPos.y * cellSize);
        DrawRectangle(drawX, drawY, cellSize, cellSize, poisonRed);
    }
};

int main(){
    InitWindow(2 * offset + cellSize * cellCount, 2 * offset + cellSize * cellCount, "Retro Snake");
    SetTargetFPS(60);

    Game game = Game();
    ScreenState screenState = ScreenState::Menu;
    int menuSelection = 0;
    bool botMode = false;

    while (WindowShouldClose() == false){
        BeginDrawing();

        if (screenState == ScreenState::Menu){
            if (IsKeyPressed(KEY_W) || IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_S) || IsKeyPressed(KEY_DOWN)){
                menuSelection = 1 - menuSelection;
            }
            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)){
                botMode = (menuSelection == 1);
                screenState = ScreenState::Playing;
                allowMove = false;
                game.snake.Reset();
                game.stage = 1;
                game.applesThisStage = 0;
                game.moveSpeed = 0.2;
                game.walls.clear();
                game.poisonActive = false;
                game.food.position = game.food.GenerateRandomPos(game.snake.body);
                game.running = true;
                game.score = 0;
            }

            ClearBackground(green);
            DrawRectangleLinesEx(Rectangle{(float)offset - 5, (float)offset - 5, (float)cellSize * cellCount + 10, (float)cellSize * cellCount + 10}, 5, darkGreen);
            DrawText("Retro Snake", offset - 5, 20, 40, darkGreen);
            int selectModeWidth = MeasureText("Select Mode", 30);
            DrawText("Select Mode", offset + cellSize * cellCount - selectModeWidth, offset - 40, 30, darkGreen);
            Color playColor = (menuSelection == 0) ? darkGreen : wallGreen;
            Color botColor = (menuSelection == 1) ? darkGreen : wallGreen;
            DrawText("Play", offset + 40, offset + 40, 32, playColor);
            DrawText("Watch Bot", offset + 40, offset + 90, 32, botColor);
            DrawText("Enter/Space to start", offset, offset + 150, 20, darkGreen);
        }
        else{
            if (IsKeyPressed(KEY_M)){
                screenState = ScreenState::Menu;
                allowMove = false;
                game.running = false;
            }

            if (EventTriggered(0.2)){
                allowMove = true;
                game.Update();
            }

            if (EventTriggered(game.moveSpeed)){
                allowMove = true;
                game.Update();
            }

            if (botMode && allowMove){
                game.snake.direction = game.ChooseBotDirection();
                allowMove = false;
            }

            if (!botMode){
                if (IsKeyPressed(KEY_W) && game.snake.direction.y != 1 && allowMove){
                    game.snake.direction = {0, -1};
                    game.running = true;
                    allowMove = false;
                }else if (IsKeyPressed(KEY_UP) && game.snake.direction.y != 1 && allowMove){
                    game.snake.direction = {0, -1};
                    game.running = true;
                    allowMove = false;
                }
                if (IsKeyPressed(KEY_S) && game.snake.direction.y != -1 && allowMove){
                    game.snake.direction = {0, 1};
                    game.running = true;
                    allowMove = false;
                }else if (IsKeyPressed(KEY_DOWN) && game.snake.direction.y != -1 && allowMove){
                    game.snake.direction = {0, 1};
                    game.running = true;
                    allowMove = false;
                }
                if (IsKeyPressed(KEY_A) && game.snake.direction.x != 1 && allowMove){
                    game.snake.direction = {-1, 0};
                    game.running = true;
                    allowMove = false;
                }else if (IsKeyPressed(KEY_LEFT) && game.snake.direction.x != 1 && allowMove){
                    game.snake.direction = {-1, 0};
                    game.running = true;
                    allowMove = false;
                }
                if (IsKeyPressed(KEY_D) && game.snake.direction.x != -1 && allowMove){
                    game.snake.direction = {1, 0};
                    game.running = true;
                    allowMove = false;
                }else if (IsKeyPressed(KEY_RIGHT) && game.snake.direction.x != -1 && allowMove){
                    game.snake.direction = {1, 0};
                    game.running = true;
                    allowMove = false;
                }
            }

            // Drawing
            ClearBackground(green);
            DrawRectangleLinesEx(Rectangle{(float)offset - 5, (float)offset - 5, (float)cellSize * cellCount + 10, (float)cellSize * cellCount + 10}, 5, darkGreen);
            DrawText("Retro Snake", offset - 5, 20, 40, darkGreen);
            DrawText(TextFormat("Stage %i", game.stage), offset + 450, 20, 40, darkGreen);
            DrawText(TextFormat("%i", game.score), offset - 5, offset + cellSize * cellCount + 10, 40, darkGreen);
            game.Draw();
        }

        EndDrawing();
    }
    CloseWindow();
    return 0;
}