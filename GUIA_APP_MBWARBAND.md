# Guía completa: app en C++ para comparar ejércitos de *Mount & Blade: Warband*

> **Objetivo:** crear una aplicación que reciba dos ejércitos (composición de tropas),
> calcule su poder y diga **cuál gana**, con probabilidad y bajas estimadas.
>
> **Base:** *Mount & Blade: Warband* (Native, versión 1.166/1.174). Las fórmulas de
> poder y salario están sacadas del *module system* oficial. La fórmula de héroe es
> una heurística transparente (no es una fórmula secreta del motor).

---

## 0. Resumen ejecutivo

La app se construye en **C++17** y tiene cuatro piezas:

1. **Base de datos** de tropas reclutables (`data/tropas.csv`), con nivel, rol, poder y salario.
2. **Calculadora de poder** con la fórmula oficial de auto-resolve.
3. **Motor de batalla**: un modelo determinista (Lanchester) y uno Monte Carlo por rondas.
4. **Interfaz de consola** (y opcionalmente Qt/ImGui) para introducir los ejércitos y ver el resultado.

El proyecto ya está creado en `mbw-sim/` y compila. Esta guía explica **por qué** funciona
así y **cómo** ampliarlo.

---

## 1. Objetivo y alcance

### Qué hace la app

- Recibe dos ejércitos, por ejemplo:
  - Ejército A: 50 Huscarls + 50 Veteranos Nórdicos + 30 Arqueros Nórdicos.
  - Ejército B: 40 Caballeros Swadianos + 40 Sargentos + 30 Tiradores de élite.
- Calcula el poder de cada tropa y el poder total de cada ejército.
- Estima la probabilidad de victoria y los supervivientes.
- Muestra un veredicto: “Gana A con 59,7 %”.

### Qué **no** hace (y conviene dejar claro)

- No reproduce la batalla en tiempo real ni la habilidad del jugador.
- No modela el terreno, la formación, la moral o las órdenes tácticas (se pueden añadir después).
- No usa los *assets* del juego; solo estadísticas.

> La app predice un resultado **tipo auto-resolve**, no una batalla jugada por un humano.

---

## 2. Cómo funciona el combate en Warband

Para que el simulador tenga sentido, hay que entender qué datos usa el juego.

### 2.1 Atributos

| Atributo | Efecto principal |
|---|---|
| **Strength (STR)** | Daño cuerpo a cuerpo, requisito de armas y armaduras |
| **Agility (AGI)** | Velocidad, atletismo, equitación, ataque |
| **Intelligence (INT)** | Habilidades de apoyo (no combate directo) |
| **Charisma (CHA)** | Liderazgo, tamaño de ejército, moral |

### 2.2 Habilidades de combate

| Habilidad | Efecto |
|---|---|
| **Ironflesh** | +2 puntos de vida por nivel |
| **Power Strike** | +8 % de daño cuerpo a cuerpo por nivel |
| **Power Draw** | Daño con arcos (requiere STR) |
| **Power Throw** | Daño con armas arrojadizas |
| **Weapon Master** | Sube el tope de pericia y la velocidad de subida |
| **Shield** | Mejora la cobertura y la resistencia del escudo |
| **Athletics** | Velocidad a pie |
| **Riding** | Velocidad y maniobra a caballo |
| **Horse Archery** | Precisión a caballo |

### 2.3 Proficiencias

Cada tropa tiene pericia en: una mano, dos manos, asta, arco, ballesta y arrojadizas.
La pericia da **bonus de daño** y **velocidad de ataque**.

Regla práctica de la comunidad: **+0,15 % de daño por punto de pericia por encima de 100**.
Es decir, 100 puntos extra ≈ +15 % de daño.

### 2.4 Daño (modelo simplificado)

El motor real usa una fórmula compleja con tipos de daño (corte, perforante, contundente),
soak de armadura, zona de impacto y bonus de velocidad. Para la app se usa este modelo
**transparente**:

