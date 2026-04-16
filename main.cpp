#include <iostream>
#include <raylib.h>
#include <deque>
#include <vector>
#include <queue>
#include <cmath>
#include <raymath.h>

using namespace std;

static bool allowMove = false; // Makes it so that the snake doesn't move until I move it
// Some colors for the game
Color green = {173, 204, 96, 255};
Color darkGreen = {43, 51, 24, 255};
Color wallGreen = {90, 110, 50, 255};
Color poisonRed = {200, 40, 40, 255};
// Screen and grid settings
int cellSize = 30;
int cellCountX = 3;
int cellCountY = 4;
int offset = 75;
const int minMapWidth = 3;
const int minMapHeight = 4;
const int maxMapWidth = 30;
const int maxMapHeight = 30;

bool IsExcludedClassicSize(int width, int height){
    return width == 10 && height == 9;
}

void ClampClassicSize(int& width, int& height){
    width = max(minMapWidth, min(maxMapWidth, width));
    height = max(minMapHeight, min(maxMapHeight, height));
    if (IsExcludedClassicSize(width, height)){
        height = max(minMapHeight, height - 1);
    }
}

deque<Vector2> BuildInitialSnakeBody(){
    int startX = cellCountX - 1;
    if (startX > 2) startX = 2;
    if (startX < 0) startX = 0;

    int startY = cellCountY / 2;
    if (startY < 0) startY = 0;
    if (startY >= cellCountY) startY = cellCountY - 1;

    deque<Vector2> initialBody;
    initialBody.push_back(Vector2{(float)startX, (float)startY});
    if (startX - 1 >= 0) initialBody.push_back(Vector2{(float)(startX - 1), (float)startY});
    if (startX - 2 >= 0) initialBody.push_back(Vector2{(float)(startX - 2), (float)startY});
    return initialBody;
}
// Function to check if a Vector2 element is in a deque of Vector2
bool ElementInDeque(Vector2 element, deque<Vector2> deque){
    for (unsigned int i = 0; i < deque.size(); i++){
        if (Vector2Equals(deque[i], element)){
            return true;
        }
    }
    return false;
}
// Function to check if a Vector2 element is in a vector of Vector2
bool ElementInVector(Vector2 element, const vector<Vector2>& vec){
    for (unsigned int i = 0; i < vec.size(); i++){
        if (Vector2Equals(vec[i], element)){
            return true;
        }
    }
    return false;
}
// Enum for the different screen states
enum class ScreenState{
    Menu,
    Playing
};

