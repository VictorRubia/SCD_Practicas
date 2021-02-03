monitor ProdConsSULIFO;

var num_celdas_total = 10    : integer ;
    buffer[num_celdas_total] : integer ;
    primera_libre            : integer ;
    ocupadas                 : condition ;
    libres                   : condition ;

process leer ;
begin
    while primera_libre == 0 then
        ocupadas.wait() ; 
    end
    assert(0 < primera_libre) ;
    var valor = buffer[primera_libre - 1] : integer ;
    primera_libre-- ;
    libres.signal() ;
    return valor
end

process escribir ;
begin
    while primera_libre == num_celdas_total then
        libres.wait() ;
    assert(primera_libre < num_celdas_total);
    buffer[primera_libre] = valor ;
    primera_libre++ ;
    ocupadas.signal() ;
end