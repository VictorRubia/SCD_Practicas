#include <iostream>
#include <cassert>
#include <thread>
#include <mutex>
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include "Semaphore.h"
#include "HoareMonitor.h"

using namespace std ;
using namespace HM ;


// Variables compartidas

const int num_lectores = 3;     // Numero de lectores
const int num_escritores = 5;   // Numero de escritores


/*******************************************************************************/

template< int min, int max > int aleatorio()
{
  static default_random_engine generador( (random_device())() );
  static uniform_int_distribution<int> distribucion_uniforme( min, max ) ;
  return distribucion_uniforme( generador );
}

/*******************************************************************************/

class Lec_Esc : public HoareMonitor
{
 private:
 CondVar                       // variables condicion:
   lectura,
   escritura;

int
   n_lec,
   num_lecturas,               // Numero de lecturas que se han hecho
   num_escrituras;             // Numero de escrituras que se han hecho

bool
  escrib,
  lectores_esperando,        // Indica si el lector está esperando al escritor (en caso de igualdad)
  escritores_esperando;      // Indica si el escritor está esperando al lector (en caso de igualdad)


 public:                    // constructor y métodos públicos
   Lec_Esc() ;
   void ini_lectura ();
   void ini_escritura();
   void fin_lectura ();
   void fin_escritura();
} ;

//-------------------------------------------------------------------------
// Constructor

Lec_Esc::Lec_Esc()
{
   lectura = newCondVar();
   escritura = newCondVar();

   num_lecturas = 0;
   num_escrituras = 0;

   lectores_esperando = false;
   escritores_esperando = false;

   escrib = false;
   n_lec = 0;  
}

//-------------------------------------------------------------------------
// Inicio de lectura

void Lec_Esc::ini_lectura()
{
  if (escrib){                                            // Si hay un escritor dentro

    if (num_lecturas > num_escrituras){                   // Si se han hecho más lecturas que escrituras 
      if (!(!lectura.empty() && escritura.empty()))       // Si no ocurre que la cola de lecturas está vacía y la otra no
        lectura.wait();
    }

    else if (num_lecturas == num_escrituras){             // Si se han hecho el mismo número de lecturas que de escrituras
      if (!escritores_esperando){                         // Si el escritor no está esperando al lector
        int espero = aleatorio<0,1>();                    // Decido aleatoriamente si espero yo o no

        if (espero == 1){                                 // Si el que espera soy yo (espero == 1)
          lectura.wait();
          lectores_esperando = true;                      // Indico que estoy esperando
        }
      }
    }
  }

  n_lec++;

  lectura.signal();
}

//-------------------------------------------------------------------------
// Inicio de escritura

void Lec_Esc::ini_escritura()
{
  if (escrib || n_lec > 0){                             // Valga como comentario el mismo de ini_lectura, pero cambiando los papeles

    if (num_escrituras > num_lecturas){                   
      if (!(!escritura.empty() && lectura.empty()))
        escritura.wait();
    }

    else if (num_lecturas == num_escrituras){
      if (!lectores_esperando){
        int espero = aleatorio<0,1>();

        if (espero == 1){
          escritura.wait();
          escritores_esperando = true;
        }
      }
    }
  }

  escrib = true;
}


//-------------------------------------------------------------------------
// Fin de lectura

void Lec_Esc::fin_lectura()
{
  n_lec--;

  if (lectores_esperando)
    lectores_esperando = false;

  if (n_lec == 0)
    escritura.signal();
}

//-------------------------------------------------------------------------
// Fin de escritura

void Lec_Esc::fin_escritura()
{
  escrib = false;

  if (escritores_esperando)
    escritores_esperando = false;

  if (!lectura.empty())
    lectura.signal();
  else
    escritura.signal();
}

// *****************************************************************************
// funciones de espera aleatoria

//-------------------------------------------------------------------------
// Función que simula la acción de lectura, con un retardo aleatorio

