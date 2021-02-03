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

const int num_pacientes = 14;
const int num_higienistas = 4;
const int num_dentistas = 2;
const int num_profesional = num_higienistas+num_dentistas;

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

void profesionales_espera(int TC){
    chrono::milliseconds dur_escritura(aleatorio<20,200>());

    //  Espera bloqueada de dur_escritura milisegundos
    if(TC == 1){ // Si es empaste
        this_thread::sleep_for(dur_escritura*2);  
        mtx.lock();
        cout << "Empastando.. " << endl;
        mtx.unlock();
    }else
    {
        this_thread::sleep_for(dur_escritura); 
        mtx.lock();
        cout << "Limpiando boca.. " << endl;
        mtx.unlock();
    }
    
}

void Espera(){
    chrono::milliseconds tiempo(aleatorio<20,200>());
    this_thread::sleep_for(tiempo);
}

class Clinica: public HoareMonitor{
    private:
        int num_dent, num_hig, num_pac;

        CondVar pacientes, profesionales;

    public:
        Clinica(){
            num_dent = 2;
            num_hig = 4;
            num_pac = 0;
            pacientes = newCondVar();
            profesionales = newCondVar();
        }
        void acceso_consulta(int TC){
            num_pac++;
            if(TC == 0){ //  Si el motivo es limpieza
                cout << "Entra paciente con limpieza" << endl;
                if(num_hig == 0){
                    pacientes.wait(); // Si estan ocupados se espera
                }
                else{
                    num_hig--;
                    profesionales.signal(); // Se lanza una hebra profesional y se atiende al paciente
                }
            }
            else if(TC == 1){   // Si el motivo es empaste
                cout << "Entra paciente con empaste" << endl;
                if(num_dent == 0) //mismos comentarios que arriba
                    pacientes.wait();
                else
                {
                    num_dent--;
                    profesionales.signal();
                }
            }
        }
        void fin_consulta(int TC){
            num_pac--;
            if(TC == 0){
                num_hig++;  // Se libera un higienista
                mtx.lock();
                cout << "Se ha terminado de limpiar la boca" << endl;
                mtx.unlock();
                pacientes.signal();
            }
            else if(TC == 1){
                num_dent++;
                mtx.lock();
                cout << "Se ha terminado de empastar" << endl;
                mtx.unlock();
                pacientes.signal();
            }
            profesionales.signal();
        }
};

void funcion_hebra_pacientes(MRef<Clinica> monitor, int numPaciente){
    int motivo;
    while(true){
        motivo = aleatorio<0,1>();
        monitor->acceso_consulta(motivo);
        profesionales_espera(motivo);
        monitor->fin_consulta(motivo);
        Espera();
    }
}


int main(){
    cout << "-------------------------------------------------------" << endl <<
            "- Problema de la clinica. Monitor SU. --" << endl <<
            "-------------------------------------------------------" << endl << flush;
    MRef<Clinica> monitor = Create<Clinica>();

    thread hebras_paciente[num_pacientes], hebras_profesional[num_profesional];

    for(int i = 0; i < num_pacientes; i++)
        hebras_paciente[i] = thread(funcion_hebra_pacientes, monitor, i);

    for(int i = 0; i < num_pacientes; i++)
        hebras_paciente[i].join();


    
 }
