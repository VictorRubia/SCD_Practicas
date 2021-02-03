#include <iostream>
#include <cassert>
#include <thread>
#include <mutex>
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include "HoareMonitor.h"

using namespace std ;
using namespace HM ;

const int num_fumadores = 3;
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

class Estanco : public HoareMonitor{
   private:
      CondVar fumador[num_fumadores], mostrador;
      int ingrediente;
   public:
      Estanco(){
         ingrediente=-1;
         mostrador= newCondVar();
         for(int i = 0; i < num_fumadores; i++)
            fumador[i]= newCondVar();
      }

      void obtenerIngredienteFumador(int num_fumador){
         if(num_fumador==ingrediente){
            mostrador.signal();
            fumador[num_fumador].wait();
            ingrediente=-1;
         }
         else{
            fumador[num_fumador].wait();
         }
      }

      void ponerIngredienteMostrador(int ingr){
         ingrediente=ingr;
         fumador[ingrediente].signal();
      }

      void esperarIngredienteMostrador(){
         if(ingrediente!=-1)
            mostrador.wait();
      }
};

//-------------------------------------------------------------------------
// Función que simula la acción de producir un ingrediente, como un retardo
// aleatorio de la hebra (devuelve número de ingrediente producido)

int producir_ingrediente()
{
   // calcular milisegundos aleatorios de duración de la acción de fumar)
   chrono::milliseconds duracion_produ( aleatorio<10,100>() );

   // informa de que comienza a producir
   mtx.lock();
   cout << "Estanquero : empieza a producir ingrediente (" << duracion_produ.count() << " milisegundos)" << endl;
   mtx.unlock();

   // espera bloqueada un tiempo igual a ''duracion_produ' milisegundos
   this_thread::sleep_for( duracion_produ );

   const int num_ingrediente = aleatorio<0,num_fumadores-1>() ;

   // informa de que ha terminado de producir
   mtx.lock();
   cout << "Estanquero : termina de producir ingrediente " << num_ingrediente << endl;
   mtx.unlock();

   return num_ingrediente ;
}

//----------------------------------------------------------------------
// función que ejecuta la hebra del estanquero

void funcion_hebra_estanquero( MRef<Estanco> monitor )
{
   while(true){
      const int ingr = producir_ingrediente();
      monitor->ponerIngredienteMostrador(ingr);
      monitor->esperarIngredienteMostrador();
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
void  funcion_hebra_fumador( MRef<Estanco> monitor, int num_fumador )
{
   while( true )
   {
      monitor->obtenerIngredienteFumador(num_fumador);
      fumar(num_fumador);
   }
}

//----------------------------------------------------------------------

int main()
{

   MRef<Estanco> monitorEstanco = Create<Estanco>();
   thread hebra_fumador[num_fumadores];
   thread hebra_estanquero(funcion_hebra_estanquero, monitorEstanco);

   for(int i=0; i < num_fumadores; i++)
      hebra_fumador[i] = thread(funcion_hebra_fumador, monitorEstanco, i);
   hebra_estanquero.join();

   for(int i=0; i < num_fumadores; i++)
      hebra_fumador[i].join();

}
