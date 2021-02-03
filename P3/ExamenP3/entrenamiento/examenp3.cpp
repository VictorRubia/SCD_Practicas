#include <mpi.h>
#include <thread> // this_thread::sleep_for
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include <iostream>

using namespace std;
using namespace std::this_thread ;
using namespace std::chrono ;

const int
   num_clientes = 10 ,
   num_cajas     = 3,
   num_procesos  = num_clientes + 1,
    etiq_entrar = 2,
    etiq_salir = 1;


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

// ---------------------------------------------------------------------

void funcion_clientes( int id )
{
  int id_pago = num_procesos - 1,
      peticion; 

  while ( true )
  {

    cout <<"Cliente " <<id <<" entra a la tienda y coge sus productos" <<endl << flush ;
    sleep_for( milliseconds( aleatorio<10,100>() ) );

    cout << "Cliente " << id << " solicita pagar" << endl << flush;
    MPI_Ssend( &peticion, 1, MPI_INT, id_pago, etiq_entrar, MPI_COMM_WORLD);


    cout <<"Cliente " <<id <<" esta pagando" <<endl << flush ;
    sleep_for( milliseconds( aleatorio<10,100>() ) );


    cout << "Cliente " << id << " ha pagado" << endl << flush;
    MPI_Ssend( &peticion, 1, MPI_INT, id_pago, etiq_salir, MPI_COMM_WORLD);

    cout << "Cliente " << id << " se va" << endl << flush;
    sleep_for( milliseconds( aleatorio<10,100>() ) );
 }
}
// ---------------------------------------------------------------------

void funcion_pago(){
   int cajas_ocupadas = 0,
       etiq_actual,
       valor; // valor recibido
   MPI_Status estado; // metadatos de las dos recepciones

   while(true){
      if( cajas_ocupadas >= num_cajas - 1)
         etiq_actual = etiq_salir;
      else
         etiq_actual = MPI_ANY_TAG;
      
      MPI_Recv( &valor, 1, MPI_INT, MPI_ANY_SOURCE, etiq_actual, MPI_COMM_WORLD, &estado);

      switch (estado.MPI_TAG)
      {
      case etiq_entrar:
         cout << "El cliente " << estado.MPI_SOURCE << " pasa por la caja " << cajas_ocupadas << endl << flush;
         cajas_ocupadas++;
         break;
      
      case etiq_salir:
         cajas_ocupadas--;
         cout << "El cliente " << estado.MPI_SOURCE << " sale de su caja." << endl << flush;
         break;
      }
   }
}

int main( int argc, char** argv )
{
   int id_propio, num_procesos_actual ;

   MPI_Init( &argc, &argv );
   MPI_Comm_rank( MPI_COMM_WORLD, &id_propio );
   MPI_Comm_size( MPI_COMM_WORLD, &num_procesos_actual );


   if ( num_procesos == num_procesos_actual )
   {
      // ejecutar la función correspondiente a 'id_propio'
      if( id_propio < num_procesos - 1)
         funcion_clientes(id_propio);
      else{
         funcion_pago();
      }
   }
   else
   {
      if ( id_propio == 0 ) // solo el primero escribe error, indep. del rol
      { cout << "el número de procesos esperados es:    " << num_procesos << endl << flush
             << "el número de procesos en ejecución es: " << num_procesos_actual << endl << flush
             << "(programa abortado)" << endl << flush ;
      }
   }

   MPI_Finalize( );
   return 0;
}

// ---------------------------------------------------------------------