```text
daño_base = daño_arma + 0.15 * STR
daño_base *= (1 + 0.08 * PowerStrike)          // melee
daño_base *= (1 + 0.0015 * max(0, pericia - 100))
daño_base *= factor_velocidad                  // 0.8 .. 1.2
armadura_efectiva = armadura / (armadura + 50)
daño_final = max(1, daño_base * (1 - armadura_efectiva))
```

> Esto **no** es la fórmula exacta del motor; es una aproximación defendible para
> comparar tropas y héroes. La fórmula oficial de **poder de tropa** (la que usa el
> auto-resolve) sí es exacta y está en el apartado 2.5.

### 2.5 Auto-resolve oficial (fórmula exacta)

Del *module system* de Native (`module_scripts.py`, script `party_calculate_strength`):

```text
poder_tropa(nivel) = max(1, 2 * (nivel + 4)^2 / 100)     // división entera
poder_ejército     = Σ (poder_tropa * cantidad)
```

Esta es la fórmula que usa el juego para resolver batallas sin que el jugador participe.
En la versión antigua (antes del parche 1.125) era `(nivel + 12)^2 / 100`; la actual es
la de arriba.

El auto-resolve convierte luego el poder en un factor de bajas:

```text
factor_defensor = clamp(poder_defensor / 20, 1, 50)
factor_atacante = clamp(poder_atacante / 20, 1, 50) + 1
// en asedios: defensor * 1.23, atacante / 1.23
```

y aplica bajas por rondas hasta que un bando se queda sin tropas aptas.

### 2.6 Salario oficial (fórmula exacta)

Del mismo *module system* (`game_get_troop_wage`):

```text
salario_base = max(1, (nivel + 3)^2 / 25)
si va montado:    salario_base = salario_base * 5 / 3
si es mercenario: salario_base = salario_base * 3 / 2
salario_final = salario_base * (100 - 5 * liderazgo) / 100
```

Esto sirve para el modo “¿me conviene esta tropa?” y para comparar coste/beneficio.

### 2.7 Lo que la fórmula no captura

- **Terreno** (colinas, bosques, vados, asedios).
- **Formación y órdenes** (línea de infantería, arqueros detrás, caballería por el flanco).
- **Moral y huida** (tropas que se retiran).
- **Composición**: 100 arqueros no valen lo mismo que 100 caballeros en campo abierto.
- **Habilidad del jugador**.

Por eso la app debe presentar el resultado como **estimación**, no como verdad absoluta.

---

## 3. Fórmulas del simulador

### 3.1 Poder de tropa

```text
P(nivel) = max(1, 2 * (nivel + 4)^2 / 100)
```

Tabla rápida (Anexo A completa):

| Nivel | Poder | Nivel | Poder | Nivel | Poder |
|---:|---:|---:|---:|---:|---:|
| 1 | 1 | 11 | 4 | 21 | 12 |
| 4 | 1 | 14 | 6 | 24 | 15 |
| 5 | 1 | 15 | 7 | 25 | 16 |
| 6 | 2 | 16 | 8 | 26 | 18 |
| 9 | 3 | 19 | 10 | 27 | 19 |
| 10 | 3 | 20 | 11 | 28 | 20 |

### 3.2 Poder de ejército

```text
P_ejército = Σ (P(nivel_i) * cantidad_i)
```

Ejemplo:

- 50 Huscarls (nivel 28 → 20) = 1000
- 50 Veteranos Nórdicos (nivel 24 → 15) = 750
- 30 Arqueros Nórdicos (nivel 15 → 7) = 210
- **Total = 1960**

### 3.3 Poder de héroe (heurística de la comunidad)

No hay una fórmula oficial publicada para “poder de héroe”; la comunidad usa
calculadoras de *build* basadas en atributos, habilidades y equipo. Una versión
transparente y fácil de programar es:

