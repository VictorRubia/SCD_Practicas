#include <iostream>
#include <iomanip>
#include <cassert>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono> // duraciones (duration), unidades de tiempo
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include "HoareMonitor.h"

using namespace std;
using namespace HM;

const int num_coches = 5;

//**********************************************************************
// plantilla de función para generar un entero aleatorio uniformemente
// distribuido entre dos valores enteros, ambos incluidos
// (ambos tienen que ser dos constantes, conocidas en tiempo de compilación)
//----------------------------------------------------------------------

template< int min, int max > int aleatorio()
{
  static default_random_engine generador( (random_device())() );
  static uniform_int_distribution<int> distribucion_uniforme( min, max ) ;
  return distribucion_uniforme( generador );
}

void Espera(){
    chrono::milliseconds tiempo(aleatorio<20,200>());
    this_thread::sleep_for(tiempo);
}

void pagar_peaje(int coche, int n_cabina){
    chrono::milliseconds dur_lectura(aleatorio<20,200>());

    //  Se empieza a leer
    cout << "El coche " << coche << " paga el peaje en la cabina " << n_cabina << " ( " << dur_lectura.count() << " milisegundos)" << endl;

    //  Espera bloqueada de dur_lectura milisegundos
    this_thread::sleep_for(dur_lectura);

    //  Informa que ha terminado de leer
    cout << "El coche " << coche << " ha terminado de pagar" << endl; 

}

class Peaje: public HoareMonitor{
    private:
        int num_coches_c1;
        int num_coches_c2;

        bool coche_c1;
        bool coche_c2;

        CondVar pagando_c1,
                pagando_c2;

    public:
        Peaje(){
            num_coches_c1 = 0;
            num_coches_c2 = 0;
            coche_c1 = false;
            coche_c2 = false;
            pagando_c1 = newCondVar();
            pagando_c2 = newCondVar();
        }
        int llegada_peaje(){
            int x;
            if(!coche_c2)
                pagando_c2.wait();
            if(num_coches_c1 < num_coches_c2){
                x = 1;
                num_coches_c1++;
                if(!coche_c1)
                    pagando_c1.wait();
                coche_c1 = true;
                pagando_c1.signal();
            }
            else{
                x = 2;
                num_coches_c2++;
                if(!coche_c2)
                    pagando_c2.wait();
                coche_c2 = true;
                pagando_c2.signal();
            }
            return x;
        }
        void pagado(int cab){
            if(cab == 1)
                num_coches_c1--;
                coche_c1 = false;
            if(cab == 2)
                num_coches_c2--;
                coche_c2 = false;
        }
};

void funcion_hebra_coche(MRef<Peaje> monitor, int numCoches){
    int cabina;
    while(true){
        cabina = monitor->llegada_peaje();
        Espera();
        pagar_peaje(numCoches, cabina);
        monitor->pagado(cabina);
        Espera();
    }
}

int main(){
    cout << "-------------------------------------------------------" << endl <<
            "-                   Ejercicio 3 examen               --" << endl <<
            "-------------------------------------------------------" << endl << flush;
    MRef<Peaje> monitor = Create<Peaje>();

    thread hebras_coches[num_coches];

    for(int i = 0; i < num_coches; i++)
        hebras_coches[i] = thread(funcion_hebra_coche, monitor, i);
    
    for(int i = 0; i < num_coches; i++)
        hebras_coches[i].join();
    
 }