enum class MapMode{
    Expanding,
    Classic
};
// Everything that has to do with the snake
class Snake{
public:
    deque<Vector2> body = BuildInitialSnakeBody();
    deque<Vector2> previousBody = body;
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
    // Makes the snakes body segment match the correct size and draws it to the screen
    void DrawScaledTexture(Texture2D texture, int drawX, int drawY){
        Rectangle source = Rectangle{0.0f, 0.0f, (float)texture.width, (float)texture.height};
        Rectangle dest = Rectangle{(float)drawX, (float)drawY, (float)cellSize, (float)cellSize};
        Vector2 origin = Vector2{0.0f, 0.0f};
        DrawTexturePro(texture, source, dest, origin, 0.0f, WHITE);
    }
    // Snake textures constructor
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
    // Snake textures destructor
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
    // Replaces the snake head texture to match the direction it's moving in
    Texture2D GetHeadTexture(){
        if (direction.x == 1) return headRight;
        if (direction.x == -1) return headLeft;
        if (direction.y == 1) return headDown;
        return headUp;
    }
    // Gets the direction from the difference of two positions, but only returns the dominant direction (the one with the biggest absolute value)
    Vector2 GetDominantDirection(Vector2 delta){
        if (fabs(delta.x) >= fabs(delta.y)){
            return Vector2{delta.x >= 0 ? 1.0f : -1.0f, 0.0f};
        }
        return Vector2{0.0f, delta.y >= 0 ? 1.0f : -1.0f};
    }
    // Replaces the snake tail texture to match the direction it's moving in
    Texture2D GetTailTextureForPositions(Vector2 beforeTail, Vector2 tail){
        Vector2 diff = GetDominantDirection(Vector2Subtract(tail, beforeTail));
        if (diff.x == 1) return tailRight;
        if (diff.x == -1) return tailLeft;
        if (diff.y == 1) return tailDown;
        return tailUp;
    }
    // Replaces the snake body segment texture to match the direction it's moving in
    Texture2D GetBodyTexture(Vector2 prev, Vector2 current, Vector2 next){
        Vector2 prevDir = GetDominantDirection(Vector2Subtract(prev, current));
        Vector2 nextDir = GetDominantDirection(Vector2Subtract(next, current));

        if ((prevDir.x != 0 && nextDir.x != 0) || fabs(prev.y - next.y) < 0.01f) return bodyHorizontal;
        if ((prevDir.y != 0 && nextDir.y != 0) || fabs(prev.x - next.x) < 0.01f) return bodyVertical;

        bool up = (prevDir.y < 0) || (nextDir.y < 0);
        bool down = (prevDir.y > 0) || (nextDir.y > 0);
        bool left = (prevDir.x < 0) || (nextDir.x < 0);
        bool right = (prevDir.x > 0) || (nextDir.x > 0);

        if (up && left) return bodyTopLeft;
        if (up && right) return bodyTopRight;
        if (down && left) return bodyBottomLeft;
        return bodyBottomRight;
    }
    // Makes the snake move smoothly 
    Vector2 GetInterpolatedPosition(unsigned int index, float alpha){
        Vector2 endPos = body[index];
        Vector2 startPos;

        if (previousBody.empty()){
            startPos = endPos;
        }
        else if (index < previousBody.size()){
            startPos = previousBody[index];
        }
        else{
            startPos = previousBody.back();
        }

        return Vector2Lerp(startPos, endPos, alpha);
    }
    // Draws the snake to the screen with the correct textures and interpolated positions
    void Draw(float alpha){
        for (unsigned int i = 0; i < body.size(); i++){
            Vector2 drawPos = GetInterpolatedPosition(i, alpha);
            float x = drawPos.x;
            float y = drawPos.y;
            int drawX = offset + (int)roundf(x * cellSize);
            int drawY = offset + (int)roundf(y * cellSize);

            if (i == 0){
                DrawScaledTexture(GetHeadTexture(), drawX, drawY);
            }
            else if (i == body.size() - 1){
                Vector2 tailStart = GetInterpolatedPosition(i - 1, alpha);
                DrawScaledTexture(GetTailTextureForPositions(tailStart, drawPos), drawX, drawY);
            }
            else{
                Vector2 prevPos = GetInterpolatedPosition(i - 1, alpha);
                Vector2 nextPos = GetInterpolatedPosition(i + 1, alpha);
                Texture2D segmentTexture = GetBodyTexture(prevPos, drawPos, nextPos);
                DrawScaledTexture(segmentTexture, drawX, drawY);
            }
        }
    }
    // Updates the snakes position and body segments, and adds a segment if it has just eaten food
    void Update(){
        previousBody = body;
        body.push_front(Vector2Add(body[0], direction));
        if (addSegment == true){
            addSegment = false;
        }
        else{
            body.pop_back();
        }
    }
    // Resets the snake to its starting position and direction
    void Reset(){
        body = BuildInitialSnakeBody();
        previousBody = body;
        direction = {1, 0};
    }
};
struct FruitSpawn{
    Vector2 position;
    int textureIndex;
    bool active;
};
// Everything that has nothing to do with the snake and the fruit
class Game{
public:
    Snake snake = Snake();
    static constexpr int fruitCount = 5;
    vector<FruitSpawn> fruits;
    vector<Texture2D> fruitTextures;
    bool running = true;
    int score = 0;
    int stage = 1;
    int applesThisStage = 0;
    int stageGoals[3] = {5, 5, 5};
    int stageTarget = 5;
    double moveSpeed = 0.2;
    vector<Vector2> walls;
    bool poisonActive = false;
    Vector2 poisonPos = {0, 0};
    Texture2D poisonTexture;
    Sound eatSound;
    Sound wallSound;
    MapMode mapMode = MapMode::Expanding;
    int classicMapWidth = minMapWidth;
    int classicMapHeight = minMapHeight;
    // Updates map dimensions based on the selected map mode.
    void UpdateMapSizeForStage(){
        if (mapMode == MapMode::Classic){
            ClampClassicSize(classicMapWidth, classicMapHeight);
            cellCountX = classicMapWidth;
            cellCountY = classicMapHeight;
            return;
        }

        int stageOffset = stage - 1;
        if (stageOffset < 0) stageOffset = 0;
        cellCountX = minMapWidth + stageOffset;
        cellCountY = minMapHeight + stageOffset;
    }
    // Game constructor
    Game(){
        InitAudioDevice();
        const char* fruitPaths[5] = {
            "Graphics/apple.png",
            "Graphics/Banana.png",
            "Graphics/Cherry.png",
            "Graphics/Peach.png",
            "Graphics/Watermelon.png"
        };

        for (int i = 0; i < 5; i++){
            fruitTextures.push_back(LoadTexture(fruitPaths[i]));
        }

        poisonTexture = LoadTexture("Graphics/apple.png");
        eatSound = LoadSound("Sounds/eat.mp3");
        wallSound = LoadSound("Sounds/wall.mp3");
        UpdateMapSizeForStage();
        RelocateFruits();
    }
    // Game destructor
    ~Game(){
        for (unsigned int i = 0; i < fruitTextures.size(); i++){
            UnloadTexture(fruitTextures[i]);
        }
        UnloadTexture(poisonTexture);
        UnloadSound(eatSound);
        UnloadSound(wallSound);
        CloseAudioDevice();
    }
    // Draws everything that needs to be drawn to the screen
    void Draw(float alpha){
        DrawFruits();
        DrawWalls();
        DrawPoison();
        snake.Draw(alpha);
    }
    // Updates the game logic, checks for collisions and updates the snake position
    void Update(){
        if (running){
            snake.Update();
            CheckCollisionWithFood();
            CheckCollisionWithEdges();
            CheckCollisionWithWalls();
            CheckCollisionWithTail();
        }
    }
    // Checks if a cell is blocked for the bot to move into
    bool IsCellBlockedForMove(Vector2 pos){
        if (pos.x < 0 || pos.x >= cellCountX || pos.y < 0 || pos.y >= cellCountY) return true;
        deque<Vector2> bodyToCheck = snake.body;
        if (!snake.addSegment && !bodyToCheck.empty()){
            bodyToCheck.pop_back();
        }
        if (ElementInDeque(pos, bodyToCheck)) return true;
        if (ElementInVector(pos, walls)) return true;
        if (poisonActive && Vector2Equals(pos, poisonPos)) return true;
        return false;
    }
    // Simulates snake body state after one move in the given direction
    deque<Vector2> SimulateBodyAfterMove(Vector2 dir){
        deque<Vector2> simulated = snake.body;
        simulated.push_front(Vector2Add(snake.body[0], dir));
        if (!snake.addSegment && !simulated.empty()){
            simulated.pop_back();
        }
        return simulated;
    }
    // Checks if a position is blocked in a hypothetical future state
    bool IsCellBlockedInState(Vector2 pos, const deque<Vector2>& stateBody){
        if (pos.x < 0 || pos.x >= cellCountX || pos.y < 0 || pos.y >= cellCountY) return true;
        if (ElementInDeque(pos, stateBody)) return true;
        if (ElementInVector(pos, walls)) return true;
        if (poisonActive && Vector2Equals(pos, poisonPos)) return true;
        return false;
    }
    // Shortest path distance between two cells in a hypothetical future state
    int GetPathDistance(Vector2 start, Vector2 target, const deque<Vector2>& stateBody){
        if (Vector2Equals(start, target)) return 0;

        vector<vector<int>> distance(cellCountY, vector<int>(cellCountX, -1));
        queue<Vector2> bfs;
        bfs.push(start);
        distance[(int)start.y][(int)start.x] = 0;

        Vector2 options[4] = {Vector2{1, 0}, Vector2{-1, 0}, Vector2{0, 1}, Vector2{0, -1}};

        while (!bfs.empty()){
            Vector2 current = bfs.front();
            bfs.pop();

            for (int i = 0; i < 4; i++){
                Vector2 next = Vector2Add(current, options[i]);

                if (next.x < 0 || next.x >= cellCountX || next.y < 0 || next.y >= cellCountY) continue;

                int nx = (int)next.x;
                int ny = (int)next.y;
                if (distance[ny][nx] != -1) continue;

                if (IsCellBlockedInState(next, stateBody) && !Vector2Equals(next, target)) continue;

                distance[ny][nx] = distance[(int)current.y][(int)current.x] + 1;
                if (Vector2Equals(next, target)){
                    return distance[ny][nx];
                }

                bfs.push(next);
            }
        }

        return -1;
    }
    // Counts how much open space is reachable from a cell in a hypothetical future state
    int GetReachableCellCount(Vector2 start, const deque<Vector2>& stateBody){
        if (IsCellBlockedInState(start, stateBody)) return 0;

        vector<vector<bool>> visited(cellCountY, vector<bool>(cellCountX, false));
        queue<Vector2> bfs;
        bfs.push(start);
        visited[(int)start.y][(int)start.x] = true;

        int reachable = 0;
        Vector2 options[4] = {Vector2{1, 0}, Vector2{-1, 0}, Vector2{0, 1}, Vector2{0, -1}};

        while (!bfs.empty()){
            Vector2 current = bfs.front();
            bfs.pop();
            reachable++;

            for (int i = 0; i < 4; i++){
                Vector2 next = Vector2Add(current, options[i]);
                if (next.x < 0 || next.x >= cellCountX || next.y < 0 || next.y >= cellCountY) continue;

                int nx = (int)next.x;
                int ny = (int)next.y;
                if (visited[ny][nx]) continue;
                if (IsCellBlockedInState(next, stateBody)) continue;

                visited[ny][nx] = true;
                bfs.push(next);
            }
        }

        return reachable;
    }
    // Makes the bot move
    Vector2 ChooseBotDirection(){
        Vector2 options[4] = {Vector2{1, 0}, Vector2{-1, 0}, Vector2{0, 1}, Vector2{0, -1}};
        Vector2 bestDir = snake.direction;
        int bestScore = 1000000;
        bool foundMove = false;

        for (int i = 0; i < 4; i++){
            Vector2 dir = options[i];
            if (Vector2Equals(dir, Vector2Scale(snake.direction, -1))) continue;

            Vector2 nextPos = Vector2Add(snake.body[0], dir);
            if (IsCellBlockedForMove(nextPos)) continue;

            deque<Vector2> simulatedBody = SimulateBodyAfterMove(dir);
            int pathDistance = -1;
            for (unsigned int fruitIndex = 0; fruitIndex < fruits.size(); fruitIndex++){
                if (!fruits[fruitIndex].active) continue;
                int candidateDistance = GetPathDistance(nextPos, fruits[fruitIndex].position, simulatedBody);
                if (candidateDistance >= 0 && (pathDistance < 0 || candidateDistance < pathDistance)){
                    pathDistance = candidateDistance;
                }
            }
            int openArea = GetReachableCellCount(nextPos, simulatedBody);
            int spaceNeeded = (int)simulatedBody.size();

            int score = 0;
            if (pathDistance >= 0){
                score += pathDistance * 20;
            }
            else{
                score += 2000;
            }

            if (openArea < spaceNeeded){
                score += 1500;
            }

            score -= openArea;

            // Slight preference for stable movement to avoid jittery turns.
            if (Vector2Equals(dir, snake.direction)){
                score -= 3;
            }

            if (score < bestScore){
                bestScore = score;
                bestDir = dir;
                foundMove = true;
            }
        }

        if (!foundMove){
            for (int i = 0; i < 4; i++){
                Vector2 dir = options[i];
                Vector2 nextPos = Vector2Add(snake.body[0], dir);
                if (!IsCellBlockedForMove(nextPos)){
                    return dir;
                }
            }
        }

        return bestDir;
    }
    // Gets the current stage goal, making sure to not go out of bounds of the stageGoals array
    int CurrentStageGoal() const {
        int index = stage - 1;
        if (index < 0) index = 0;
        if (index > 2) index = 2;
        return stageGoals[index];
    }
    // Generates a random cell position for the walls and poison fruit to spawn in
    Vector2 GenerateRandomCell(){
        float x = GetRandomValue(0, cellCountX - 1);
        float y = GetRandomValue(0, cellCountY - 1);
        return Vector2{x, y};
    }
    // Checks if a cell is blocked by the snake body
    bool IsCellBlocked(Vector2 pos, const deque<Vector2>& snakeBody, bool checkFood, bool checkPoison){
        if (ElementInDeque(pos, snakeBody)) return true;
        if (ElementInVector(pos, walls)) return true;
        if (checkFood && IsFruitAtPosition(pos)) return true;
        if (checkPoison && poisonActive && Vector2Equals(pos, poisonPos)) return true;
        return false;
    }
    // Generates a random position for the walls and poison fruit to spawn in, making sure they don't spawn on the snake body, food, or each other
    Vector2 GenerateRandomPos(const deque<Vector2>& snakeBody, bool checkFood, bool checkPoison){
        Vector2 position = GenerateRandomCell();
        while (IsCellBlocked(position, snakeBody, checkFood, checkPoison)){
            position = GenerateRandomCell();
        }
        return position;
    }
    // Checks if a fruit already exists at a given position
    bool IsFruitAtPosition(Vector2 pos, int ignoreIndex = -1){
        for (unsigned int i = 0; i < fruits.size(); i++){
            if ((int)i == ignoreIndex) continue;
            if (!fruits[i].active) continue;
            if (Vector2Equals(fruits[i].position, pos)) return true;
        }
        return false;
    }
    // Respawns all fruits while avoiding snake body, walls, poison fruit and fruit overlap
    void RelocateFruits(){
        fruits.clear();
        int totalCells = cellCountX * cellCountY;
        int availableCells = totalCells - (int)snake.body.size() - (int)walls.size() - (poisonActive ? 1 : 0);
        if (availableCells < 0) availableCells = 0;
        int spawnCount = min(fruitCount, availableCells);

        for (int i = 0; i < spawnCount; i++){
            FruitSpawn fruit;
            fruit.textureIndex = GetRandomValue(0, (int)fruitTextures.size() - 1);
            fruit.active = true;
            fruit.position = GenerateRandomPos(snake.body, true, true);
            while (IsFruitAtPosition(fruit.position)){
                fruit.position = GenerateRandomPos(snake.body, true, true);
            }
            fruits.push_back(fruit);
        }

        stageTarget = min(CurrentStageGoal(), (int)fruits.size());
    }
    // Relocates the poison to a new random position, avoiding snake body, walls, and food
    void RelocatePoison(){
        if (!poisonActive) return;
        poisonPos = GenerateRandomPos(snake.body, true, false);
        while (ElementInVector(poisonPos, walls) || IsFruitAtPosition(poisonPos)){
            poisonPos = GenerateRandomPos(snake.body, true, false);
        }
    }
    // Builds the stage by adding walls and poison fruit based on the current stage, and relocates the food
    void BuildStage(){
        UpdateMapSizeForStage();

        walls.clear();
        if (mapMode == MapMode::Classic && stage >= 2){
            int requestedWallCount = 8 + (stage - 2) * 4;
            int totalCells = cellCountX * cellCountY;
            int reservedCells = (int)snake.body.size() + fruitCount + (stage >= 3 ? 1 : 0);
            int maxSafeWalls = totalCells - reservedCells;
            if (maxSafeWalls < 0) maxSafeWalls = 0;
            int wallCount = min(requestedWallCount, maxSafeWalls);
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

        RelocateFruits();
    }
    // Checks if the snake head is colliding with the food, and if so, relocates the food, adds a segment to the snake, increases the score and stage progress, and plays the eat sound. Also checks if the stage goal has been reached and if so, advances to the next stage
    void CheckCollisionWithFood(){
        int eatenFruitIndex = -1;
        for (unsigned int i = 0; i < fruits.size(); i++){
            if (fruits[i].active && Vector2Equals(snake.body[0], fruits[i].position)){
                eatenFruitIndex = (int)i;
                break;
            }
        }

        if (eatenFruitIndex >= 0){
            fruits[eatenFruitIndex].active = false;
            snake.addSegment = true;
            score++;
            applesThisStage++;
            PlaySound(eatSound);
        }
        if(stageTarget > 0 && applesThisStage >= stageTarget){
            stage++;
            applesThisStage = 0;
            moveSpeed = max(0.06, moveSpeed * 0.9);
            score = 0;
            BuildStage();
        }
    }
    // Checks if the snake head is colliding with the edges of the screen
    void CheckCollisionWithEdges(){
        if (snake.body[0].x == cellCountX || snake.body[0].x == -1){
            GameOver();
        }
        if (snake.body[0].y == cellCountY || snake.body[0].y == -1){
            GameOver();
        }
    }
    // Checks if the snake head is colliding with the walls or poison fruit
    void CheckCollisionWithWalls(){
        if (stage >= 2 && ElementInVector(snake.body[0], walls)){
            GameOver();
        }
        if (poisonActive && Vector2Equals(snake.body[0], poisonPos)){
            GameOver();
        }
    }
    // Resets the game to the starting conditions of stage 1
    void GameOver(){
        stage = 1;
        UpdateMapSizeForStage();
        snake.Reset();
        applesThisStage = 0;
        stageTarget = stageGoals[0];
        moveSpeed = 0.2;
        walls.clear();
        poisonActive = false;
        RelocateFruits();
        running = false;
        score = 0;
        PlaySound(wallSound);
    }
    // Checks if the snake head is colliding with its own body, ignoring the head itself
    void CheckCollisionWithTail(){
        deque<Vector2> headlessBody = snake.body;
        headlessBody.pop_front();
        if (ElementInDeque(snake.body[0], headlessBody)){
            GameOver();
        }
    }
    // Draws the walls to the screen
    void DrawWalls(){
        for (unsigned int i = 0; i < walls.size(); i++){
            int drawX = offset + (int)(walls[i].x * cellSize);
            int drawY = offset + (int)(walls[i].y * cellSize);
            DrawRectangle(drawX, drawY, cellSize, cellSize, wallGreen);
        }
    }
    // Draws all fruits to the screen with their corresponding textures
    void DrawFruits(){
        for (unsigned int i = 0; i < fruits.size(); i++){
            if (!fruits[i].active) continue;
            Texture2D fruitTexture = fruitTextures[fruits[i].textureIndex];
            int drawX = offset + (int)(fruits[i].position.x * cellSize);
            int drawY = offset + (int)(fruits[i].position.y * cellSize);
            Rectangle source = Rectangle{0.0f, 0.0f, (float)fruitTexture.width, (float)fruitTexture.height};
            Rectangle dest = Rectangle{(float)drawX, (float)drawY, (float)cellSize, (float)cellSize};
            Vector2 origin = Vector2{0.0f, 0.0f};
            DrawTexturePro(fruitTexture, source, dest, origin, 0.0f, WHITE);
        }
    }
    // Draws the poison fruit to the screen
    void DrawPoison(){
        if (!poisonActive) return;
        int drawX = offset + (int)(poisonPos.x * cellSize);
        int drawY = offset + (int)(poisonPos.y * cellSize);
        Rectangle source = Rectangle{0.0f, 0.0f, (float)poisonTexture.width, (float)poisonTexture.height};
        Rectangle dest = Rectangle{(float)drawX, (float)drawY, (float)cellSize, (float)cellSize};
        Vector2 origin = Vector2{0.0f, 0.0f};
        DrawTexturePro(poisonTexture, source, dest, origin, 0.0f, WHITE);
    }
};

constexpr int Game::fruitCount;

// Main function, initializes the window and game, and contains the main game loop that checks for input, updates the game logic, and draws everything to the screen
int main(){
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    int screenWidth = 1000;
    int screenHeight = 700;
    InitWindow(screenWidth, screenHeight, "Retro Snake");
    SetExitKey(KEY_NULL);
    SetTargetFPS(60);

    Game game = Game();
    ScreenState screenState = ScreenState::Menu;
    int menuSelection = 0;
    bool botMode = false;
    MapMode selectedMapMode = MapMode::Expanding;
    int selectedClassicMapWidth = minMapWidth;
    int selectedClassicMapHeight = minMapHeight;
    bool paused = false;
    float movementAccumulator = 0.0f;

    auto StartNewRun = [&](bool startRunning, MapMode mode){
        allowMove = true;
        game.mapMode = mode;
        game.classicMapWidth = selectedClassicMapWidth;
        game.classicMapHeight = selectedClassicMapHeight;
        game.stage = 1;
        game.UpdateMapSizeForStage();
        game.snake.Reset();
        game.applesThisStage = 0;
        game.moveSpeed = 0.2;
        game.walls.clear();
        game.poisonActive = false;
        game.RelocateFruits();
        game.running = startRunning;
        game.score = 0;
        paused = false;
        movementAccumulator = 0.0f;
    };

    auto DrawCenteredText = [&](const char* text, int y, int fontSize, Color color){
        int textWidth = MeasureText(text, fontSize);
        int x = (GetScreenWidth() - textWidth) / 2;
        DrawText(text, x, y, fontSize, color);
    };

    while (WindowShouldClose() == false){
        float frameTime = GetFrameTime();
        BeginDrawing();

        if (screenState == ScreenState::Menu){
            if (IsKeyPressed(KEY_W) || IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_S) || IsKeyPressed(KEY_DOWN)){
                menuSelection = 1 - menuSelection;
            }
            if (IsKeyPressed(KEY_A) || IsKeyPressed(KEY_LEFT)){
                selectedMapMode = MapMode::Classic;
            }
            if (IsKeyPressed(KEY_D) || IsKeyPressed(KEY_RIGHT)){
                selectedMapMode = MapMode::Expanding;
            }
            if (selectedMapMode == MapMode::Classic){
                if (IsKeyPressed(KEY_Q)){
                    selectedClassicMapWidth = max(minMapWidth, selectedClassicMapWidth - 1);
                }
                if (IsKeyPressed(KEY_E)){
                    selectedClassicMapWidth = min(maxMapWidth, selectedClassicMapWidth + 1);
                }
                if (IsKeyPressed(KEY_Z)){
                    selectedClassicMapHeight = max(minMapHeight, selectedClassicMapHeight - 1);
                }
                if (IsKeyPressed(KEY_C)){
                    selectedClassicMapHeight = min(maxMapHeight, selectedClassicMapHeight + 1);
                }

                ClampClassicSize(selectedClassicMapWidth, selectedClassicMapHeight);
            }
            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)){
                botMode = (menuSelection == 1);
                screenState = ScreenState::Playing;
                StartNewRun(botMode, selectedMapMode);
            }