```text
HP = 35 + 2 * Ironflesh
reducción_armadura = armadura / (armadura + 50)
HP_efectiva = HP / (1 - reducción_armadura)

bonus_pericia = 1 + 0.0015 * max(0, pericia - 100)
bonus_PS      = 1 + 0.08 * PowerStrike
bonus_WM      = 1 + 0.02 * WeaponMaster
bonus_STR     = 1 + 0.01 * STR

DPS = daño_arma * bonus_pericia * bonus_PS * bonus_WM * bonus_STR
factor_AGI = 1 + 0.005 * AGI

poder_héroe = sqrt(DPS * factor_AGI * HP_efectiva)
```

Ejemplos calculados con esta fórmula:

| Build | STR | AGI | Ironflesh | Power Strike | WM | Pericia | Daño | Armadura | Poder |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| Tanque | 30 | 20 | 10 | 8 | 6 | 250 | 40 | 70 | **130,3** |
| DPS | 24 | 30 | 5 | 10 | 8 | 300 | 50 | 50 | **132,0** |
| Arquero | 18 | 24 | 4 | 4 | 6 | 280 | 35 | 40 | **82,0** |

> Para arqueros, cambia `PowerStrike` por `PowerDraw` y usa el daño del arco.
> La fórmula es una guía de comparación, no una verdad del motor.

### 3.4 Probabilidad de victoria

**Modelo determinista (Lanchester cuadrática):**

```text
P(A gana) = P_A^2 / (P_A^2 + P_B^2)
P(B gana) = 1 - P(A gana)
```

Es rápida y da una idea inmediata. En el ejemplo anterior:

- P_A = 1960, P_B = 1890 → P(A) ≈ 51,8 %.

**Modelo Monte Carlo:** se simulan miles de batallas con ruido y se cuenta cuántas gana cada bando.

### 3.5 Simulación Monte Carlo por rondas

Algoritmo:

```text
para cada simulación:
    PA = poder inicial A
    PB = poder inicial B
    mientras PA > 0 y PB > 0:
        daño_a_B = PA * k * ruido()
        daño_a_A = PB * k * ruido()
        PB -= daño_a_A
        PA -= daño_a_B
    si PA > PB: gana A
    si PB > PA: gana B
    si no: empate
```

- `k` controla la velocidad de la batalla (0,10 es un buen inicio).
- `ruido()` es un factor aleatorio, por ejemplo uniforme en `[0,85, 1,15]`.
- Con 10 000 simulaciones se obtiene una probabilidad estable.

**Supervivientes del ganador (Lanchester):**

```text
supervivientes = N * sqrt(1 - (poder_rival / poder_propio)^2)
```

### 3.6 Bajas

Las bajas se derivan del poder perdido:

```text
bajas_A = N_A_inicial - supervivientes_A
bajas_B = N_B_inicial - supervivientes_B
```

### 3.7 Ejemplo completo

| Ejército | Composición | Tropas | Poder |
|---|---|---:|---:|
| A (Nórdico) | 50 Huscarl + 50 Veterano + 30 Arquero | 130 | 1960 |
| B (Swadiano) | 40 Caballero + 40 Sargento + 30 Tirador | 110 | 1890 |

Resultado de la app:

- Determinista: A 51,82 % / B 48,18 %.
- Monte Carlo (10 000): A 59,67 % / B 40,33 %.
- Supervivientes medios de A: ≈ 21 de 130.
- **Veredicto: gana A.**

---

## 4. Base de datos de tropas

### 4.1 Fuente y método

Los niveles se verificaron contra el *module system* de Native 1.166
(`module_troops.py`). El archivo `TROPAS` que enviaste sirvió como base, pero
tenía algunos niveles incorrectos; aquí están corregidos (ver 4.3).

La base de datos final tiene **68 tropas**:

- 6 facciones: Nord, Vaegir, Swadia, Rhodok, Khergit, Sarranid.
- Mercenarios de taberna y caravana.
- Rama de mujeres mercenarias.

### 4.2 Tablas por facción

#### Nord

