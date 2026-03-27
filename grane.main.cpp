#include <SDL3/SDL.h>
#include <vector>
#include <cmath>
#include <algorithm>
#include <random>

// ------------------ CONSTANTS & STRUCTS ------------------
const int WIDTH = 1280;
const int HEIGHT = 800;
const int NUM_NEIGHBORS = 13; // Set to 12 to see a perfect fit, 13 to see the struggle
const float SPHERE_RADIUS = 1.0f;
const float CONTACT_DIST = 2.0f; // 2 * Radius
const int POP_SIZE = 20;

struct Vec3 { float x, y, z; };
struct SphereDNA {
    Vec3 centers[NUM_NEIGHBORS];
    float fitness = 0.0f;
};

// ------------------ GLOBALS ------------------
float camYaw = 0, camPitch = 0, camDist = 150;
bool mouseDown = false;
std::mt19937 rng(std::random_device{}());
std::vector<SphereDNA> population;

// ------------------ HELPERS ------------------
float randf(float a, float b) { return std::uniform_real_distribution<float>(a, b)(rng); }

Vec3 normalize(Vec3 v) {
    float mag = sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
    if (mag == 0) return { 0, 1, 0 };
    return { v.x / mag, v.y / mag, v.z / mag };
}

// ------------------ PHYSICS & GA ------------------

void applyForceDirectedRelaxation(SphereDNA& d) {
    float pushStrength = 0.05f;
    for (int i = 0; i < NUM_NEIGHBORS; i++) {
        for (int j = 0; j < NUM_NEIGHBORS; j++) {
            if (i == j) continue;

            float dx = d.centers[i].x - d.centers[j].x;
            float dy = d.centers[i].y - d.centers[j].y;
            float dz = d.centers[i].z - d.centers[j].z;
            float dist = sqrtf(dx * dx + dy * dy + dz * dz);

            if (dist < CONTACT_DIST) {
                // Separation force
                float force = (CONTACT_DIST - dist) * pushStrength;
                Vec3 dir = normalize({ dx, dy, dz });
                d.centers[i].x += dir.x * force;
                d.centers[i].y += dir.y * force;
                d.centers[i].z += dir.z * force;
            }
        }
        // Constraint: Must stay exactly 2.0 units from origin (touching center)
        d.centers[i] = normalize(d.centers[i]);
        d.centers[i].x *= CONTACT_DIST;
        d.centers[i].y *= CONTACT_DIST;
        d.centers[i].z *= CONTACT_DIST;
    }
}

float evaluate(SphereDNA& d) {
    float penalty = 0.0f;
    for (int i = 0; i < NUM_NEIGHBORS; i++) {
        for (int j = i + 1; j < NUM_NEIGHBORS; j++) {
            float dx = d.centers[i].x - d.centers[j].x;
            float dy = d.centers[i].y - d.centers[j].y;
            float dz = d.centers[i].z - d.centers[j].z;
            float dist = sqrtf(dx * dx + dy * dy + dz * dz);
            if (dist < CONTACT_DIST) penalty += powf(CONTACT_DIST - dist, 2);
        }
    }
    d.fitness = 1.0f / (1.0f + penalty);
    return d.fitness;
}

void evolve() {
    for (auto& d : population) {
        applyForceDirectedRelaxation(d);
        evaluate(d);
    }

    std::sort(population.begin(), population.end(), [](const SphereDNA& a, const SphereDNA& b) {
        return a.fitness > b.fitness;
        });

    // Breed top 5 into the rest of the population
    for (int i = 5; i < POP_SIZE; i++) {
        SphereDNA& p1 = population[rand() % 5];
        SphereDNA& p2 = population[rand() % 5];
        for (int j = 0; j < NUM_NEIGHBORS; j++) {
            population[i].centers[j] = (randf(0, 1) < 0.5f) ? p1.centers[j] : p2.centers[j];
            // Mutation
            if (randf(0, 1) < 0.1f) {
                population[i].centers[j].x += randf(-0.1f, 0.1f);
                population[i].centers[j].y += randf(-0.1f, 0.1f);
                population[i].centers[j].z += randf(-0.1f, 0.1f);
            }
        }
    }
}

