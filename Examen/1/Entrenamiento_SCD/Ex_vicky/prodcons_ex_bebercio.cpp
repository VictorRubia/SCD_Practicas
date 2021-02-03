// Corrección del examen
#include <iostream>
#include <iomanip>
#include <cassert>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <random>
#include "HoareMonitor.h"
using namespace std;
using namespace HM;

constexpr int
   num_items  = 4 ;     // número de items a producir/consumir

mutex
   mtx ;                 // mutex de escritura en pantalla
unsigned
   cont_prod[num_items], // contadores de verificación: producidos
   cont_cons[num_items]; // contadores de verificación: consumidos
const int numero_productores=2; //numero de hebras productoras
const int numero_consumidores=4; //numero de hebras consumidoras

int ultimo_servido = -1;
int valores_por_hebra[numero_productores]={0};
int producir_por_hebra = num_items/numero_productores;  //numero de valores que produce cada hebra productora
int consumir_por_hebra = num_items/numero_consumidores;  //numero de valores que consume cada hebra consumidora



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

//**********************************************************************
// funciones comunes a las dos soluciones (fifo y lifo)
//----------------------------------------------------------------------

int elegir_cocktail(int numero_hebra)
{
   this_thread::sleep_for( chrono::milliseconds( aleatorio<20,100>() ));

   int producido;

   do{
      producido = aleatorio<0,3>();
   }while(ultimo_servido == producido);   // no se servirá en la barra el mismo cóctel 2 veces seguidas 
   
   return producido ;
}

// *****************************************************************************
// clase para monitor buffer, version LIFO, semántica SC, un prod. y un cons.

class Bebercio : public HoareMonitor
{
private:
   CondVar buffer[4], productores;
   int producido;

public:                    
   Bebercio(){
      producido = -1;
      productores = newCondVar();
      for(int i = 0; i < numero_consumidores; i++)
         buffer[i]= newCondVar();
   }   

   void servir_cocktail(int c){

      if(producido != -1){
         productores.wait();
      }
      
      producido = c;
      buffer[c].signal();

   }

   void pedir_cocktail(int i){

      if(i == producido){
         producido=-1;
         productores.signal();
      }
      else{
         buffer[i].wait();
      }
   }
} ;

// *****************************************************************************
// funciones de hebras

void funcion_hebra_barman( MRef<Bebercio> monitor, int num_hebra )
{
   // for( unsigned i = 0 ; i < producir_por_hebra*3 ; i++ ){
   while(true){
		int cocktail = elegir_cocktail(num_hebra) ;
      this_thread::sleep_for( chrono::milliseconds( aleatorio<20,100>() )); // Preparando la bebida
		mtx.lock();
      cout << "Barman " << num_hebra << " ha servido el cocktail " << cocktail << endl;
      mtx.unlock();
      monitor->servir_cocktail( cocktail );
	}
}
//-----------------------------------------------------------------------------
void funcion_hebra_consumidora( MRef<Bebercio> monitor, int num_hebra )
{
   // for( unsigned i = 0 ; i < consumir_por_hebra*3 ; i++ ){
   while(true){
      mtx.lock();
      cout << "Consumidor " << num_hebra << " pide el cocktail " << num_hebra << endl;
      mtx.unlock();
      monitor->pedir_cocktail(num_hebra);
      this_thread::sleep_for( chrono::milliseconds( aleatorio<20,100>() )); // Preparando la bebida
      mtx.lock();
      cout << "Consumidor " << num_hebra << " está bebiendo feliz." << endl;
      mtx.unlock();
   }
}
// -----------------------------------------------------------------------------

int main()
{
   cout << "-------------------------------------------------------------------------------" << endl
        << " Productores consumidores con  (" << numero_productores
		  << " productores y " << numero_consumidores << " consumidores, versión SU y LIFO). " << endl
        << "-------------------------------------------------------------------------------" << endl
        << flush ;
   
   MRef<Bebercio> monitor = Create<Bebercio>();
   
   thread hebra_productora[numero_productores];
   thread hebra_consumidora[numero_consumidores];

   for (int i=0;i<numero_productores;i++)
         hebra_productora[i] = thread(funcion_hebra_barman, monitor, i);
   for (int i =0;i<numero_consumidores;i++)
      hebra_consumidora[i] = thread(funcion_hebra_consumidora, monitor, i);

   for (int i=0;i<numero_productores;i++)
      hebra_productora[i].join();
   for (int i =0;i<numero_consumidores;i++)
      hebra_consumidora[i].join();
}