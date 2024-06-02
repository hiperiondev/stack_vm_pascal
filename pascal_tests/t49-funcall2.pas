var x, y: uinteger;
procedure newline(); var nl: char; begin nl := 10; write(nl) end;
function add(x, y : uinteger): uinteger; begin add := x + y; end;

begin
   x := 1; y := 2;
   write(add(x,y)); newline();
   write(add(-3,-3)); newline();
end.