// ------------------ RENDERING ------------------

void drawWireframeSphere(SDL_Renderer* renderer, Vec3 center, float radius, SDL_Color col) {
    const int rings = 6, segments = 10;
    auto project = [&](Vec3 p) -> SDL_FPoint {
        float cY = cosf(-camYaw), sY = sinf(-camYaw);
        float cP = cosf(-camPitch), sP = sinf(-camPitch);
        float rx = p.x * cY - p.z * sY;
        float rz_t = p.x * sY + p.z * cY;
        float ry = p.y * cP - rz_t * sP;
        float rz = p.y * sP + rz_t * cP + camDist;
        return { rx * 1000.0f / rz + WIDTH / 2.0f, ry * 1000.0f / rz + HEIGHT / 2.0f };
        };

    SDL_SetRenderDrawColor(renderer, col.r, col.g, col.b, col.a);
    for (int r = 0; r <= rings; r++) {
        float phi1 = 3.1415f * r / rings;
        float phi2 = 3.1415f * (r + 1) / rings;
        for (int s = 0; s < segments; s++) {
            float theta1 = 2.0f * 3.1415f * s / segments;
            float theta2 = 2.0f * 3.1415f * (s + 1) / segments;

            Vec3 p1 = { center.x + radius * sinf(phi1) * cosf(theta1), center.y + radius * cosf(phi1), center.z + radius * sinf(phi1) * sinf(theta1) };
            Vec3 p2 = { center.x + radius * sinf(phi1) * cosf(theta2), center.y + radius * cosf(phi1), center.z + radius * sinf(phi1) * sinf(theta2) };
            Vec3 p3 = { center.x + radius * sinf(phi2) * cosf(theta1), center.y + radius * cosf(phi2), center.z + radius * sinf(phi2) * sinf(theta1) };

            SDL_FPoint sp1 = project(p1), sp2 = project(p2), sp3 = project(p3);
            SDL_RenderLine(renderer, sp1.x, sp1.y, sp2.x, sp2.y);
            SDL_RenderLine(renderer, sp1.x, sp1.y, sp3.x, sp3.y);
        }
    }
}

// ------------------ MAIN ------------------

int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow("Kissing Number Solver: GA + Force-Directed", WIDTH, HEIGHT, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, NULL);

    for (int i = 0; i < POP_SIZE; i++) {
        SphereDNA d;
        for (int j = 0; j < NUM_NEIGHBORS; j++) {
            d.centers[j] = normalize({ randf(-1,1), randf(-1,1), randf(-1,1) });
            d.centers[j].x *= CONTACT_DIST; d.centers[j].y *= CONTACT_DIST; d.centers[j].z *= CONTACT_DIST;
        }
        population.push_back(d);
    }

    bool running = true;
    SDL_Event e;
    while (running) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_EVENT_QUIT) running = false;
            if (e.type == SDL_EVENT_MOUSE_BUTTON_DOWN) mouseDown = true;
            if (e.type == SDL_EVENT_MOUSE_BUTTON_UP) mouseDown = false;
            if (e.type == SDL_EVENT_MOUSE_MOTION && mouseDown) {
                camYaw += e.motion.xrel * 0.005f;
                camPitch += e.motion.yrel * 0.005f;
            }
        }

        evolve();

        SDL_SetRenderDrawColor(renderer, 10, 10, 20, 255);
        SDL_RenderClear(renderer);

        float visualScale = 20.0f;
        SphereDNA& best = population[0];

        // Draw Center
        drawWireframeSphere(renderer, { 0,0,0 }, visualScale, { 255, 255, 255, 50 });

        // Draw Neighbors
        for (int i = 0; i < NUM_NEIGHBORS; i++) {
            Vec3 p = { best.centers[i].x * visualScale, best.centers[i].y * visualScale, best.centers[i].z * visualScale };
            SDL_Color c = (best.fitness > 0.999f) ? SDL_Color{ 0, 255, 0, 255 } : SDL_Color{ 255, 50, 50, 255 };
            drawWireframeSphere(renderer, p, visualScale, c);
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(1);
    }

    SDL_Quit();
    return 0;
}
