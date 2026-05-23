#include "raylib.h"
#pragma comment(linker, "/SUBSYSTEM:CONSOLE")
#include <cmath>

int main() {
    const int screenWidth = 1200;
    const int screenHeight = 800;

    InitWindow(screenWidth, screenHeight, "lol_sdk_bro]");

    const int mapWidth = 8;
    const int mapHeight = 8;
    int map[mapHeight][mapWidth] = {
        {1, 1, 1, 1, 1, 1, 1, 1},
        {1, 0, 0, 0, 1, 0, 0, 1},
        {1, 0, 0, 0, 1, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 1},
        {1, 1, 1, 0, 1, 1, 0, 1},
        {1, 0, 0, 0, 0, 1, 0, 1},
        {1, 0, 1, 1, 0, 0, 0, 1},
        {1, 1, 1, 1, 1, 1, 1, 1}
    };

    // НАСТРОЙКА КАМЕРЫ (ИГРОК)
    Camera3D camera = { 0 };
    camera.position = Vector3{ 2.0f, 0.6f, 2.0f };
    camera.target = Vector3{ 3.0f, 0.6f, 2.0f };
    camera.up = Vector3{ 0.0f, 1.0f, 0.0f };
    camera.fovy = 60.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    float cameraAngleX = 0.0f;
    float cameraAngleY = 0.0f;

    // --- НАСТРОЙКА ПЛАВНОЙ ФИЗИКИ ПРЫЖКА ---
    float playerHeight = 0.6f;
    float verticalVelocity = 0.0f;
    bool isGrounded = true;
    const float gravity = -2.5f;
    const float jumpForce = 1.2f;

    // Загрузка текстуры. Оставляем относительный путь
    Texture2D wallTexture = LoadTexture("52.png");

    // Создаем сетку куба и преобразуем в модель. Встроенная развертка меша будет работать стабильно
    Mesh cubeMesh = GenMeshCube(1.0f, 2.0f, 1.0f);
    Model wallModel = LoadModelFromMesh(cubeMesh);

    // Привязываем загруженную текстуру к материалу нашей модели стен
    if (wallTexture.id > 0) {
        wallModel.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = wallTexture;
    }

    DisableCursor();
    SetTargetFPS(60);

    const float moveSpeed = 3.0f;
    const float mouseSpeed = 0.0025f;

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();

        // --- УПРАВЛЕНИЕ КАМЕРОЙ (МЫШЬ) ---
        Vector2 mouseDelta = GetMouseDelta();
        cameraAngleX += mouseDelta.x * mouseSpeed;
        cameraAngleY -= mouseDelta.y * mouseSpeed;

        if (cameraAngleY > 1.4f) cameraAngleY = 1.4f;
        if (cameraAngleY < -1.4f) cameraAngleY = -1.4f;

        Vector3 forward = {
            cosf(cameraAngleY) * cosf(cameraAngleX),
            sinf(cameraAngleY),
            cosf(cameraAngleY) * sinf(cameraAngleX)
        };

        Vector3 walkForward = { cosf(cameraAngleX), 0.0f, sinf(cameraAngleX) };
        Vector3 right = { -sinf(cameraAngleX), 0.0f, cosf(cameraAngleX) };

        Vector3 oldPosition = camera.position;
        Vector3 desiredPosition = camera.position;

        // --- ОБРАБОТКА НАЖАТИЙ WASD ---
        if (IsKeyDown(KEY_W)) {
            desiredPosition.x += walkForward.x * moveSpeed * dt;
            desiredPosition.z += walkForward.z * moveSpeed * dt;
        }
        if (IsKeyDown(KEY_S)) {
            desiredPosition.x -= walkForward.x * moveSpeed * dt;
            desiredPosition.z -= walkForward.z * moveSpeed * dt;
        }
        if (IsKeyDown(KEY_A)) {
            desiredPosition.x -= right.x * moveSpeed * dt;
            desiredPosition.z -= right.z * moveSpeed * dt;
        }
        if (IsKeyDown(KEY_D)) {
            desiredPosition.x += right.x * moveSpeed * dt;
            desiredPosition.z += right.z * moveSpeed * dt;
        }

        // --- ЛОГИКА ПРЫЖКА И ГРАВИТАЦИИ ---
        if (isGrounded && IsKeyPressed(KEY_SPACE)) {
            verticalVelocity = jumpForce;
            isGrounded = false;
        }

        verticalVelocity += gravity * dt;
        desiredPosition.y += verticalVelocity * dt;

        if (desiredPosition.y <= playerHeight) {
            desiredPosition.y = playerHeight;
            verticalVelocity = 0.0f;
            isGrounded = true;
        }

        // --- РАЗДЕЛЬНАЯ КОЛЛИЗИЯ СО СТЕНАМИ ---
        float playerRadius = 0.2f;

        // Коллизия по оси X
        camera.position.x = desiredPosition.x;
        for (int z = 0; z < mapHeight; z++) {
            for (int x = 0; x < mapWidth; x++) {
                if (map[z][x] == 1) {
                    BoundingBox wallBox = { Vector3{ (float)x - 0.5f, 0.0f, (float)z - 0.5f }, Vector3{ (float)x + 0.5f, 2.0f, (float)z + 0.5f } };
                    if (CheckCollisionBoxSphere(wallBox, camera.position, playerRadius)) camera.position.x = oldPosition.x;
                }
            }
        }

        // Коллизия по оси Z
        camera.position.z = desiredPosition.z;
        for (int z = 0; z < mapHeight; z++) {
            for (int x = 0; x < mapWidth; x++) {
                if (map[z][x] == 1) {
                    BoundingBox wallBox = { Vector3{ (float)x - 0.5f, 0.0f, (float)z - 0.5f }, Vector3{ (float)x + 0.5f, 2.0f, (float)z + 0.5f } };
                    if (CheckCollisionBoxSphere(wallBox, camera.position, playerRadius)) camera.position.z = oldPosition.z;
                }
            }
        }

        camera.position.y = desiredPosition.y;

        camera.target.x = camera.position.x + forward.x;
        camera.target.y = camera.position.y + forward.y;
        camera.target.z = camera.position.z + forward.z;

        // --- РЕНДЕРИНГ КАДРА ---
        BeginDrawing();
        ClearBackground(RAYWHITE);

        BeginMode3D(camera);
        DrawGrid(20, 1.0f);

        for (int z = 0; z < mapHeight; z++) {
            for (int x = 0; x < mapWidth; x++) {
                if (map[z][x] == 1) {
                    Vector3 cubePos = { (float)x, 1.0f, (float)z };
                    // Отрисовываем модель. Текстура наложится сама, если файл лежит в нужной папке
                    DrawModel(wallModel, cubePos, 1.0f, WHITE);
                }
            }
        }
        EndMode3D();

        // 2D Прицел
        DrawRectangle(screenWidth / 2 - 2, screenHeight / 2 - 10, 4, 20, RED);
        DrawRectangle(screenWidth / 2 - 10, screenHeight / 2 - 2, 20, 4, RED);

        DrawText("WASD - Hod'ba, SPACE - Pryzhok, ESC - Vyhod", 10, 10, 20, DARKGRAY);

        EndDrawing();
    }

    // Очистка памяти
    if (wallTexture.id > 0) UnloadTexture(wallTexture);
    UnloadModel(wallModel);

    CloseWindow();
    return 0;
}
