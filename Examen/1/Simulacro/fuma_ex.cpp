#include <iostream>
#include <cassert>
#include <thread>
#include <mutex>
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include "Semaphore.h"

using namespace std ;
using namespace SEM ;

const int num_fumadores = 4;
Semaphore mostrador=1; //1 si no se atiende a nadie, 0 si está ocupado con algún fumador
std::vector<Semaphore> ingredientes; // 1 si ingrediente i está disponible, 0 si no. Inicializado a 0 para solo poder entrar en la función estanquero.

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

int producir_ingrediente()
{
   // calcular milisegundos aleatorios de duración de la acción de fumar)
   chrono::milliseconds duracion_produ( aleatorio<10,100>() );

   // informa de que comienza a producir
   cout << "Estanquero : empieza a producir ingrediente (" << duracion_produ.count() << " milisegundos)" << endl;

   // espera bloqueada un tiempo igual a ''duracion_produ' milisegundos
   this_thread::sleep_for( duracion_produ );

   const int num_ingrediente = aleatorio<0,num_fumadores-1>() ;

   // informa de que ha terminado de producir
   cout << "Estanquero : termina de producir ingrediente " << num_ingrediente << endl;

   return num_ingrediente ;
}

//----------------------------------------------------------------------
// función que ejecuta la hebra del estanquero

void funcion_hebra_estanquero(  )
{
   int num_fumador;
   while(true){
      sem_wait(mostrador);
      num_fumador=producir_ingrediente();   
      <vector> aux;

      cout << "Se ha puesto el ingrediente número: " << num_fumador << endl;

      for(i = 0; i < num_fumadores; i++){
        if(igredientes[i] == num_fumador)
            aux.push_back(i);
      }
      
      sem_signal(ingredientes[aleatorio<0,aux.size()-1>()]);
   }

}

//-------------------------------------------------------------------------
// Función que simula la acción de fumar, como un retardo aleatoria de la hebra

void fumar( int num_fumador )
{

   // calcular milisegundos aleatorios de duración de la acción de fumar)
   chrono::milliseconds duracion_fumar( aleatorio<20,200>() );

   // informa de que comienza a fumar
   
    cout << "Fumador " << num_fumador << "  :"
          << " empieza a fumar (" << duracion_fumar.count() << " milisegundos)" << endl;

   // espera bloqueada un tiempo igual a ''duracion_fumar' milisegundos
   this_thread::sleep_for( duracion_fumar );

   // informa de que ha terminado de fumar
   
    cout << "Fumador " << num_fumador << "  : termina de fumar, comienza espera de ingrediente." << endl;

}

//----------------------------------------------------------------------
// función que ejecuta la hebra del fumador
void  funcion_hebra_fumador( int num_fumador )
{
   while( true )
   {
      sem_wait(ingredientes[num_fumador]);
      
      
      cout << "Se retira el ingrediente número: " << num_fumador << endl;


      sem_signal(mostrador);
      fumar(num_fumador);

   }
}

//----------------------------------------------------------------------

int main()
{
   // declarar hebras y ponerlas en marcha
   // ......

   for(int i = 0; i < num_fumadores-1; i++)
      ingredientes.push_back(0);
    ingredientes.push_back(2)

   thread hebra_estanquero(funcion_hebra_estanquero);
   thread hebra_fumador[num_fumadores];

   for(unsigned long i = 0; i < num_fumadores; i++)
      hebra_fumador[i] = thread(funcion_hebra_fumador, i);
   hebra_estanquero.join();

   for(unsigned long i = 0; i < num_fumadores; i++)
      hebra_fumador[i].join();
   

}
