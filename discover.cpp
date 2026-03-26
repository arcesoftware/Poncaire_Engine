#include <SDL3/SDL.h>
#include <vector>
#include <cmath>
#include <algorithm>
#include <random>
#include <string>

// ------------------ STRUCTS ------------------

struct Vec3 { float x, y, z; };
struct Particle { Vec3 pos; SDL_Color color; };

struct PhysicalConstant {
    std::string name;
    float value;
    std::string raw;
};

// ------------------ PHYSICS TABLE ------------------

// --- Your Full Physics Table ---
std::vector<PhysicalConstant> physicsTable = {
    {"Standard Pi",3.14159f, "3.14159"},
    {"Muon g-factor", 2.0023318f, "-2.00233184123"},
    {"Electron g-factor", 2.0023193f, "-2.00231930436"},
    {"Cosmological Constant", 1.089f, "1.089e-52"},
    {"Reduced Planck", 1.0545718f, "1.054571817e-34"},
    {"Planck Constant", 6.6260701f, "6.62607015e-34"},
    {"Electron Mass", 9.1093837f, "9.1093837139e-31"},
    {"Thomson Cross Section", 6.6524587f, "6.6524587051e-29"},
    {"Muon Mass", 1.8835316f, "1.883531627e-28"},
    {"Atomic Mass Constant", 1.6605391f, "1.66053906892e-27"},
    {"Proton Mass", 1.6726219f, "1.67262192595e-27"},
    {"Neutron Mass", 1.6749275f, "1.67492750056e-27"},
    {"Tau Mass", 3.16754f, "3.16754e-27"},
    {"Nuclear Magneton", 5.0507837f, "5.0507837393e-27"},
    {"Bohr Magneton", 9.2740101f, "9.2740100657e-24"},
    {"Boltzmann Constant", 1.380649f, "1.380649e-23"},
    {"Elementary Charge", 1.6021766f, "1.602176634e-19"},
    {"Rydberg Energy", 2.1798724f, "2.1798723611e-18"},
    {"Hartree Energy", 4.3597447f, "4.3597447222e-18"},
    {"1st Radiation (Radiance)", 1.1910429f, "1.191042972e-16"},
    {"1st Radiation Constant", 3.7417718f, "3.741771852e-16"},
    {"Magnetic Flux Quantum", 2.0678338f, "2.067833848e-15"},
    {"Classical Electron Radius", 2.8179403f, "2.8179403205e-15"},
    {"Vacuum Permittivity", 8.8541878f, "8.8541878188e-12"},
    {"Bohr Radius", 5.2917721f, "5.29177210544e-11"},
    {"Gravitational Constant", 6.67430f, "6.67430e-11"},
    {"Molar Planck Constant", 3.9903127f, "3.990312712e-10"},
    {"Stefan-Boltzmann", 5.6703744f, "5.670374419e-8"},
    {"Vacuum Permeability", 1.2566371f, "1.256637061e-6"},
    {"Fermi Coupling", 1.1663787f, "1.1663787e-5"},
    {"Molar Vol Silicon", 1.2058832f, "1.205883199e-5"},
    {"Conductance Quantum", 7.7480917f, "7.748091729e-5"},
    {"Quantum Circulation", 3.6369475f, "3.6369475467e-4"},
    {"Molar Mass Constant", 1.0000000f, "1.00000000105e-3"},
    {"Wien Wavelength", 2.8977719f, "2.897771955e-3"},
    {"Wien Entropy", 3.0029161f, "3.002916077e-3"},
    {"Fine-Structure Constant", 7.2973525f, "7.2973525643e-3"},
    {"Molar Mass C-12", 1.2000000f, "1.20000000126e-2"},
    {"2nd Radiation Constant", 1.4387768f, "1.438776877e-2"},
    {"Weak Mixing Angle", 0.23153f, "0.23153"},
    {"W-to-Z Mass Ratio", 0.88145f, "0.88145"},
    {"Proton g-factor", 5.5856947f, "5.5856946893"},
    {"Molar Gas Constant", 8.3144626f, "8.314462618"},
    {"Inv Fine-Structure", 1.3703599f, "137.035999"},
    {"Vacuum Impedance", 3.7673031f, "376.730313412"},
    {"Proton-Electron Ratio", 1.8361527f, "1836.152673"},
    {"Inv Conductance Quantum", 1.2906403f, "12906.40372"},
    {"Von Klitzing Constant", 2.5812807f, "25812.80745"},
    {"Faraday Constant", 9.6485332f, "96485.3321"},
    {"Rydberg Constant", 1.0973731f, "10973731.568"},
    {"Speed of Light", 2.9979245f, "299792458"},
    {"Cesium Frequency", 9.1926317f, "9192631770"},
    {"Wien Frequency", 5.8789257f, "5.878925757e10"},
    {"Josephson Constant", 4.8359784f, "4.835978484e14"},
    {"Avogadro Constant", 6.0221407f, "6.02214076e23"}
};

// ------------------ GLOBALS ------------------

int WIDTH = 1280;
int HEIGHT = 800;

float camYaw = 0, camPitch = 0, camDist = 120;
bool mouseDown = false;

std::mt19937 rng(std::random_device{}());

// ------------------ HELPERS ------------------

float randf(float a, float b) {
    return std::uniform_real_distribution<float>(a, b)(rng);
}

int randi(int max) {
    return std::uniform_int_distribution<int>(0, max - 1)(rng);
}

float normalizeConstant(float v) {
    float logv = log10f(fabs(v) + 1e-9f);
    return std::tanh(logv);
}

float getValue(int idx) {
    return normalizeConstant(physicsTable[idx].value) * 3.0f;
}

// ------------------ DNA ------------------