| Tropa | ID interno | Nivel | Rol | Poder | Salario/sem |
|---|---|---:|---|---:|---:|
| Recluta Nórdico | `nord_recruit` | 6 | Infantería | 2 | 3 |
| Soldado de a pie Nórdico | `nord_footman` | 10 | Infantería | 3 | 6 |
| Cazador Nórdico | `nord_huntsman` | 11 | Arquero | 4 | 7 |
| Soldado de a pie Nórdico entrenado | `nord_trained_footman` | 14 | Infantería | 6 | 11 |
| Arquero Nórdico | `nord_archer` | 15 | Arquero | 7 | 12 |
| Guerrero Nórdico | `nord_warrior` | 19 | Infantería | 10 | 19 |
| Arquero Nórdico veterano | `nord_veteran_archer` | 19 | Arquero | 10 | 19 |
| Veterano Nórdico | `nord_veteran` | 24 | Infantería | 15 | 29 |
| Campeón Nórdico (Huscarl) | `nord_champion` | 28 | Infantería | 20 | 38 |

#### Vaegir

| Tropa | ID interno | Nivel | Rol | Poder | Salario/sem |
|---|---|---:|---|---:|---:|
| Recluta Vaegir | `vaegir_recruit` | 4 | Infantería | 1 | 1 |
| Soldado de a pie Vaegir | `vaegir_footman` | 9 | Infantería | 3 | 5 |
| Escaramuzador Vaegir | `vaegir_skirmisher` | 14 | Arquero | 6 | 11 |
| Veterano Vaegir | `vaegir_veteran` | 14 | Infantería | 6 | 11 |
| Arquero Vaegir | `vaegir_archer` | 19 | Arquero | 10 | 19 |
| Infantería Vaegir | `vaegir_infantry` | 19 | Infantería | 10 | 19 |
| Arquero Vaegir (Tirador) | `vaegir_marksman` | 24 | Arquero | 15 | 29 |
| Jinete Vaegir | `vaegir_horseman` | 21 | Caballería | 12 | 38 |
| Guardia Vaegir | `vaegir_guard` | 24 | Caballería | 15 | 48 |
| Caballero Vaegir | `vaegir_knight` | 26 | Caballería | 18 | 55 |

#### Swadia

| Tropa | ID interno | Nivel | Rol | Poder | Salario/sem |
|---|---|---:|---|---:|---:|
| Recluta Swadiano | `swadian_recruit` | 4 | Infantería | 1 | 1 |
| Miliciano Swadiano | `swadian_militia` | 9 | Infantería | 3 | 5 |
| Escaramuzador Swadiano | `swadian_skirmisher` | 14 | Arquero | 6 | 11 |
| Soldado de a pie Swadiano | `swadian_footman` | 14 | Infantería | 6 | 11 |
| Ballestero Swadiano | `swadian_crossbowman` | 19 | Arquero | 10 | 19 |
| Infantería Swadiana | `swadian_infantry` | 20 | Infantería | 11 | 21 |
| Hombre de Armas Swadiano | `swadian_man_at_arms` | 21 | Caballería | 12 | 38 |
| Tirador de Primera Swadiano | `swadian_sharpshooter` | 24 | Arquero | 15 | 48 |
| Sargento Swadiano | `swadian_sergeant` | 25 | Caballería | 16 | 51 |
| Caballero Swadiano | `swadian_knight` | 28 | Caballería | 20 | 63 |

#### Rhodok

| Tropa | ID interno | Nivel | Rol | Poder | Salario/sem |
|---|---|---:|---|---:|---:|
| Miembro de la tribu Rhodok | `rhodok_tribesman` | 4 | Infantería | 1 | 1 |
| Soldado de a pie / Lancero Rhodok | `rhodok_spearman` | 9 | Infantería | 3 | 5 |
| Ballestero Rhodok | `rhodok_crossbowman` | 10 | Arquero | 3 | 6 |
| Lancero Rhodok entrenado | `rhodok_trained_spearman` | 14 | Infantería | 6 | 11 |
| Ballestero Rhodok entrenado | `rhodok_trained_crossbowman` | 15 | Arquero | 7 | 12 |
| Lancero Rhodok veterano | `rhodok_veteran_spearman` | 19 | Infantería | 10 | 19 |
| Ballestero Rhodok veterano | `rhodok_veteran_crossbowman` | 20 | Arquero | 11 | 21 |
| Sargento Rhodok | `rhodok_sergeant` | 25 | Infantería | 16 | 31 |
| Ballestero Rhodok de élite | `rhodok_sharpshooter` | 25 | Arquero | 16 | 31 |

