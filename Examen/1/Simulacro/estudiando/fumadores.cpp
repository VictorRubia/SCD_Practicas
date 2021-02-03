#include <iostream>
#include <cassert>
#include <thread>
#include <mutex>
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include "Semaphore.h"

using namespace std ;
using namespace SEM ;

Semaphore fum_disponibles[4] = {0,0,0,0}; // El cuarto fumador necesita lo mismo q el tercero
Semaphore mostrador=1; //1 si no se atiende a nadie, 0 si está ocupado con algún fumador
int ultima_vez; //   Saber cual es el fumador que fumó la última vez
bool primera_vez = true;
Semaphore mensajes = 1;

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
   int producido = aleatorio <0, 2>();

   return producido;
}

//----------------------------------------------------------------------
// función que ejecuta la hebra del estanquero

void funcion_hebra_estanquero(  )
{
   while(true){
      int num_fumador = producir_ingrediente();
      sem_wait(mostrador);
      sem_wait(mensajes);
      cout << "El estanquero ha puesto el ingrediente " << num_fumador << " en el mostrador" << endl;
      sem_signal(mensajes);

      if (num_fumador == 2){
         if(primera_vez){
            num_fumador = aleatorio <2,3>();
            primera_vez = false;
         }
         else
         {
            if (ultima_vez == 2)
               num_fumador = 3;
            else
            {
               num_fumador = 2;
            }
         }
         ultima_vez = num_fumador;
      }
      
      sem_signal(fum_disponibles[num_fumador]);
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
   sem_wait(mensajes);
   cout << "Fumador " << num_fumador << "  : termina de fumar, comienza espera de ingrediente." << endl;
   sem_signal(mensajes);

}

//----------------------------------------------------------------------
// función que ejecuta la hebra del fumador
void  funcion_hebra_fumador( int num_fumador )
{
   while( true )
   {
      sem_wait(mensajes);
      cout << "El fumador " << num_fumador << " quiere " << num_fumador << endl;
      sem_signal(mensajes);
      sem_wait(fum_disponibles[num_fumador]);
      
      sem_wait(mensajes);
      cout << "Se retira el ingrediente número: " << num_fumador << endl;
      sem_signal(mensajes);

      sem_signal(mostrador);
      fumar(num_fumador);

   }
}

//----------------------------------------------------------------------

int main()
{
   // declarar hebras y ponerlas en marcha
   // ......

   cout << "Los fumadores 2 y 3 quieren cerillas para fumar." << endl;

   thread hebra_estanquero(funcion_hebra_estanquero);
   thread hebra_fumador[4];
   thread hebra_Cuarto_fumador;

   for(unsigned long i = 0; i < 4; i++)
      hebra_fumador[i] = thread(funcion_hebra_fumador, i);
   hebra_estanquero.join();

   for(unsigned long i = 0; i < 4; i++)
      hebra_fumador[i].join();
   

}
