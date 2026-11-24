#include<iostream>
#include "base_de_datos.h"
#include<vector>
#include <thread>           //Animacion para
#include <chrono>           // el cargando
#include <cstdlib> 
#include<limits>
#include<string>
#include<map>
#include<cmath>
#include <algorithm>
#include <sstream>   //convertir string en numeros

using namespace std;
using map_t=map<int,int>;
using pair_t=pair<int,int>;

void ejemplo(){
    cout << "Ingresa las tropas separadas por '+'"<< endl;
    cout << "El primer numero es el id de la tropa y el segundo la cantidad" << endl;
    cout << "Ejemplo: 0 5 + 27 30 + 35 87;" << endl;
    cout<<"Ejercito:"<<endl;
    cout<<"-5 Recluta Nórdico"<<endl;
    cout<<"--Poder por unidad: 2"<<endl;
    cout<<"--Nivel de la unidad: 6"<<endl;
    cout<<"--Salario de la unidad: 3"<<endl;
    cout<<"-30 Sargento Swadiano"<<endl;
    cout<<"--Poder por unidad: 16"<<endl;
    cout<<"--Nivel de la unidad: 25"<<endl;
    cout<<"--Salario de la unidad: 51"<<endl;
    cout<<"-87 Ballestero Rhodok veterano"<<endl;
    cout<<"--Poder por unidad: 11"<<endl;
    cout<<"--Nivel de la unidad: 20"<<endl;
    cout<<"--Salario de la unidad: 21"<<endl;
    cout<<"Poder Total del Ejercito: 1447"<<endl;
    cout<<"Costo total del Ejercito: 3372 denares"<<endl;
}

void pausar() {
    cout << "\nPresione ENTER para continuar...";
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    cin.get();
}

void animacionCarga(const string& mensaje, int ms = 1500) {
    cout << mensaje;
    int ciclos = ms / 300;
    for (int i = 0; i < ciclos; i++) {
        int puntos = i % 3 + 1;
        cout << "\r" << mensaje;
        for (int j = 0; j < puntos; j++) cout << ".";
        for (int j = puntos; j < 3; j++)  cout << " ";
        cout << flush;
        this_thread::sleep_for(chrono::milliseconds(500));
    }
    cout << "\r" << mensaje << "... ✔\n";
}

void menu(){ 
    cout << "\n";
    cout << "  ╔══════════════════════════════════════════════════╗\n";
    cout << "  ║      ⚔  COMPARADOR DE EJERCITOS  ⚔               ║\n";
    cout << "  ╚══════════════════════════════════════════════════╝\n";
    cout << "\n";
    cout << "   [1]" << " Leer los 2 ejercitos\n";
    cout << "   [2]" << " Ver catalogo de tropas\n";
    cout << "   [3]" << " Comparar los ejercitos\n";
    cout << "   [4]" << " Salir\n";
    cout << "\n"<< "   > ";
}
void lista_tropas(){
    animacionCarga("Contando tropas");
    cout<<"Tropas Disponibles :\n"<<endl;
    for(size_t i=0;i<base_de_datos.size();i++){
        cout<<i<<"--"<<base_de_datos[i].nombre<<"\n";
    }
}

//Coge el string lo convierte en su numero y lo guarda en el mapa
map_t string_to_int(const string& ejercito){
    map_t ejer;
    string limpio=ejercito;
    //Sustituye el + por un espacio para q el stringstream no tenga error
    for (char &c : limpio){
        if (c=='+') c=' ';
        
    }
    

    stringstream ss(limpio);
    int id_tropa;
    int cantidad;
    while (ss >> id_tropa >> cantidad) {
        if (id_tropa<0 || id_tropa>=base_de_datos.size()){
            cout<<"ID invalido"<<id_tropa<<endl;
            continue;
        }
        if (cantidad<=0){
            cout<<"Cantidad invalida para ID"<<cantidad<<endl;
            continue;
        }
        // ¡Lo guardamos en el mapa!
        ejer[id_tropa] += cantidad; 
    }
    
    return ejer;
}


void eic_tropas(const map_t &x){  //Extraer Imprimir Caluclar
    int cantidad;
    int id;
    int total_poder=0;
    int tmp_poder;
    int total_salario=0;
    int tmp_salario;
    int total_soldados=0;
    for(const auto& par : x){
        tmp_poder=0;
        tmp_salario=0;
        //Saco datos del mapa
        id=par.first;
        cantidad=par.second;
        //Imprime el ejercito
        cout<<-cantidad<<" "<<base_de_datos[id].nombre<<endl;
        //Calcula el poder total del ejercito
        const int s=base_de_datos[id].nivel +4;
        const int v=(2*s*s) /100;
        tmp_poder=max(1,v);
        //Imprime su poder
        cout<<"--Poder por unidad: "<<tmp_poder<<endl;
        cout<<"--Nivel de la unidad: "<<base_de_datos[id].nivel<<endl;
        //Calcula el salario
        int w=((s-1)*(s-1))/25;
        tmp_salario=max(1,w);
        if (base_de_datos[id].rol=="Caballería") tmp_salario=tmp_salario*5/3;
        //Imprime el salario
        cout<<"--Salario de la unidad: "<<tmp_salario<<endl;
        //Calcual el total del salario y del poder
        total_poder+=tmp_poder*cantidad;
        total_salario+=tmp_salario*cantidad;
        total_soldados+=cantidad;
    }
    cout<< "  ─────────────────────────────────────\n";
    cout<< "  Unidades totales: "<< total_soldados << "\n";
    cout<< "  Poder total:      "<< total_poder<< "\n";
    cout<< "  Costo total:      "<< total_salario<< " denares"<< "\n";
}
int main(){
    int aux=0;
    string ejercito1;
    string ejercito2;
    map_t ejer1;
    map_t ejer2;
    

    animacionCarga("Cargando el menu");
    do{ 
        menu();
        //Analiza entradas invalidas
        if (!(cin >> aux)) {
            //Limpia el estado de error de cin
            cin.clear();
            //Bota toda la basura del buffer hasta el Enter.
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            //Avisa que la opción es inválida.
            cout <<"  Opcion invalida. Mete un numero.\n";
            //Sáltate el resto del bucle y vuelve a mostrar el menú.”
            continue;
            }
        switch(aux){
        case 1:
            system("clear");
            ejemplo();
            pausar();
            system("clear");
            // --- EJERCITO 1 ---
            cout << "\nDiga el primer ejercito: ";
            getline(cin, ejercito1); 
            ejer1 = string_to_int(ejercito1);
            // --- EJERCITO 2 ---
            cout << "Diga el segundo ejercito: ";
            getline(cin, ejercito2); 
            ejer2 = string_to_int(ejercito2);
            break;
        case 2:
            system("clear");
            lista_tropas();
            break;
        case 3:
            system("clear");
            eic_tropas(ejer1);
            eic_tropas(ejer2);
            break;
        default: 
            if(aux != 4) {
                cout<<"Numero incorrecto, por favor escriba un numero correcto"<<endl;
                pausar();
            }
            break;
        }
    } while (aux!=4);
    



    return 0;
}