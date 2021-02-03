monitor ProdConsSUFIFO;

var ntotal_celdas = 10       : integer ;
    buffer[ntotal_celdas]    : integer ;
    primera_libre            : integer ;
    primera_ocupada          : integer ;
    nceldas_ocupadas         : integer ;
    ocupadas                 : condition ;
    libres                   : condition ;

process leer ;
begin
    while n_celdasocupadas == 0 then
        ocupadas.wait() ; 
    end
    assert(0 < n_celdasocupadas) ;
    var valor = buffer[primera_ocupada] : integer ;
    primera_ocupada = (primera_ocupada + 1) % ntotal_celdas ;
    n_celdasocupadas-- ;
    libres.signal() ;
    return valor
end

process escribir ;
begin
    while n_celdasocupadas == ntotal_celdas then
        libres.wait() ;
    assert(n_celdasocupadas < ntotal_celdas);
    buffer[primera_libre] = valor ;
    primera_libre = (primera_libre + 1) % ntotal_celdas ;
    n_celdasocupadas++ ;
    ocupadas.signal() ;
end