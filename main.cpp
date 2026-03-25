#include <SDL3/SDL.h>
#include <vector>
#include <cmath>
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

// --- Configuration ---
int windowWidth = 1280;
int windowHeight = 800;

// --- Hypercube State ---
int currentDim = 4;
float rotationAngle = 0.0f;
float rotationSpeed = 0.01f;
bool isRotating = true;

struct Camera {
    float x = 0, y = 0, z = -10.0f;
    float pitch = 0.0f;
    float yaw = 0.0f;
    float speed = 0.12f;
    float sensitivity = 0.002f;
} cam;

// --- Input State ---
std::vector<bool> keys(SDL_SCANCODE_COUNT, false);
bool mouseLocked = true;
float zoom = 1.0f;

struct PointND {
    std::vector<float> coords;
    uint32_t id;
};

std::vector<PointND> vertices;

// Generates vertices for an N-Dimensional Hypercube
void initHypercube(int dim) {
    vertices.clear();
    uint32_t numVertices = 1 << dim;
    for (uint32_t i = 0; i < numVertices; ++i) {
        PointND p;
        p.id = i;
        p.coords.resize(dim);
        for (int j = 0; j < dim; ++j) {
            // Map bits of the ID to -1.0 or 1.0 coordinates
            p.coords[j] = (i & (1 << j)) ? 1.0f : -1.0f;
        }
        vertices.push_back(p);
    }
}

// Projects N-Dimensions into 2D Screen Space
SDL_FPoint project(const std::vector<float>& coords) {
    float px = 0, py = 0, pz = 0;

    // 1. High-dimensional "Folding" (Rotation)
    for (int i = 0; i < (int)coords.size(); ++i) {
        float s = sinf(rotationAngle + i * 0.5f);
        float c = cosf(rotationAngle + i * 0.5f);
        px += coords[i] * c;
        py += coords[i] * s;
        pz += coords[i] * (i % 2 == 0 ? c : s);
    }

    // 2. Relative Translation to Camera
    float dx = px - cam.x;
    float dy = py - cam.y;
    float dz = pz - cam.z;

    // 3. FPS Rotation (Yaw/Pitch)
    float cosY = cosf(cam.yaw), sinY = sinf(cam.yaw);
    float x_rotated = dx * cosY - dz * sinY;
    float z_rotated = dx * sinY + dz * cosY;

    float cosP = cosf(cam.pitch), sinP = sinf(cam.pitch);
    float y_final = dy * cosP - z_rotated * sinP;
    float z_final = dy * sinP + z_rotated * cosP;

    // 4. Perspective Projection
    float viewDist = 5.0f;
    float f = (800.0f * zoom) / (z_final + viewDist);

    return {
        windowWidth / 2.0f + x_rotated * f,
        windowHeight / 2.0f + y_final * f
    };
}

