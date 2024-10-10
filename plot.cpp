#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>
#include <raylib.h>
#include <vector>

// ASTNode
struct ASTNode {
    std::string name;
    std::vector<ASTNode*> children;
    Vector2 position;

    ASTNode(std::string nodeName) : name(nodeName), position({0, 0}) {}
};

// Function to recursively parse the AST from JSON
ASTNode* ParseAST(const nlohmann::json& node) {
    std::string name = node.value("kind", "Unknown");
    ASTNode* astNode = new ASTNode(name);

    if (node.contains("inner")) {
        for (const auto& child : node["inner"]) {
            astNode->children.push_back(ParseAST(child));
        }
    }
    return astNode;
}


void DrawAST(ASTNode* node, Vector2 position, float x_offset, float y_offset, float zoom, Vector2 offset,const std::string searchQuery) {
    Vector2 adjustedPosition = { (position.x + offset.x) * zoom, (position.y + offset.y) * zoom };

    Color nodeColor = (node->name.find(searchQuery) != std::string::npos && !searchQuery.empty()) ? RED : DARKBLUE;

    DrawCircleV(adjustedPosition, 80 * zoom, nodeColor);
    DrawText(node->name.c_str(), adjustedPosition.x - MeasureText(node->name.c_str(), 10 * zoom) / 2, adjustedPosition.y - 5 * zoom, 10 * zoom, WHITE);

    float x_pos = position.x - (x_offset * (node->children.size() - 1)) / 2;
    for (auto& child : node->children) {
        Vector2 childPos = {x_pos, position.y + y_offset};
        Vector2 adjustedChildPos = {(childPos.x + offset.x) * zoom, (childPos.y + offset.y) * zoom};

        DrawLineV(adjustedPosition, adjustedChildPos, DARKGRAY);

        DrawAST(child, childPos, x_offset / 2, y_offset, zoom, offset,searchQuery);
        x_pos += x_offset;
    }
}

Vector2 Vector2Subtract(Vector2 v1, Vector2 v2) {
    return (Vector2){v1.x - v2.x, v1.y - v2.y};
}

Vector2 Vector2Add(Vector2 v1, Vector2 v2) {
    return (Vector2){v1.x + v2.x, v1.y + v2.y};
}

Vector2 Vector2Scale(Vector2 v, float scale) {
    return (Vector2){v.x * scale, v.y * scale};
}


int main() {
    InitWindow(800, 600, "ASTViz");
    SetTargetFPS(60);

    std::ifstream astFile("ast.json");
    nlohmann::json astJson;
    astFile >> astJson;

    ASTNode* root = ParseAST(astJson);

    float zoom = 1.0f;
    Vector2 offset = {0, 0};
    Vector2 dragStart = {0, 0};
    bool dragging =0;

    bool typing = 0;
    std::string searchQuery = "";


    while (!WindowShouldClose()) {
        float zoomSpeed = 0.1f;
        zoom += GetMouseWheelMove() * zoomSpeed;
        const char* bigText = "TREEEEEE";
        int fontSize = 80;
        int textWidth = MeasureText(bigText, fontSize);
        int xPosition = (GetScreenWidth() - textWidth) / 2; 
        int yPosition = 20;

        DrawText(bigText, xPosition, yPosition, fontSize, DARKGRAY);

        if (zoom < 0.2f) zoom = 0.2f;
        if (zoom > 3.0f) zoom = 3.0f;

        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            if (!dragging) {
                dragging = true;
                dragStart = GetMousePosition();
            } else {
                Vector2 currentMousePos = GetMousePosition();
                Vector2 dragDelta = Vector2Subtract(currentMousePos, dragStart);

                offset = Vector2Add(offset, Vector2Scale(dragDelta, 1.0f / zoom));
                dragStart = currentMousePos;
            }
        } else {
            dragging = false;
        }
        if (IsKeyPressed(KEY_ENTER)) {
            typing = !typing;
        }

        if (typing) {
            int key = GetKeyPressed();
            if (key >= 32 && key <= 125) {
                searchQuery += (char)key;
            }
            if (key == KEY_BACKSPACE && searchQuery.length() > 0) {
                searchQuery.pop_back();
            }
        }

        BeginDrawing();
        ClearBackground(BLACK);

        DrawAST(root, {400, 500}, 800, 400,zoom,offset,searchQuery);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}

