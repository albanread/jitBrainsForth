( start.f define some common words )

 0.0000001 fconstant tolerance

: f=  f-  fabs tolerance f< ;

: f<>  f-  fabs  tolerance f< not ;

: space 32 emit ;

: spaces do space loop ;

: cr  13 emit 10 emit ;

: sq dup * ;

-1 constant TRUE

0 constant FALSE

: fact
    dup 2 < if
            drop 1 exit
        then
        dup
        begin
            dup 2 > while
            1- swap over * swap
        repeat
    drop ;

: rfact  DUP 2 < IF DROP 1 EXIT THEN  DUP 1- RECURSE * ;


s" loaded start.f " s. cr