#include <iostream>
#include <cassert>
#include <thread>
#include <mutex>
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include "Semaphore.h"
#include "HoareMonitor.h"

using namespace std ;
using namespace HM ;
using namespace SEM;

const int num_clientes = 3;

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

//************************************************************************
//  Monitor
//************************************************************************

class Barberia: public HoareMonitor{
    private:
        CondVar barbero, cliente, salaDeEspera;
    public:
        Barberia(){
            barbero = newCondVar();
            cliente= newCondVar();
            salaDeEspera= newCondVar();
        }

        void cortarPelo(int cl){
            if(!barbero.empty() && cliente.empty()){
                cout << "Cliente " << cl << ": inicia su corte de pelo" << endl;
                barbero.signal();
                salaDeEspera.wait();
            }
            else{
                cliente.wait();
                salaDeEspera.wait();
            }
        }

        void siguienteCliente(){
            if(!cliente.empty())
                cliente.signal();
            else
                barbero.wait();
        }

        void finCliente(){
            salaDeEspera.signal();
        }
};

void EsperarFueraBarberia(int cl){
    chrono::milliseconds tiempo(aleatorio<20,200>());
    cout << "Cliente " << cl << ": espera fuera durante " << tiempo.count() << " milisegundos" << endl;
    this_thread::sleep_for(tiempo);
    cout << "Cliente " << cl << " : acaba la espera fuera" << endl;
}

void cortarPeloACliente(){
    chrono::milliseconds tiempo(aleatorio<20,200>());
    cout << "Barbero corta durante" << tiempo.count() << "milisegundos" << endl; 
    this_thread::sleep_for(tiempo);
    cout << "Barbero: termina de cortar" << endl;
}

void funcion_hebra_barbero(MRef<Barberia> monitor){
    while(true){
        monitor->siguienteCliente();
        cortarPeloACliente();
        monitor->finCliente();
    }
}

void funcion_hebra_cliente(MRef<Barberia> monitor, int cl){
    while(true){
        EsperarFueraBarberia(cl);
        monitor->cortarPelo(cl);
    }
}

int main(){
    MRef<Barberia> monitorBarberia= Create<Barberia>();
    thread hebra_cliente[num_clientes];
    thread hebra_barbero(funcion_hebra_barbero, monitorBarberia);

    for(int i = 0; i < num_clientes; i++)
        hebra_cliente[i]=thread(funcion_hebra_cliente, monitorBarberia, i);

    hebra_barbero.join();
    for(int i= 0; i < num_clientes; i++)
        hebra_cliente[i].join();

}