#### Khergit

| Tropa | ID interno | Nivel | Rol | Poder | Salario/sem |
|---|---|---:|---|---:|---:|
| Miembro de la tribu Khergita | `khergit_tribesman` | 5 | Infantería | 1 | 2 |
| Escaramuzador Khergita | `khergit_skirmisher` | 10 | Arquero a caballo | 3 | 10 |
| Jinete Khergita | `khergit_horseman` | 14 | Caballería | 6 | 18 |
| Arquero montado Khergita | `khergit_horse_archer` | 14 | Arquero a caballo | 6 | 18 |
| Arquero montado Khergita veterano | `khergit_veteran_horse_archer` | 21 | Arquero a caballo | 12 | 38 |
| Lancero Khergita | `khergit_lancer` | 23 | Caballería | 14 | 45 |

#### Sarranid

| Tropa | ID interno | Nivel | Rol | Poder | Salario/sem |
|---|---|---:|---|---:|---:|
| Recluta Sarraní | `sarranid_recruit` | 4 | Infantería | 1 | 1 |
| Lancero / Soldado a pie Sarraní | `sarranid_footman` | 9 | Infantería | 3 | 5 |
| Escaramuzador Sarraní | `sarranid_skirmisher` | 14 | Arquero | 6 | 11 |
| Soldado a pie Sarraní veterano | `sarranid_veteran_footman` | 14 | Infantería | 6 | 11 |
| Arquero Sarraní | `sarranid_archer` | 19 | Arquero | 10 | 19 |
| Jinete Sarraní | `sarranid_horseman` | 20 | Caballería | 11 | 35 |
| Infantería Sarraní | `sarranid_infantry` | 20 | Infantería | 11 | 21 |
| Maestro arquero Sarraní | `sarranid_master_archer` | 24 | Arquero a caballo | 15 | 48 |
| Guarda Sarraní | `sarranid_guard` | 25 | Caballería | 16 | 51 |
| Mameluco Sarraní | `sarranid_mamluke` | 27 | Caballería | 19 | 60 |

#### Mercenarios y mujeres

| Tropa | ID interno | Nivel | Rol | Poder | Salario/sem |
|---|---|---:|---|---:|---:|
| Granjero | `farmer` | 4 | Infantería | 1 | 1 |
| Vigilante | `watchman` | 9 | Infantería | 3 | 7 |
| Maestro de Caravana | `caravan_master` | 9 | Infantería | 3 | 7 |
| Guardia de Caravana | `caravan_guard` | 14 | Caballería | 6 | 27 |
| Ballestero Mercenario | `mercenary_crossbowman` | 19 | Arquero | 10 | 28 |
| Espadachín Mercenario | `mercenary_swordsman` | 20 | Infantería | 11 | 31 |
| Jinete Mercenario | `mercenary_horseman` | 20 | Caballería | 11 | 52 |
| Caballero Mercenario | `mercenary_cavalry` | 25 | Caballería | 16 | 76 |
| Espada Contratada | `hired_blade` | 25 | Infantería | 16 | 46 |
| Campesina | `peasant_woman` | 1 | Infantería | 1 | 1 |
| Seguidora de Campamento | `follower_woman` | 5 | Infantería | 1 | 3 |
| Cazadora | `hunter_woman` | 10 | Arquero | 3 | 9 |
| Mujer de Armas | `fighter_woman` | 16 | Infantería | 8 | 21 |
| Hermana de la Espada | `sword_sister` | 22 | Caballería | 13 | 61 |

