#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>
#include <ostream>
#include <raylib.h>
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>


struct ASTNode {
    std::string name;
    std::vector<ASTNode*> children;
    Vector2 position;
    float animationProgress;
    bool highlighted;
    ASTNode(std::string nodeName) : name(nodeName), position({0, 0}), animationProgress(0), highlighted(false) {}
};


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

Color LerpColor(Color c1, Color c2, float t) {
    return {
        static_cast<unsigned char>(c1.r + t * (c2.r - c1.r)),
        static_cast<unsigned char>(c1.g + t * (c2.g - c1.g)),
        static_cast<unsigned char>(c1.b + t * (c2.b - c1.b)),
        static_cast<unsigned char>(c1.a + t * (c2.a - c1.a))
    };
}


float CustomEaseElasticOut(float t) {
    if (t == 0 || t == 1) return t;
    float p = 0.3f;
    return powf(2, -10 * t) * sinf((t - p / 4) * (2 * PI) / p) + 1;
}


void DrawLineBezierQuad(Vector2 startPos, Vector2 endPos, Vector2 controlPos, float thick, Color color) {
    float t = 0;
    Vector2 previous = startPos;
    for (int i = 1; i <= 20; i++) {
        t = i / 20.0f;
        float u = 1 - t;
        float tt = t * t;
        float uu = u * u;
        Vector2 p = {
            uu * startPos.x + 2 * u * t * controlPos.x + tt * endPos.x,
            uu * startPos.y + 2 * u * t * controlPos.y + tt * endPos.y
        };
        DrawLineEx(previous, p, thick, color);
        previous = p;
    }
}

bool SearchNode(ASTNode* node, const std::string& query) {
    if (node->name.find(query) != std::string::npos) {
        node->highlighted = true;
        return true;
    }
    bool childMatched = false;
    for (auto& child : node->children) {
        if (SearchNode(child, query)) {
            childMatched = true;
        }
    }
    node->highlighted = childMatched;
    return childMatched;
}

void ResetHighlight(ASTNode* node) {
    node->highlighted = false;
    for (auto& child : node->children) {
        ResetHighlight(child);
    }
}

void DrawAST(ASTNode* node, Vector2 position, float x_offset, float y_offset, float zoom, Vector2 offset, float time) {
    Vector2 adjustedPosition = { (position.x + offset.x) * zoom, (position.y + offset.y) * zoom };
    
    
    node->animationProgress = fmin(node->animationProgress + 0.05f, 1.0f);
    float scale = CustomEaseElasticOut(node->animationProgress);
    
    
    Color baseColor = node->highlighted ? ORANGE : SKYBLUE;
    Color gradientColor = LerpColor(baseColor, DARKBLUE, (sinf(time * 2 + adjustedPosition.x * 0.01f) + 1) * 0.5f);
    
    
    DrawCircleGradient(adjustedPosition.x, adjustedPosition.y, 40 * zoom * scale, gradientColor, DARKBLUE);
    DrawCircleLines(adjustedPosition.x, adjustedPosition.y, 40 * zoom * scale, WHITE);
    
    
    int fontSize = 10 * zoom * scale;
    DrawText(node->name.c_str(), adjustedPosition.x - MeasureText(node->name.c_str(), fontSize) / 2, adjustedPosition.y - fontSize / 2, fontSize, WHITE);
    
    float x_pos = position.x - (x_offset * (node->children.size() - 1)) / 2;
    for (auto& child : node->children) {
        Vector2 childPos = {x_pos, position.y + y_offset};
        Vector2 adjustedChildPos = {(childPos.x + offset.x) * zoom, (childPos.y + offset.y) * zoom};
        
        
        Vector2 control = {(adjustedPosition.x + adjustedChildPos.x) / 2, (adjustedPosition.y + adjustedChildPos.y) / 2};
        Color lineColor = (node->highlighted && child->highlighted) ? ORANGE : Fade(LIGHTGRAY, 0.6f);
        DrawLineBezierQuad(adjustedPosition, adjustedChildPos, control, 2 * zoom, lineColor);
        
        DrawAST(child, childPos, x_offset / 2, y_offset, zoom, offset, time);
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

int main(int argc, char* argv[]) {
    if (argc<2) {
        std::cerr << "Usage" << argv[0] << "<path-to-clangAST>" << std::endl;
        return 1;
    }
    InitWindow(1200, 900, "ASTViz");
    SetTargetFPS(60);

    std::string filePath = argv[1];
    
    std::ifstream astFile(filePath);
    nlohmann::json astJson;
    astFile >> astJson;
    ASTNode* root = ParseAST(astJson);
    
    float zoom = 1.0f;
    Vector2 offset = {0, 0};
    Vector2 dragStart = {0, 0};
    bool dragging = false;
    bool typing = false;
    std::string searchQuery = "";
    
    while (!WindowShouldClose()) {
        float time = GetTime();
        float zoomSpeed = 0.1f;
        zoom += GetMouseWheelMove() * zoomSpeed;
        
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
            if (!typing && !searchQuery.empty()) {
                ResetHighlight(root);
                SearchNode(root, searchQuery);
            }
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
            DrawAST(root, {600, 100}, 1200, 200, zoom, offset, time);
            
            
            const char* title = "Abstract Syntax Tree";
            int titleFontSize = 40;
            Color titleColor = ColorFromHSV(fmodf(time * 60, 360), 0.7f, 1.0f);
            DrawText(title, (GetScreenWidth() - MeasureText(title, titleFontSize)) / 2, 20, titleFontSize, titleColor);
            
            
            DrawRectangle(10, GetScreenHeight() - 50, 300, 40, Fade(LIGHTGRAY, 0.3f));
            DrawRectangleLines(10, GetScreenHeight() - 50, 300, 40, LIGHTGRAY);
            DrawText(searchQuery.c_str(), 20, GetScreenHeight() - 40, 20, WHITE);
            DrawText(typing ? "Press ENTER to search" : "Press ENTER to type", 20, GetScreenHeight() - 70, 15, GRAY);
            
            
            DrawText("Type to search for nodes. Matching nodes and their ancestors will be highlighted in orange.", 10, GetScreenHeight() - 100, 15, GRAY);
        EndDrawing();
    }
    
    CloseWindow();
    return 0;
}
