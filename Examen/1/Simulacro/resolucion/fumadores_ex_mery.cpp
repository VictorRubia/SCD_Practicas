#include <iostream>
#include <cassert>
#include <thread>
#include <mutex>
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include "Semaphore.h"

using namespace std ;
using namespace SEM ;

const int num_fumadores = 3;
const int num_suministradores = 3;
const int capacidad_buffer = 10;
Semaphore mostrador=1; //1 si no se atiende a nadie, 0 si está ocupado con algún fumador
std::vector<Semaphore> ingredientes; // 1 si ingrediente i está disponible, 0 si no. Inicializado a 0 para solo poder entrar en la función estanquero.
vector<Semaphore> suministradores;

int buffer[capacidad_buffer];
int num_items_buffer = 0;
bool mostrador_ocupado = false;

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

//-------------------------------------------------------------------------
// Función que simula la acción de producir un ingrediente, como un retardo
// aleatorio de la hebra (devuelve número de ingrediente producido)

int producir_ingrediente(int num_suministrador)
{
   // calcular milisegundos aleatorios de duración de la acción de fumar)
   chrono::milliseconds duracion_produ( aleatorio<10,100>() );

   // informa de que comienza a producir
   mtx.lock();
   cout << "Suministrador " << num_suministrador << " : empieza a producir ingrediente (" << duracion_produ.count() << " milisegundos)" << endl;
   mtx.unlock();

   // espera bloqueada un tiempo igual a ''duracion_produ' milisegundos
   this_thread::sleep_for( duracion_produ );

   const int num_ingrediente = aleatorio<0,num_fumadores-1>() ;

   // informa de que ha terminado de producir
   mtx.lock();
   cout << "Suministrador " << num_suministrador << " : termina de producir ingrediente " << num_ingrediente << endl;
   mtx.unlock();

   return num_ingrediente ;
}

void funcion_hebra_suministradora(int num_suministrador){
   int num_fumador;
   while(true){
      num_fumador = producir_ingrediente(num_suministrador);
      if(num_items_buffer > 9){
         mtx.lock();
         cout << "Suministrador " << num_suministrador << " se espera." << endl;
         mtx.unlock();
         sem_wait(suministradores[num_suministrador]);
      }
      if(num_items_buffer <= 9 && num_items_buffer >= 0){
         num_items_buffer++;
         buffer[num_items_buffer]=num_fumador;
         sem_signal(suministradores[num_suministrador]);
         if(!mostrador_ocupado){
            sem_signal(mostrador);
         }
      }
   }
}

//----------------------------------------------------------------------
// función que ejecuta la hebra del estanquero

void funcion_hebra_estanquero(  )
{
   int num_fumador;
   while(true){
      if(num_items_buffer == 0){
         sem_wait(mostrador);
         sem_signal(suministradores[0]);
         sem_signal(suministradores[1]);
         sem_signal(suministradores[2]);
      }
      else if(num_items_buffer > 0 && !mostrador_ocupado){
         num_fumador = buffer[num_items_buffer];
         num_items_buffer--;
         mostrador_ocupado = true;
         
         mtx.lock();
         cout << "Estanquero pone el ingrediente número: " << num_fumador << endl;
         mtx.unlock();
         
         sem_signal(ingredientes[num_fumador]);

         if(num_items_buffer > 9){
            sem_signal(suministradores[0]);
            sem_signal(suministradores[1]);
            sem_signal(suministradores[2]);
         }

      }
   }

}

//-------------------------------------------------------------------------
// Función que simula la acción de fumar, como un retardo aleatoria de la hebra

void fumar( int num_fumador )
{

   // calcular milisegundos aleatorios de duración de la acción de fumar)
   chrono::milliseconds duracion_fumar( aleatorio<20,200>() );

   // informa de que comienza a fumar
   mtx.lock();
    cout << "Fumador " << num_fumador << "  :"
          << " empieza a fumar (" << duracion_fumar.count() << " milisegundos)" << endl;
   mtx.unlock();
   // espera bloqueada un tiempo igual a ''duracion_fumar' milisegundos
   this_thread::sleep_for( duracion_fumar );

   // informa de que ha terminado de fumar
   mtx.lock();
    cout << "Fumador " << num_fumador << "  : termina de fumar, comienza espera de ingrediente." << endl;
   mtx.unlock();
}

//----------------------------------------------------------------------
// función que ejecuta la hebra del fumador
void  funcion_hebra_fumador( int num_fumador )
{
   while( true )
   {
      sem_wait(ingredientes[num_fumador]);
      
      mtx.lock();
      cout << "Se retira el ingrediente número: " << num_fumador << endl;
      mtx.unlock();

      mostrador_ocupado = false;

      sem_signal(mostrador);
      fumar(num_fumador);

   }
}

//----------------------------------------------------------------------

int main()
{
   // declarar hebras y ponerlas en marcha
   // ......

   for(int i = 0; i < num_fumadores; i++)
      ingredientes.push_back(0);
   for(int i = 0; i < num_suministradores; i++)
      suministradores.push_back(0);
   
   thread hebra_estanquero(funcion_hebra_estanquero);
   thread hebra_fumador[num_fumadores];
   thread hebra_suministradora[num_suministradores];

   for(unsigned long i = 0; i < num_suministradores; i++)
      hebra_suministradora[i] = thread(funcion_hebra_suministradora, i);

   for(unsigned long i = 0; i < num_fumadores; i++)
      hebra_fumador[i] = thread(funcion_hebra_fumador, i);
   hebra_estanquero.join();
   

   for(unsigned long i = 0; i < num_fumadores; i++)
      hebra_fumador[i].join();
   for(unsigned long i = 0; i < num_suministradores; i++)
      hebra_suministradora[i].join();
   

}