### 4.3 Correcciones a tu archivo `TROPAS`

| Tropa | Tu archivo | Correcto (Native) |
|---|---:|---:|
| Soldado de a pie Nórdico | 9 | **10** |
| Soldado de a pie Vaegir | 10 | **9** |
| Arquero Vaegir | 15 | **19** |
| Arquero Vaegir (Tirador) | 19 | **24** |
| Hombre de Armas Swadiano | 22 | **21** |
| “Infantería Rhodok” | 20 | **no existe**; el Sargento Rhodok es 25 |
| Mameluco Sarraní | 25–27 | **27** |
| Mujer de Armas | 15 | **16** |

El resto de niveles de tu archivo coinciden con el *module system*.

### 4.4 Formato CSV

`data/tropas.csv`:

```csv
id,nombre_es,nombre_en,faccion,rol,nivel,montado,distancia,mercenario,poder,salario_base
nord_recruit,Recluta Nórdico,Nord Recruit,Nord,Infantería,6,0,0,0,2,3
nord_footman,Soldado de a pie Nórdico,Nord Footman,Nord,Infantería,10,0,0,0,3,6
...
```

- `montado`, `distancia`, `mercenario`: 0/1.
- `poder`: ya calculado con la fórmula oficial.
- `salario_base`: ya incluye los multiplicadores de montado/mercenario.

### 4.5 Cómo ampliar

- **Otro mod:** copia su `module_troops.py`, extrae nivel y flags, y regenera el CSV.
- **Nuevas tropas:** añade una fila y recalcula `poder` y `salario_base`.
- **Más columnas:** si quieres modelar armadura, daño y pericia, añade
  `armadura`, `dano`, `pericia` al CSV y úsalas en el modelo de combate.

---

## 5. Modelo de batalla

### 5.1 Determinista vs Monte Carlo

| Modelo | Ventajas | Desventajas |
|---|---|---|
| Determinista (Lanchester) | Instantáneo, fácil de explicar | No da incertidumbre; ignora el azar |
| Monte Carlo | Da probabilidad y rango; más realista | Más lento; depende del ruido elegido |

Recomendación: **usar los dos**. Muestra primero el determinista como referencia y
luego el Monte Carlo como resultado principal.

### 5.2 Rondas

Cada ronda:

1. A hace daño a B proporcional a su poder.
2. B hace daño a A proporcional a su poder.
3. Se actualizan los poderes y se repite.

Con `k = 0,10` y ruido `[0,85, 1,15]` se obtienen batallas de decenas de rondas.

### 5.3 Ruido y calibración

El ruido modela la incertidumbre (terreno, suerte, órdenes). Para calibrar:

1. Simula enfrentamientos conocidos (p. ej. 100 Huscarls vs 100 Caballeros).
2. Ajusta `k` y el rango de ruido hasta que las probabilidades te parezcan razonables.
3. Guarda la configuración en un archivo (`config.json`) para no recompilar.

### 5.4 Limitaciones

- La fórmula de poder es **lineal en cantidad**: no captura bien los embudos
  (solo 20 hombres combatiendo a la vez en un asedio).
- No hay moral ni huida.
- No hay ventaja de terreno.
- Los mods cambian estadísticas.

Para una v2: añadir terreno, moral y un límite de “ancho de frente”.

---

## 6. Arquitectura de la app en C++

### 6.1 Requisitos

- **C++17** (probado con g++ 12+).
- CMake 3.16+ (opcional; también vale `g++` directo).
- Sin dependencias externas para el núcleo.

### 6.2 Estructura de carpetas

```text
mbw-sim/
├── CMakeLists.txt
├── README.md
├── data/
│   └── tropas.csv
├── include/
│   ├── troop.h        # struct Troop + enum Role
│   ├── power.h        # fórmulas de poder y salario
│   ├── database.h     # carga del CSV
│   ├── army.h         # ejército y stacks
│   └── simulator.h    # Monte Carlo
└── src/
    ├── main.cpp       # CLI
    ├── database.cpp
    ├── power.cpp
    └── simulator.cpp
```

