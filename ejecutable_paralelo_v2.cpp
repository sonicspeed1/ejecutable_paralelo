#include <iostream>
#include <vector>
#include <string>
#include <omp.h>
#include <chrono>
#include <cstring>

#define N 100000000 // Tamaño de la secuencia
const std::string PATTERN = "ATGC"; // Patrón a buscar
const int P_LEN = PATTERN.length();

using namespace std;

// --------------------------------------------------------
// Generar secuencia simulada de ADN
// --------------------------------------------------------
void generate_sequence(vector<char>& seq) {
    for (int i = 0; i < N - P_LEN; ++i) {
        seq[i] = "ATGC"[rand() % 4];
    }

    // Insertamos el patrón cerca del final
    for (int i = 0; i < P_LEN; ++i) {
        seq[N - 10000 + i] = PATTERN[i]; 
    }
}

// --------------------------------------------------------
// Ejecutar búsqueda con threads + tipo de scheduling
// --------------------------------------------------------
void run_search(int num_threads, const char* schedule_type, int chunk_size) {

    vector<char> dna_sequence(N);
    generate_sequence(dna_sequence);
    long long first_index = -1; 

    omp_set_num_threads(num_threads);

    // Configurar el schedule dinámicamente
    if (strcmp(schedule_type, "static") == 0) {
        omp_set_schedule(omp_sched_static, chunk_size);
    } 
    else if (strcmp(schedule_type, "dynamic") == 0) {
        omp_set_schedule(omp_sched_dynamic, chunk_size);
    } 
    else if (strcmp(schedule_type, "guided") == 0) {
        omp_set_schedule(omp_sched_guided, chunk_size);
    } 
    else {
        omp_set_schedule(omp_sched_auto, chunk_size);
    }

    auto start = chrono::high_resolution_clock::now();

    // --------------------------------------------------------
    // PCAM: Parallel for con scheduling configurable
    // --------------------------------------------------------
    #pragma omp parallel for schedule(runtime)
    for (int i = 0; i < N - P_LEN; ++i) {

        if (first_index != -1) continue; // Si ya hay resultado, salimos rápido

        bool match = true;
        for (int j = 0; j < P_LEN; ++j) {
            if (dna_sequence[i + j] != PATTERN[j]) {
                match = false;
                break;
            }
        }

        if (match) {
            #pragma omp critical
            {
                if (first_index == -1 || i < first_index) {
                    first_index = i;
                }
            }
        }
    }

    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double> elapsed = end - start;

    cout << "Hilos: " << num_threads 
         << ", Schedule: " << schedule_type 
         << " (" << chunk_size << ")"
         << ", Tiempo: " << elapsed.count() << " s" 
         << ", Posición: " << first_index << endl;
}

// --------------------------------------------------------
// MAIN
// --------------------------------------------------------
int main() {
    cout << "--- Búsqueda de Patrón en ADN (" << PATTERN << ") ---" << endl;

    // Pruebas con distintos schedules
    run_search(4, "static", 1000);
    run_search(4, "dynamic", 1000);
    run_search(4, "guided", 1000);
    run_search(4, "auto", 1000);

    return 0;
}