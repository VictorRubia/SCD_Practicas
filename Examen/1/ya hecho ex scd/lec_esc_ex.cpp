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

const int num_lectores = 5;
const int num_escritores = 7;

mutex mtx;

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

void escribir(int escritor){
    chrono::milliseconds dur_escritura(aleatorio<20,200>());

    //  Se empieza a escribir
    mtx.lock();
    cout << "El escritor " << escritor << " comienza a escribir ( " << dur_escritura.count() << " milisegundos)" << endl;
    mtx.unlock();

    //  Espera bloqueada de dur_escritura milisegundos
    this_thread::sleep_for(dur_escritura);

    //  Informa que ha terminado de escribir
    mtx.lock();
    cout << "El escritor " << escritor << " ha terminado de escribir" << endl; 
    mtx.unlock();

}

void Espera(){
    chrono::milliseconds tiempo(aleatorio<20,200>());
    this_thread::sleep_for(tiempo);
}

void leer(int lector){
    chrono::milliseconds dur_lectura(aleatorio<20,200>());

    //  Se empieza a leer
    mtx.lock();
    cout << "El lector " << lector << " comienza a leer ( " << dur_lectura.count() << " milisegundos)" << endl;
    mtx.unlock();

    //  Espera bloqueada de dur_lectura milisegundos
    this_thread::sleep_for(dur_lectura);

    //  Informa que ha terminado de leer
    mtx.lock();
    cout << "El lector " << lector << " ha terminado de leer" << endl; 
    mtx.unlock();

}

class LectoresEscritores: public HoareMonitor{
    private:
        int num_lec, num_esc;
        bool escribiendo, afumigar;

        CondVar lectura, escritura, fumigadora;

    public:
        LectoresEscritores(){
            num_lec = 0;
            num_esc = 0;
            escribiendo = false;
            afumigar = false;
            lectura = newCondVar();
            escritura = newCondVar();
            fumigadora = newCondVar();
        }
        void iniLec(){
            if(escribiendo || num_esc == 0 )
                lectura.wait();
            num_lec++;
            num_esc = 0;
            lectura.signal();
        }
        void finLec(){
            num_lec--;
            if(num_lec == 0)
                escritura.signal();
        }
        void iniEsc(){
            if(num_lec > 0 || escribiendo)
                escritura.wait();
            escribiendo = true;
        }
        void finEsc(){
            num_esc++;
            escribiendo = false;

            if(num_esc == 4){
                afumigar = true;
                fumigadora.signal();
                cout << "ESCRITOR " << escritura.get_nwt() << " LANZA FUMIGADORA" <<endl;
                num_esc = 0;
            }

            if(!lectura.empty())  
                lectura.signal();
            // else
                escritura.signal();            
        }
        void fumigar(){
            if(afumigar){
            afumigar = false;
            cout << "FUMIGANDO..." << endl;
            chrono::milliseconds dur_lectura(aleatorio<20,200>());
            this_thread::sleep_for(dur_lectura);
            }
            else{
                fumigadora.wait();
            }
        }
};

void funcion_hebra_fumigadora(MRef<LectoresEscritores> monitor){
    while(true){
        Espera();
        monitor->fumigar();
        cout << "Se ha terminado de fumigar" << endl;
    }
}
void funcion_hebra_lector(MRef<LectoresEscritores> monitor, int numLectores){
    while(true){
        Espera();
        monitor->iniLec();
        leer(numLectores);
        monitor->finLec();
    }
}

void funcion_hebra_escritor(MRef<LectoresEscritores> monitor, int numEscritores){
    while(true){
        Espera();
        monitor->iniEsc();
        escribir(numEscritores);
        monitor->finEsc();
    }
}


int main(){
    cout << "-------------------------------------------------------" << endl <<
            "- Problema de los lectores y escritores con fumigacion. Monitor SU. --" << endl <<
            "-------------------------------------------------------" << endl << flush;
    MRef<LectoresEscritores> monitor = Create<LectoresEscritores>();

    thread hebras_lectoras[num_lectores], hebras_escritoras[num_escritores];
    thread hebra_fumigadora(funcion_hebra_fumigadora, monitor);

    for(int i = 0; i < num_lectores; i++)
        hebras_lectoras[i] = thread(funcion_hebra_lector, monitor, i);

    for(int i = 0; i < num_escritores; i++)
        hebras_escritoras[i] = thread(funcion_hebra_escritor, monitor, i);
    
    
    for(int i = 0; i < num_lectores; i++)
        hebras_lectoras[i].join();

    for(int i = 0; i < num_escritores; i++)
        hebras_escritoras[i].join();
    hebra_fumigadora.join();
    
 }