int main(int argc, char* argv[]) {
    if (!SDL_Init(SDL_INIT_VIDEO)) return -1;

    SDL_Window* window = SDL_CreateWindow("Hyper Engine: Tab to Unlock Mouse",
        windowWidth, windowHeight,
        SDL_WINDOW_RESIZABLE);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, NULL);

    // Initial mouse state
    SDL_SetWindowRelativeMouseMode(window, true);

    initHypercube(currentDim);

    bool running = true;
    SDL_Event e;

    while (running) {
        // --- EVENT HANDLING ---
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_EVENT_QUIT) running = false;

            if (e.type == SDL_EVENT_KEY_DOWN) {
                keys[e.key.scancode] = true;

                // Toggle Mouse Lock
                if (e.key.scancode == SDL_SCANCODE_TAB) {
                    mouseLocked = !mouseLocked;
                    SDL_SetWindowRelativeMouseMode(window, mouseLocked);
                }

                if (e.key.scancode == SDL_SCANCODE_ESCAPE) running = false;
                if (e.key.scancode == SDL_SCANCODE_SPACE) isRotating = !isRotating;

                // U/I - Change Dimensions
                if (e.key.scancode == SDL_SCANCODE_U) {
                    currentDim = std::min(14, currentDim + 1);
                    initHypercube(currentDim);
                }
                if (e.key.scancode == SDL_SCANCODE_I) {
                    currentDim = std::max(1, currentDim - 1);
                    initHypercube(currentDim);
                }
                // O/P - Rotation Warp Speed
                if (e.key.scancode == SDL_SCANCODE_O) rotationSpeed += 0.005f;
                if (e.key.scancode == SDL_SCANCODE_P) rotationSpeed = std::max(0.0f, rotationSpeed - 0.005f);
            }

            if (e.type == SDL_EVENT_KEY_UP) {
                keys[e.key.scancode] = false;
            }

            if (e.type == SDL_EVENT_MOUSE_MOTION && mouseLocked) {
                cam.yaw += e.motion.xrel * cam.sensitivity;
                cam.pitch -= e.motion.yrel * cam.sensitivity;
                // Limit pitch to prevent backflips
                if (cam.pitch > 1.5f) cam.pitch = 1.5f;
                if (cam.pitch < -1.5f) cam.pitch = -1.5f;
            }

            if (e.type == SDL_EVENT_WINDOW_RESIZED) {
                SDL_GetWindowSize(window, &windowWidth, &windowHeight);
            }
        }

        // --- FPS MOVEMENT ---
        // Only move if mouse is locked (typical game behavior)
        if (mouseLocked) {
            float sY = sinf(cam.yaw), cY = cosf(cam.yaw);
            if (keys[SDL_SCANCODE_W]) { cam.x += sY * cam.speed; cam.z += cY * cam.speed; }
            if (keys[SDL_SCANCODE_S]) { cam.x -= sY * cam.speed; cam.z -= cY * cam.speed; }
            if (keys[SDL_SCANCODE_A]) { cam.x -= cY * cam.speed; cam.z += sY * cam.speed; }
            if (keys[SDL_SCANCODE_D]) { cam.x += cY * cam.speed; cam.z -= sY * cam.speed; }
            if (keys[SDL_SCANCODE_E]) cam.y += cam.speed; // Fly Up
            if (keys[SDL_SCANCODE_Q]) cam.y -= cam.speed; // Fly Down
        }

        // --- UPDATE ---
        if (isRotating) rotationAngle += rotationSpeed;

        // --- RENDER ---
        SDL_SetRenderDrawColor(renderer, 10, 10, 18, 255);
        SDL_RenderClear(renderer);

        // Pre-calculate all projected points
        std::vector<SDL_FPoint> proj(vertices.size());
        for (size_t i = 0; i < vertices.size(); ++i) {
            proj[i] = project(vertices[i].coords);
        }

        // Draw Edges (Mathematical Color Gradient)
        for (const auto& v : vertices) {
            for (int j = 0; j < currentDim; ++j) {
                uint32_t n = v.id ^ (1 << j); // Find neighbor in Jth dimension
                if (n > v.id) {
                    // Color based on the dimension of the edge
                    float hue = (float)j / (float)currentDim;
                    Uint8 r = (Uint8)(sinf(hue * 6.28f) * 127 + 128);
                    Uint8 g = (Uint8)(sinf(hue * 6.28f + 2.0f) * 127 + 128);
                    Uint8 b = (Uint8)(sinf(hue * 6.28f + 4.0f) * 127 + 128);

                    SDL_SetRenderDrawColor(renderer, r, g, b, 180);
                    SDL_RenderLine(renderer, proj[v.id].x, proj[v.id].y, proj[n].x, proj[n].y);
                }
            }
        }

        // Draw HUD status (Simple visual feedback)
        if (!mouseLocked) {
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_FRect cursorBox = { (float)windowWidth / 2 - 5, (float)windowHeight / 2 - 5, 10, 10 };
            SDL_RenderRect(renderer, &cursorBox); // Shows center when unlocked
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(10); // Cap around 100fps
    }

    SDL_Quit();
    return 0;
}