            int menuMapWidth = (selectedMapMode == MapMode::Classic) ? selectedClassicMapWidth : minMapWidth;
            int menuMapHeight = (selectedMapMode == MapMode::Classic) ? selectedClassicMapHeight : minMapHeight;

            ClearBackground(green);
            DrawRectangleLinesEx(Rectangle{(float)offset - 5, (float)offset - 5, (float)cellSize * menuMapWidth + 10, (float)cellSize * menuMapHeight + 10}, 5, darkGreen);
            int menuCenterY = GetScreenHeight() / 2;
            DrawCenteredText("Retro Snake", menuCenterY - 200, 52, darkGreen);
            DrawCenteredText("Select Mode", menuCenterY - 145, 30, darkGreen);
            Color playColor = (menuSelection == 0) ? darkGreen : wallGreen;
            Color botColor = (menuSelection == 1) ? darkGreen : wallGreen;
            DrawCenteredText("Play", menuCenterY - 95, 32, playColor);
            DrawCenteredText("Watch Bot", menuCenterY - 50, 32, botColor);
            DrawCenteredText("Enter/Space to start", menuCenterY - 8, 20, darkGreen);
            const char* modeLabel = (selectedMapMode == MapMode::Expanding) ? "Expanding Map" : "Classic Map";
            DrawCenteredText(TextFormat("Map Mode: %s", modeLabel), menuCenterY + 36, 28, darkGreen);
            DrawCenteredText("A/Left: Classic   D/Right: Expanding", menuCenterY + 74, 20, darkGreen);
            DrawCenteredText(TextFormat("Classic Size: %ix%i", selectedClassicMapWidth, selectedClassicMapHeight), menuCenterY + 105, 24, darkGreen);
            DrawCenteredText("Q/E width  Z/C height", menuCenterY + 138, 20, darkGreen);
        }
        else{
            if (IsKeyPressed(KEY_M) || IsKeyPressed(KEY_ESCAPE)){
                screenState = ScreenState::Menu;
                selectedMapMode = game.mapMode;
                selectedClassicMapWidth = game.classicMapWidth;
                selectedClassicMapHeight = game.classicMapHeight;
                allowMove = false;
                game.running = false;
                paused = false;
                movementAccumulator = 0.0f;
            }

            if (botMode && !game.running && (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE))){
                StartNewRun(true, game.mapMode);
            }

            if (IsKeyPressed(KEY_P) && game.running){
                paused = !paused;
            }

            if (!paused){
                movementAccumulator += frameTime;
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

            while (!paused && movementAccumulator >= game.moveSpeed){
                movementAccumulator -= (float)game.moveSpeed;
                if (botMode){
                    game.snake.direction = game.ChooseBotDirection();
                }
                game.Update();
                allowMove = true;
            }

            float interpolationAlpha = 0.0f;
            if (game.moveSpeed > 0.0){
                interpolationAlpha = movementAccumulator / (float)game.moveSpeed;
            }
            interpolationAlpha = Clamp(interpolationAlpha, 0.0f, 1.0f);

            // Drawing
            ClearBackground(green);
            DrawRectangleLinesEx(Rectangle{(float)offset - 5, (float)offset - 5, (float)cellSize * cellCountX + 10, (float)cellSize * cellCountY + 10}, 5, darkGreen);
            DrawCenteredText("Retro Snake", 20, 40, darkGreen);
            DrawCenteredText(TextFormat("Stage %i", game.stage), 65, 34, darkGreen);
            DrawCenteredText((game.mapMode == MapMode::Expanding) ? "Expanding" : "Classic", 102, 24, darkGreen);
            DrawCenteredText(TextFormat("Map %ix%i", cellCountX, cellCountY), 132, 24, darkGreen);
            DrawCenteredText(TextFormat("Score %i", game.score), GetScreenHeight() - 90, 34, darkGreen);
            DrawCenteredText("P to pause/resume   Esc to menu", GetScreenHeight() - 52, 20, darkGreen);
            if (botMode && !game.running){
                DrawCenteredText("Press Enter to restart bot", GetScreenHeight() - 26, 20, darkGreen);
            }
            if (paused){
                int pausedWidth = MeasureText("PAUSED", 50);
                int pausedX = (GetScreenWidth() - pausedWidth) / 2;
                int pausedY = (GetScreenHeight() - 50) / 2;
                DrawText("PAUSED", pausedX, pausedY, 50, darkGreen);
            }
            game.Draw(interpolationAlpha);
        }

        EndDrawing();
    }
    CloseWindow();
    return 0;
}