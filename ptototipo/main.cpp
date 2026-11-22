#include<iostream>
#include "base_de_datos.h"
#include<vector>
#include <thread>           //Animacion para
#include <chrono>           // el cargando
#include <cstdlib> 
#include<limits>
#include<string>
#include<map>
#include <sstream>   //convertir string en numeros

using namespace std;
using map_t=map<int,int>;
using pair_t=pair<int,int>;

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
   
    cout<<"Bienvenido al fucking programa para ver cual es el mejor ejercito"<<endl;
    cout<<"Opcion 1:Leer los 2 ejercitos"<<endl;
    cout<<"Opcion 2:Ver codigo de tropas"<<endl;
    cout<<"Opcion 3:Comparar los ejercitos"<<endl;
    cout<<"Opcion 4:Salir"<<endl;
}
void lista_tropas(){
    animacionCarga("Contando tropas");
    cout<<"Tropas Disponibles :\n"<<endl;
    for(size_t i=0;i<base_de_datos.size();i++){
        cout<<i<<"--"<<base_de_datos[i].nombre<<"\n";
    }
}
void detectar_tropas(){

}
//Coge el string lo convierte en su numero y lo guarda en el mapa
map_t string_to_int(string ejercito){
    //convertir string en numeros
    map_t ejer;

    stringstream ss(ejercito);
    int id_tropa;
    int cantidad;
    while (ss >> id_tropa >> cantidad) {
        // ¡Lo guardamos en el mapa!
        ejer[id_tropa] += cantidad; 
        }
    
    return ejer;
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
            cout << "Ingresa las tropas separadas por '+' y termina con ';'" << endl;
            cout << "El primer numero es el id de la tropa y el segundo la cantidad" << endl;
            cout << "Ejemplo: 0 5 + 1 6 + 2 7;" << endl;
            
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
            lista_tropas();
            break;
        case 3:
            
        default: 
            if(aux != 4) {
                cout<<"Numero incorrecto, por favor escriba un numero correcto"<<endl;
            }
            break;
        }
    } while (aux!=4);
    



    return 0;
}