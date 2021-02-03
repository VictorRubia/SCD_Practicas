#include <iostream>
#include <iomanip>
#include <cassert>
#include <thread>
#include <random>
#include "HoareMonitor.h"

using namespace std ;
using namespace HM ;

//**********************************************************************
// variables compartidas
constexpr int num_items  = 40, productores = 4, consumidores = 4;     
mutex mtx ;  
unsigned cont_prod[num_items] = {0},
         cont_cons[num_items] = {0},
         producidos[productores];
//  Parámetros examen
constexpr int v1_tam = 4, v2_tam = 4

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

int producir_dato( int i)
{
		static int contador = 0 ;
		this_thread::sleep_for( chrono::milliseconds( aleatorio<20,100>() ));
		mtx.lock();
		cout << "producido: " << contador << endl << flush ;
        if(i % 2 == 0)
		    v2[i]++;
        else
            v1[i]++;
		mtx.unlock();
		cont_prod[contador] ++ ;
		return contador++;
}
//-----------------------------------------------------------------------------

void consumir_dato( unsigned dato )
{
   assert( dato < num_items );
   cont_cons[dato] ++ ;
   this_thread::sleep_for( chrono::milliseconds( aleatorio<20,100>() ));
   mtx.lock();
   cout << "                  consumido: " << dato << endl ;
   mtx.unlock();
}
//-----------------------------------------------------------------------------
void test_contadores()
{
   bool ok = true ;
   cout << "comprobando contadores ...." << flush ;

   for( unsigned i = 0 ; i < num_items ; i++ )
   {
      if ( cont_prod[i] != 1 )
      {
         cout << "error: valor " << i << " producido " << cont_prod[i] << " veces." << endl ;
         ok = false ;
      }
      if ( cont_cons[i] != 1 )
      {
         cout << "error: valor " << i << " consumido " << cont_cons[i] << " veces" << endl ;
         ok = false ;
      }
   }
   if (ok)
      cout << endl << flush << "solución (aparentemente) correcta." << endl << flush ;
}
//-----------------------------------------------------------------------------
class ProdConsSULIFO : public HoareMonitor
{
private:
   static const int ntotal_celdas = 10 ;   
   int buffer[ntotal_celdas], primera_libre ;          
   CondVar ocupadas, libres ;                 

public:                    
   ProdConsSULIFO(){
      primera_libre = 0 ;
      ocupadas = newCondVar();
      libres = newCondVar();
   }      
   int  leer(){
      while (primera_libre == 0)
         ocupadas.wait();
      assert(0 < primera_libre);
      const int valor = buffer[primera_libre - 1];
      primera_libre--;

      libres.signal();

      return valor;
   }
   void escribir(int valor){
      while (primera_libre == ntotal_celdas)
         libres.wait();
      assert(primera_libre < ntotal_celdas);
      buffer[primera_libre] = valor;
      primera_libre++;

      ocupadas.signal();
   }
} ;

//-----------------------------------------------------------------------------
void funcion_hebra_productora( MRef<ProdConsSULIFO> monitor, int num_hebra )
{
	for( unsigned i = 0 ; i < num_items/productores ; i++ )
   {
		int valor = producir_dato(num_hebra) ;
		monitor->escribir( valor );
	}
}
//-----------------------------------------------------------------------------
void funcion_hebra_consumidora( MRef<ProdConsSULIFO> monitor, int num_hebra )
{
   for( unsigned i = 0 ; i < num_items/consumidores ; i++ )
   {
    int valor = monitor->leer();
    consumir_dato( valor );
   }
}
//-----------------------------------------------------------------------------
int main()
{
   cout << "-------------------------------------------------------------------------------" << endl
        << " Productores consumidores con  (" << productores
		  << " productores y " << consumidores << " consumidores, versión SU y LIFO). " << endl
        << "-------------------------------------------------------------------------------" << endl
        << flush ;

    MRef<ProdConsSULIFO> monitor = Create<ProdConsSULIFO>( );
	thread hebraprod[productores], hebracons[consumidores];

    v1[productores];
    v2[productores];

	for(int k = 0; k < productores; k++){
		v1[k] = 0;
        v2[k] = 0;
	}

	for(int i = 0; i < productores; i++){
		hebraprod[i] = thread ( funcion_hebra_productora, monitor, i );
	}

	for(int j = 0; j < consumidores; j++){
		hebracons[j] = thread ( funcion_hebra_consumidora, monitor, j );
	}

	for(int i = 0; i < productores; i++){
		hebraprod[i].join();
	}

	for(int j = 0; j < consumidores; j++){
		hebracons[j].join();
	}

	for(int l = 0; l < productores; l++){
		if(l % 2 == 0)
            cout << "Hebra " <<  l << " ha producido " << v2[l] << " valores" << endl;
        else
            cout << "Hebra " <<  l << " ha producido " << v1[l] << " valores" << endl;
    }

   test_contadores() ;
}