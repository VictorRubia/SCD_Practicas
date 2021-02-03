process Fumador[ i : 0..2 ] ;
begin
    while true do begin
        Estanco.producir_ingrediente();
        fumar(i);
    end
end

monitor Estanco ;

var fumador[i]  : condition ; { qué fumadores se encuentran fumando }
    mostrador   : condition ; { ocupación del mostrador }
    ingrediente : integer   ; { ingrediente que se produce }

process obtenerIngredienteFumador[ i : 0..2 ] ;
begin
    if i == ingrediente then { si el fumador quiere el ingrediente que el estanco ha producido}
        mostrador.signal ; { se pone el mostrador a disponible }
        fumador[i].wait() ; { se hace una espera aleatoria }
        ingrediente = -1 ; { se quita el ingrediente del mostrador }
    end if
    else then
        fumador[i].wait() ; { se espera el fumador en la calle }
    end else
end


process ponerIngredientesMostrador[ i : 0..2 ];
begin
    ingrediente = i ; {si el ingrediente es igual al de i}
    fumador[ingrediente].signal() ; { el fumador que quiere el ingrediente i se le da paso al mostrador }
end

proces esperarIngredienteMostrador
begin
    if ingrediente != -1 then
        mostrador.wait() ;
    end if
end