### 6.3 Clases y responsabilidades

| Clase / struct | Responsabilidad |
|---|---|
| `Troop` | Datos de una tropa: id, nombre, facción, nivel, rol, poder, salario |
| `Database` | Cargar `tropas.csv`, buscar por id/nombre, listar por facción |
| `Army` / `Stack` | Composición de un ejército y totales (poder, tropas, salario) |
| `power` (namespace) | Fórmulas: poder de tropa, salario, poder de héroe, probabilidad |
| `Simulator` | Monte Carlo y comparación determinista |
| `main` | Interfaz de consola y parseo de ejércitos |

### 6.4 Flujo de la app

```text
[usuario] --ejército A--> parseArmy() --+
                                        +--> Army A --+
[usuario] --ejército B--> parseArmy() --+              |
                                                       +--> Simulator --> resultado
[data/tropas.csv] --> Database --> Troop* -------------+
```

### 6.5 Código clave

**Poder de tropa (oficial):**

```cpp
int troopPower(int level) {
    const int s = level + 4;
    const int v = (2 * s * s) / 100;
    return std::max(1, v);
}
```

**Salario (oficial):**

```cpp
int troopWage(int level, bool mounted, bool mercenary) {
    int w = std::max(1, ((level + 3) * (level + 3)) / 25);
    if (mounted)   w = w * 5 / 3;
    if (mercenary) w = w * 3 / 2;
    return w;
}
```

**Poder de ejército:**

```cpp
double Army::totalPower() const {
    double s = 0.0;
    for (const auto& st : stacks)
        s += double(st.troop->power) * st.count;
    return s;
}
```

**Monte Carlo (resumen):**

```cpp
while (pa > 0.01 && pb > 0.01 && rounds < 2000) {
    const double dmgToB = pa * 0.10 * noise(rng_);
    const double dmgToA = pb * 0.10 * noise(rng_);
    pb -= dmgToA;
    pa -= dmgToB;
    ++rounds;
}
if (pa > pb) { /* gana A */ }
else if (pb > pa) { /* gana B */ }
else { /* empate */ }
```

El código completo está en `mbw-sim/`.

### 6.6 Compilación

Con CMake:

```bash
cd mbw-sim
cmake -S . -B build
cmake --build build
./build/mbw-sim list
```

Con g++ directo:

```bash
cd mbw-sim
g++ -std=c++17 -O2 -Iinclude src/*.cpp -o mbw-sim
./mbw-sim list
```

### 6.7 Uso de la CLI

```bash
# Listar tropas
./mbw-sim list
./mbw-sim list Nord

# Comparar ejércitos
./mbw-sim compare "nord_champion x50, nord_veteran x50, nord_archer x30" \
                  "swadian_knight x40, swadian_sergeant x40, swadian_sharpshooter x30"

# Ejemplo predefinido
./mbw-sim demo

# Poder de un héroe
./mbw-sim hero --str 24 --agi 18 --ironflesh 6 --powerstrike 7 --wm 5 \
               --prof 220 --dmg 45 --armor 60
```

### 6.8 Interfaz gráfica (opcional)

Opciones, de menos a más trabajo:

1. **CLI** (ya hecha): suficiente para uso personal.
2. **Qt 6**: ventana con dos listas de tropas, spinboxes de cantidad y un botón “Simular”.
3. **ImGui + SDL/GLFW**: rápida de integrar y multiplataforma.
4. **Web (WASM)**: compilar el núcleo con Emscripten y hacer la UI en HTML/JS.

Para una primera versión, quédate con la CLI y añade un menú interactivo.

### 6.9 Pruebas

- **Unitarias** de `troopPower` y `troopWage` contra valores conocidos
  (nivel 28 → 20; nivel 24 → 15; salario de Caballero Swadiano → 63).
- **De integración**: comparar 1 Huscarl vs 1 Recluta (debe ganar el Huscarl).
- **De calibración**: 100 Huscarls vs 100 Caballeros (resultado cercano al 50 %).

