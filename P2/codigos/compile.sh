g++ -pthread -O3 -std=c++11 fumadores.cpp Semaphore.cpp HoareMonitor.cpp -o fumadores_exe
g++ -pthread -O3 -std=c++11 lectorescritorSU.cpp Semaphore.cpp HoareMonitor.cpp -o lectorescritorSU_exe
g++ -pthread -O3 -std=c++11 prodconsSUFIFO.cpp Semaphore.cpp HoareMonitor.cpp -o prodconsSUFIFO_exe
g++ -pthread -O3 -std=c++11 prodconsSULIFO.cpp Semaphore.cpp HoareMonitor.cpp -o prodconsSULIFO_exe