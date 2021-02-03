//  Realizado por Víctor José Rubia López
//  Compilar:  mpicxx -std=c++11 ejerciciop2.cpp -o ejerciciop2
//  Ejecutar:  mpirun -np 14 ./ejerciciop2
#include <mpi.h>
#include <thread> // this_thread::sleep_for
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include <iostream>

using namespace std;
using namespace std::this_thread ;
using namespace std::chrono ;

const int
    num_productores       = 1, //   Número de dependientes
    num_consumidores      = 12, //  Número de alumnos
    id_productor          = 13, //  ID del cocinero
    id_dependiente        = 12, //  ID del dependiente
    etiq_tragones         = 1, // etiqueta para alumnos tragones
    etiq_delgados         = 2,  // etiqueta para alumnos no tragones
    etiq_cocinero         = 3,  // etiqueta para el cocinero
    num_procesos_esperado = num_productores + num_consumidores + 1; // número de procesos que espera el programa (14)

    
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

void funcion_cocinero()
{
    int peticion,
        valor_prod,
        valor_rec = 1;
    MPI_Status estado;
    
    while(true){
        MPI_Recv ( &valor_rec, 1, MPI_INT, id_dependiente, 0, MPI_COMM_WORLD, &estado); //  Cuando recibo el mensaje del dependiente
        cout << "Se empiezan a preparar 10 bocatas." << endl << flush;
        sleep_for(seconds(aleatorio<1,3>()));
        cout << "Se han preparado 10 bocatas." << endl << flush;
        cout << "Cocinero va a enviar 10 bocatas." << endl << flush;
        MPI_Ssend( &valor_prod, 1, MPI_INT, id_dependiente, etiq_cocinero, MPI_COMM_WORLD); // Le mando los 20 bocatas
    }
}

void funcion_alumno(int id_propio){
   int         peticion,
               valor_rec = 1 ;
   MPI_Status  estado ;

    while(true){
        cout << "\t\tEl alumno " << id_propio << " va hacia el camion" << endl << flush;
        sleep_for(seconds(aleatorio<1,2>()));
        cout << "\t\tEl alumno " << id_propio << " ha llegado al camion" << endl << flush;
        cout << "\t\tEl alumno " << id_propio << " va a pedir un bocata" << endl << flush;

        //  1. pide bocadilo

        if(id_propio % 2 == 0){     // Si es tragón
            MPI_Ssend( &peticion, 1, MPI_INT, id_dependiente, etiq_tragones, MPI_COMM_WORLD);
        }
        else{   // Si no es tragón
            MPI_Ssend ( &peticion, 1, MPI_INT, id_dependiente, etiq_delgados, MPI_COMM_WORLD);
        }

        //  2. recibe el bocadillo

        MPI_Recv ( &valor_rec, 1, MPI_INT, id_dependiente, 0, MPI_COMM_WORLD, &estado);

        if(id_propio % 2 == 0){
            cout << "\t\tEl alumno " << id_propio << " ha recibido 2 bocatas" << endl << flush;
        }
        else{
            cout << "\t\tEl alumno " << id_propio << " ha recibido 1 bocata" << endl << flush;
        }

        //  3. se ha comido el bocadillo y se va

        cout << "\t\tEl alumno " << id_propio << " empieza a comer y se aleja del camion" << endl << flush;
        sleep_for(seconds(aleatorio<1,2>()));

    }

}

void funcion_dependiente(){
   int         disponibles = 20,
               valor,
               peticion;
   MPI_Status  estado ;                   // Metadatos del mensaje recibido

    while(true){

      // 1. determinar si puede enviar solo cocinero., solo alumno, o todos

        cout << disponibles << " bocatas disponibles" << endl << flush;
        if( disponibles >= 3 ){
            MPI_Recv( &valor, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &estado);
        }
        else if (disponibles >= 1 ){
            MPI_Recv( &valor, 1, MPI_INT, MPI_ANY_SOURCE, etiq_delgados, MPI_COMM_WORLD, &estado);
        }
        else{
            MPI_Ssend( &peticion, 1, MPI_INT, id_productor, 0, MPI_COMM_WORLD);
            MPI_Recv( &valor, 1, MPI_INT, id_productor, etiq_cocinero, MPI_COMM_WORLD, &estado);
            cout << "Cocinero! Mas bocatas por favor!" << endl << flush;
        }


      switch( estado.MPI_TAG ){   // leer emisor del mensaje en metadatos
         
         case etiq_cocinero:  // si ha sido el cocinero: insertar bocatas
            disponibles = 20;
            break;
         
         case etiq_delgados:    // si ha sido el alumno: extraer y enviarle un bocata
            disponibles--;
            cout << "Dependiente va a enviar un bocata al alumno " << estado.MPI_SOURCE << endl << flush;
            MPI_Ssend( &valor, 1, MPI_INT, estado.MPI_SOURCE, 0, MPI_COMM_WORLD);
            break;
         case etiq_tragones:  // si ha sido el alumno tragón: extraer y enviarle 2 bocatas
            disponibles -= 2;
            cout << "Dependiente va a enviar dos bocatas al alumno " << estado.MPI_SOURCE << endl << flush;
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
    if ( id_propio == id_productor )  // si mi ident. es el del cocinero
      funcion_cocinero();            //    ejecutar función cocinero
    else if ( id_propio == id_dependiente )// si mi ident. es el del dependiente
      funcion_dependiente();               //    ejecutar función buffer
    else                              // en otro caso, mi ident es alumno
      funcion_alumno(id_propio);           //    ejecutar función alumno
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