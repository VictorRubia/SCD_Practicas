monitor LectoresEscritores;

var num_lec                  : integer ;
    escribiendo              : boolean ;
    lectura                  : condition ;
    escritura                : condition ;

process iniLec ;
begin
    if escribiendo then
        lectura.wait() ;
    end if
    num_lec++ ;
    lectura.signal() ;
end

process finLec ;
begin
    num_lec-- ;
    if num_lec == 0 then
        escritura.signal () ;
    end if
end

process iniEsc ;
begin
    if num_lec > 0 OR escribiendo then
        escritura.wait() ;
    end if
    escribiendo = true ;
end

process finEsc ;
begin
    escribiendo = false ;
    if lectura.get_nwt() != 0 then {si hay lectores leyendo, tienen prioridad sobre un escritor}
        lectura.signal() ; {si el número de procesos esperando es dto de 0}
    end if
    else then
        escritura.signal() ;
    end else
end