struct DNA {
    int genes[6];
    float fitness = 0.0f;
};

// ------------------ POPULATION ------------------

const int POP_SIZE = 30;
std::vector<DNA> population;

// ------------------ SYSTEM ------------------

Vec3 step(Vec3 p, const DNA& d) {

    float a = getValue(d.genes[0]);
    float b = getValue(d.genes[1]);
    float c = getValue(d.genes[2]);
    float d2 = getValue(d.genes[3]);
    float e = getValue(d.genes[4]);

    float dx = sin(a * p.y) - p.z * cos(b * p.x);
    float dy = p.z * sin(c * p.x) - cos(d2 * p.y);
    float dz = sin(p.x) - p.y * cos(e * p.z);

    return { p.x + dx * 0.01f, p.y + dy * 0.01f, p.z + dz * 0.01f };
}

// ------------------ FITNESS ------------------

float evaluate(DNA& d) {
    Vec3 p = { 0.1f,0.1f,0.1f };
    float score = 0;

    for (int i = 0; i < 1500; i++) {
        p = step(p, d);

        float mag = fabs(p.x) + fabs(p.y) + fabs(p.z);

        if (mag > 200) return -1.0f;

        score += mag;
    }

    d.fitness = score;
    return score;
}

// ------------------ GENETICS ------------------

DNA randomDNA() {
    DNA d;
    for (int i = 0; i < 6; i++) d.genes[i] = randi(physicsTable.size());
    return d;
}

// 🔥 Crossover
DNA crossover(const DNA& a, const DNA& b) {
    DNA child;

    for (int i = 0; i < 6; i++) {
        child.genes[i] = (randf(0, 1) < 0.5f) ? a.genes[i] : b.genes[i];
    }

    return child;
}

// 🔥 Mutation
void mutate(DNA& d) {
    if (randf(0, 1) < 0.2f) {
        int i = randi(6);
        d.genes[i] = randi(physicsTable.size());
    }
}

// ------------------ PARTICLES ------------------

std::vector<Particle> particles;

void buildParticles(const DNA& d) {
    particles.clear();

    Vec3 p = { 0.1f,0.1f,0.1f };

    for (int i = 0; i < 50000; i++) {
        p = step(p, d);

        Uint8 r = (Uint8)(fabs(p.x) * 40);
        Uint8 g = (Uint8)(fabs(p.y) * 40);
        Uint8 b = (Uint8)(fabs(p.z) * 40);

        particles.push_back({ p,{r,g,b,255} });
    }
}

// ------------------ EVOLUTION ------------------

void evolve() {

    // Evaluate
    for (auto& d : population) evaluate(d);

    // Sort best first
    std::sort(population.begin(), population.end(),
        [](const DNA& a, const DNA& b) {
            return a.fitness > b.fitness;
        });

    // Keep top 50%
    int survivors = POP_SIZE / 2;

    // Breed
    for (int i = survivors; i < POP_SIZE; i++) {
        DNA parentA = population[randi(survivors)];
        DNA parentB = population[randi(survivors)];

        DNA child = crossover(parentA, parentB);
        mutate(child);

        population[i] = child;
    }

    // Build best
    buildParticles(population[0]);

    // Log best
    SDL_Log("Best Fitness: %f", population[0].fitness);
}

// ------------------ INPUT ------------------

void handleMouse(SDL_Event& e) {
    if (e.type == SDL_EVENT_MOUSE_BUTTON_DOWN) mouseDown = true;
    if (e.type == SDL_EVENT_MOUSE_BUTTON_UP) mouseDown = false;

    if (e.type == SDL_EVENT_MOUSE_MOTION && mouseDown) {
        camYaw += e.motion.xrel * 0.005f;
        camPitch += e.motion.yrel * 0.005f;
        camPitch = std::clamp(camPitch, -1.5f, 1.5f);
    }
}

// ------------------ MAIN ------------------

int main(int argc, char* argv[]) {

    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window* window = SDL_CreateWindow(
        "Genetic Physics Attractor Engine",
        WIDTH, HEIGHT,
        SDL_WINDOW_RESIZABLE
    );

    SDL_Renderer* renderer = SDL_CreateRenderer(window, NULL);

    // Init population
    for (int i = 0; i < POP_SIZE; i++)
        population.push_back(randomDNA());

    bool running = true;
    SDL_Event e;

    while (running) {

        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_EVENT_QUIT) running = false;
            handleMouse(e);
        }

        // 🔥 Continuous evolution
        evolve();

        // Camera
        const bool* keys = SDL_GetKeyboardState(NULL);
        if (keys[SDL_SCANCODE_W]) camDist -= 2;
        if (keys[SDL_SCANCODE_S]) camDist += 2;

        // Render
        SDL_SetRenderDrawColor(renderer, 5, 5, 10, 255);
        SDL_RenderClear(renderer);

        float cY = cos(-camYaw), sY = sin(-camYaw);
        float cP = cos(-camPitch), sP = sin(-camPitch);

        for (auto& p : particles) {

            float rx = p.pos.x * cY - p.pos.z * sY;
            float rz_t = p.pos.x * sY + p.pos.z * cY;

            float ry = p.pos.y * cP - rz_t * sP;
            float rz = p.pos.y * sP + rz_t * cP + camDist;

            if (rz > 1.0f) {
                float sx = rx * 800.0f / rz + WIDTH / 2;
                float sy = ry * 800.0f / rz + HEIGHT / 2;

                SDL_SetRenderDrawColor(renderer,
                    p.color.r, p.color.g, p.color.b, 255);

                SDL_RenderPoint(renderer, sx, sy);
            }
        }

        SDL_RenderPresent(renderer);
    }

    SDL_Quit();
    return 0;
}
