
#include <mpi.h>
#include <thread> // this_thread::sleep_for
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include <iostream>

using namespace std;
using namespace std::this_thread ;
using namespace std::chrono ;

const int
    num_procesos_esperado = 9,
    num_items             = 20,
    tam_vector            = 1,
    np                    = 1,
    nc                    = 7,
    id_productor          = 8,
    id_buffer             = 7,
    id_consumidor         = 5,
    etiq_gordos           = 1,
    etiq_delgados         = 2,
    etiq_productor        = 3;

    
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

void funcion_productor()
{
    int peticion,
        valor_prod,
        valor_rec = 1;
    MPI_Status estado;
    
    while(true){
        MPI_Recv ( &valor_rec, 1, MPI_INT, id_buffer, 0, MPI_COMM_WORLD, &estado);
        cout << "\t\tSe empiezan a preparar 10 hamburguesas." << endl << flush;
        sleep_for(milliseconds(aleatorio<5,10>()));
        cout << "\t\tSe han preparado 10 hamburguesas." << endl << flush;
        cout << "\t\tReponedor va a enviar 10 hamburguesas." << endl << flush;
        MPI_Ssend( &valor_prod, 1, MPI_INT, id_buffer, etiq_productor, MPI_COMM_WORLD);
    }
}

void funcion_consumidor(int id_propio){
   int         peticion,
               valor_rec = 1 ;
   MPI_Status  estado ;

    while(true){
        cout << "\t\tEl alumno " << id_propio << " va hacia el camion" << endl << flush;
        sleep_for(milliseconds(aleatorio<5,10>()));
        cout << "\t\tEl alumno " << id_propio << " ha llegado al camion" << endl << flush;
        cout << "\t\tEl alumno " << id_propio << " va a pedir una hamburguesa" << endl << flush;

        if(id_propio % 2 == 0){
            MPI_Ssend( &peticion, 1, MPI_INT, id_buffer, etiq_gordos, MPI_COMM_WORLD);
        }
        else{
            MPI_Ssend ( &peticion, 1, MPI_INT, id_buffer, etiq_delgados, MPI_COMM_WORLD);
        }

        MPI_Recv ( &valor_rec, 1, MPI_INT, id_buffer, 0, MPI_COMM_WORLD, &estado);

        if(id_propio % 2 == 0){
            cout << "\t\tEl alumno " << id_propio << " ha recibido 2 hamburguesas" << endl << flush;
        }
        else{
            cout << "\t\tEl alumno " << id_propio << " ha recibido 1 hamburguesa" << endl << flush;
        }

        cout << "\t\tEl alumno " << id_propio << " empieza a comer y se aleja del camion" << endl << flush;
        sleep_for(milliseconds(aleatorio<5,10>()));

    }

}

void funcion_buffer(){
   int         disponibles = 10,
               valor,
               peticion;
   MPI_Status  estado ;                   // Metadatos del mensaje recibido

    while(true){

      // 1. determinar si puede enviar solo prod., solo cons, o todos

        cout << "\t\t" << disponibles << " hamburguesas disponibles" << endl << flush;
        if( disponibles >= 2 ){
            MPI_Recv( &valor, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &estado);
        }
        else if (disponibles == 1){
            MPI_Recv( &valor, 1, MPI_INT, MPI_ANY_SOURCE, etiq_delgados, MPI_COMM_WORLD, &estado);
        }
        else{
            MPI_Ssend( &peticion, 1, MPI_INT, id_productor, 0, MPI_COMM_WORLD);
            MPI_Recv( &valor, 1, MPI_INT, id_productor, etiq_productor, MPI_COMM_WORLD, &estado);
        }


      switch( estado.MPI_TAG ){   // leer emisor del mensaje en metadatos
         
         case etiq_productor:  // si ha sido el productor: insertar en buffer
            disponibles = 10;
            break;
         
         case etiq_delgados:    // si ha sido el consumidor: extraer y enviarle
            disponibles--;
            cout << "Dependiente va a enviar una hamburguesa al alumno" << estado.MPI_SOURCE << endl << flush;
            MPI_Ssend( &valor, 1, MPI_INT, estado.MPI_SOURCE, 0, MPI_COMM_WORLD);
            break;
         case etiq_gordos:
            disponibles -= 2;
            cout << "Dependiente va a enviar dos hamburguesas al alumno" << estado.MPI_SOURCE << endl << flush;
            MPI_Ssend( &valor, 1, MPI_INT, estado.MPI_SOURCE, 0, MPI_COMM_WORLD);
            break;
      }
   }
}

int main( int argc, char *argv[] )
{
  int id_propio, num_procesos_actual; // ident. propio, núm. de procesos

  MPI_Init( &argc, &argv );
  MPI_Comm_rank( MPI_COMM_WORLD, &id_propio );
  MPI_Comm_size( MPI_COMM_WORLD, &num_procesos_actual );

  if ( num_procesos_esperado == num_procesos_actual )
  {
    if ( id_propio == id_productor )  // si mi ident. es el del productor
      funcion_productor();            //    ejecutar función del productor
    else if ( id_propio == id_buffer )// si mi ident. es el del buffer
      funcion_buffer();               //    ejecutar función buffer
    else                              // en otro caso, mi ident es consumidor
      funcion_consumidor(id_propio);           //    ejecutar función consumidor
  }
  else if ( id_propio == 0 ){  // si hay error, el proceso 0 informa
    cerr << "error: número de procesos distinto del esperado." << endl ;
    cout << "el número de procesos esperados es:    " << num_procesos_esperado << endl
             << "el número de procesos en ejecución es: " << num_procesos_actual << endl
             << "(programa abortado)" << endl ;
  }
  
  MPI_Finalize( );
  return 0;
}