void lectura(int n_hebra)
{
   // calcular milisegundos aleatorios de duración de la acción de leer)
   chrono::milliseconds duracion_lectura( aleatorio<100,1000>() );

   // informa de que comienza a leer
   cout << "Hebra " << n_hebra << " : empieza a leer ";
   cout << "(" << duracion_lectura.count() << " milisegundos)" << endl;

   // espera bloqueada un tiempo igual a ''duracion_lectura' milisegundos
   this_thread::sleep_for( duracion_lectura );

   // informa de que ha terminado de leer
   cout << "Hebra " << n_hebra << " : termina de leer " << endl;
}


//-------------------------------------------------------------------------
// Función que simula la acción de escribir, con un retardo aleatorio

void escritura(int n_hebra)
{
   // calcular milisegundos aleatorios de duración de la acción de escribir)
   chrono::milliseconds duracion_escritura( aleatorio<100,1000>() );

   // informa de que comienza a escribir
   cout << "Hebra " << n_hebra << " : empieza a escribir ";
   cout << "(" << duracion_escritura.count() << " milisegundos)" << endl;

   // espera bloqueada un tiempo igual a ''duracion_escritura' milisegundos
   this_thread::sleep_for( duracion_escritura );

   // informa de que ha terminado de escribir
   cout << "Hebra " << n_hebra << " : termina de escribir " << endl;
}

//-------------------------------------------------------------------------
// Función que simula la acción de ejecutar el resto del código (lectores)

void resto_codigo_lectores(int n_hebra)
{
   // calcular milisegundos aleatorios de duración de la acción de esperar)
   chrono::milliseconds duracion_espera_lectores( aleatorio<100,1000>() );

   // espera bloqueada un tiempo igual a ''duracion_espera_lectores' milisegundos
   this_thread::sleep_for( duracion_espera_lectores );
}

//-------------------------------------------------------------------------
// Función que simula la acción de ejecutar el resto del código (escritores)

void resto_codigo_escritores(int n_hebra)
{
   // calcular milisegundos aleatorios de duración de la acción de esperar)
   chrono::milliseconds duracion_espera_escritores( aleatorio<60,600>() );

   // espera bloqueada un tiempo igual a ''duracion_espera_lectores' milisegundos
   this_thread::sleep_for( duracion_espera_escritores );
}



// *****************************************************************************
// funciones de hebras

//----------------------------------------------------------------------
// función que ejecuta la hebra del lector

void Lector (MRef <Lec_Esc> monitor, int num_hebra)
{
  while (true)
  {
    monitor->ini_lectura();

    lectura(num_hebra);

    monitor->fin_lectura();

    resto_codigo_lectores(num_hebra);
  }
}


//----------------------------------------------------------------------
// función que ejecuta la hebra del escritor

void Escritor (MRef <Lec_Esc> monitor, int num_hebra)
{
  while (true)
  {
    monitor->ini_escritura();

    escritura(num_hebra);

    monitor->fin_escritura();

    resto_codigo_escritores(num_hebra);
  }
}

//----------------------------------------------------------------------

int main()
{
   cout << "--------------------------------------------------------" << endl
        << "Problema del Lec_Esc - Monitor SU." << endl
        << "--------------------------------------------------------" << endl
        << flush ;
        
    MRef <Lec_Esc> monitor = Create <Lec_Esc> ();     

   thread hebras_lectores [num_lectores];
   thread hebras_escritores[num_escritores];
   
    for (int i=0; i<num_lectores; i++)
      hebras_lectores[i] = thread (Lector, monitor, i);

    for (int i=0; i<num_escritores; i++)
      hebras_escritores[i] = thread (Escritor, monitor, i);
             
    for (int i=0; i<num_lectores; i++)
      hebras_lectores[i].join();

    for (int i=0; i<num_escritores; i++)
      hebras_escritores[i].join();
}