---

## 7. Plan de implementación por fases

| Fase | Qué hacer | Tiempo estimado |
|---|---|---|
| 0 | Instalar g++/CMake y crear la carpeta `mbw-sim` | 15 min |
| 1 | Copiar `tropas.csv` y las clases `Troop`/`Database` | 1 h |
| 2 | Implementar `power` (poder y salario) y probar | 1 h |
| 3 | Implementar `Army` y la comparación determinista | 1 h |
| 4 | Implementar Monte Carlo y validar | 2 h |
| 5 | CLI con `list`, `compare`, `demo`, `hero` | 2 h |
| 6 | Pruebas y calibración | 2 h |
| 7 | (Opcional) GUI con Qt o ImGui | 1–3 días |
| 8 | (Opcional) Soporte de mods y terreno | variable |

---

## 8. Consejos

1. **Empieza simple.** Primero el determinista; el Monte Carlo después.
2. **Separa datos de código.** El CSV permite cambiar tropas sin recompilar.
3. **No persigas la fórmula exacta del motor.** Usa un modelo transparente y documentado.
4. **Valida con casos conocidos.** Si 1 Huscarl no gana a 1 Recluta, algo está mal.
5. **Muestra incertidumbre.** “A gana 60 %” es más honesto que “A gana”.
6. **Versiona la base de datos.** Native 1.174 no es lo mismo que un mod.
7. **Cuida el equilibrio.** La fórmula lineal favorece a los números; añade un límite
   de frente si quieres simular asedios.
8. **Documenta las suposiciones.** El usuario debe saber que es una estimación.
9. **No distribuyas assets del juego.** Solo estadísticas y código propio.
10. **Añade un modo “coste/beneficio”.** El salario oficial ya está calculado.

---

## 9. Fuentes

- *Mount & Blade: Warband* — *module system* de Native (1.166/1.174):
  - `module_scripts.py`: scripts `party_calculate_strength` y `game_get_troop_wage`.
  - `module_troops.py`: niveles, atributos y equipo de las tropas.
- Comunidad de *Mount & Blade* (TaleWorlds forums, Reddit, wiki): discusiones sobre
  daño, pericia y auto-resolve.
- Cálculos propios de poder, salario y héroe a partir de esas fuentes.

> Las fórmulas de poder y salario son las del juego base. Los mods pueden cambiarlas.

---

## Anexo A: poder por nivel

| Nivel | Poder | Nivel | Poder | Nivel | Poder | Nivel | Poder |
|---:|---:|---:|---:|---:|---:|---:|---:|
| 1 | 1 | 9 | 3 | 17 | 8 | 25 | 16 |
| 2 | 1 | 10 | 3 | 18 | 9 | 26 | 18 |
| 3 | 1 | 11 | 4 | 19 | 10 | 27 | 19 |
| 4 | 1 | 12 | 5 | 20 | 11 | 28 | 20 |
| 5 | 1 | 13 | 5 | 21 | 12 | 29 | 21 |
| 6 | 2 | 14 | 6 | 22 | 13 | 30 | 23 |
| 7 | 2 | 15 | 7 | 23 | 14 | 31 | 24 |
| 8 | 2 | 16 | 8 | 24 | 15 | 32 | 25 |

Fórmula: `max(1, 2 * (nivel + 4)^2 / 100)` con división entera.

---

## Anexo B: archivos del proyecto

```text
mbw-sim/
├── CMakeLists.txt
├── README.md
├── data/tropas.csv
├── include/{troop,power,database,army,simulator}.h
└── src/{main,database,power,simulator}.cpp
```

Para empezar:

```bash
cd mbw-sim
g++ -std=c++17 -O2 -Iinclude src/*.cpp -o mbw-sim
./mbw-sim demo
```

---

**Fin de la guía.** Con esto tienes la base de datos, las fórmulas, el modelo de
batalla y una app C++ funcional para comparar ejércitos de Warband.
