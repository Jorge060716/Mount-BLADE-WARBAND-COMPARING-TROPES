#pragma once
#include <string>
#include<vector>
using namespace std;

struct Tropa{
    string nombre;
    int nivel;
    string rol;
    int poder;
    int salario;
};

vector<Tropa> base_de_datos{
   // Reino Nórdico (9)
    {"Recluta Nórdico",                         6,  "Infantería",        2,  3},
    {"Soldado de a pie Nórdico",               10,  "Infantería",        3,  6},
    {"Cazador Nórdico",                        11,  "Arquero",           4,  7},
    {"Soldado de a pie Nórdico entrenado",     14,  "Infantería",        6, 11},
    {"Arquero Nórdico",                        15,  "Arquero",           7, 12},
    {"Guerrero Nórdico",                       19,  "Infantería",       10, 19},
    {"Arquero Nórdico veterano",               19,  "Arquero",          10, 19},
    {"Veterano Nórdico",                       24,  "Infantería",       15, 29},
    {"Campeón Nórdico (Huscarl)",              28,  "Infantería",       20, 38},

    // Reino de Vaegir (10)
    {"Recluta Vaegir",                          4,  "Infantería",        1,  1},
    {"Soldado de a pie Vaegir",                 9,  "Infantería",        3,  5},
    {"Escaramuzador Vaegir",                   14,  "Arquero",           6, 11},
    {"Veterano Vaegir",                        14,  "Infantería",        6, 11},
    {"Arquero Vaegir",                         19,  "Arquero",          10, 19},
    {"Infantería Vaegir",                      19,  "Infantería",       10, 19},
    {"Arquero Vaegir (Tirador)",               24,  "Arquero",          15, 29},
    {"Jinete Vaegir",                          21,  "Caballería",       12, 38},
    {"Guardia Vaegir",                         24,  "Caballería",       15, 48},
    {"Caballero Vaegir",                       26,  "Caballería",       18, 55},

    // Reino de Swadia (10)
    {"Recluta Swadiano",                        4,  "Infantería",        1,  1},
    {"Miliciano Swadiano",                      9,  "Infantería",        3,  5},
    {"Escaramuzador Swadiano",                 14,  "Arquero",           6, 11},
    {"Soldado de a pie Swadiano",              14,  "Infantería",        6, 11},
    {"Ballestero Swadiano",                    19,  "Arquero",          10, 19},
    {"Infantería Swadiana",                    20,  "Infantería",       11, 21},
    {"Hombre de Armas Swadiano",               21,  "Caballería",       12, 38},
    {"Tirador de Primera Swadiano",            24,  "Arquero",          15, 48},
    {"Sargento Swadiano",                      25,  "Caballería",       16, 51},
    {"Caballero Swadiano",                     28,  "Caballería",       20, 63},

    // Reino de Rhodok (9)
    {"Miembro de la tribu Rhodok",              4,  "Infantería",        1,  1},
    {"Soldado de a pie / Lancero Rhodok",       9,  "Infantería",        3,  5},
    {"Ballestero Rhodok",                      10,  "Arquero",           3,  6},
    {"Lancero Rhodok entrenado",               14,  "Infantería",        6, 11},
    {"Ballestero Rhodok entrenado",            15,  "Arquero",           7, 12},
    {"Lancero Rhodok veterano",                19,  "Infantería",       10, 19},
    {"Ballestero Rhodok veterano",             20,  "Arquero",          11, 21},
    {"Sargento Rhodok",                        25,  "Infantería",       16, 31},
    {"Ballestero Rhodok de élite",             25,  "Arquero",          16, 31},

    // Kanato Khergita (6)
    {"Miembro de la tribu Khergita",            5,  "Infantería",        1,  2},
    {"Escaramuzador Khergita",                 10,  "Arquero a caballo", 3, 10},
    {"Jinete Khergita",                        14,  "Caballería",        6, 18},
    {"Arquero montado Khergita",               14,  "Arquero a caballo", 6, 18},
    {"Arquero montado Khergita veterano",      21,  "Arquero a caballo",12, 38},
    {"Lancero Khergita",                       23,  "Caballería",       14, 45},

    // Sultanato Sarraní (10)
    {"Recluta Sarraní",                         4,  "Infantería",        1,  1},
    {"Lancero / Soldado a pie Sarraní",         9,  "Infantería",        3,  5},
    {"Escaramuzador Sarraní",                  14,  "Arquero",           6, 11},
    {"Soldado a pie Sarraní veterano",         14,  "Infantería",        6, 11},
    {"Arquero Sarraní",                        19,  "Arquero",          10, 19},
    {"Jinete Sarraní",                         20,  "Caballería",       11, 35},
    {"Infantería Sarraní",                     20,  "Infantería",       11, 21},
    {"Maestro arquero Sarraní",                24,  "Arquero a caballo",15, 48},
    {"Guarda Sarraní",                         25,  "Caballería",       16, 51},
    {"Mameluco Sarraní",                       27,  "Caballería",       19, 60},

    // Mercenarios y mujeres (14)
    {"Granjero",                                4,  "Infantería",        1,  1},
    {"Vigilante",                               9,  "Infantería",        3,  7},
    {"Maestro de Caravana",                     9,  "Infantería",        3,  7},
    {"Guardia de Caravana",                    14,  "Caballería",        6, 27},
    {"Ballestero Mercenario",                  19,  "Arquero",          10, 28},
    {"Espadachín Mercenario",                  20,  "Infantería",       11, 31},
    {"Jinete Mercenario",                      20,  "Caballería",       11, 52},
    {"Caballero Mercenario",                   25,  "Caballería",       16, 76},
    {"Espada Contratada",                      25,  "Infantería",       16, 46},
    {"Campesina",                               1,  "Infantería",        1,  1},
    {"Seguidora de Campamento",                 5,  "Infantería",        1,  3},
    {"Cazadora",                               10,  "Arquero",           3,  9},
    {"Mujer de Armas",                         16,  "Infantería",        8, 21},
    {"Hermana de la Espada",                   22,  "Caballería",       13, 61},
};

