var ans, u, v, w : integer;
    x: uinteger;

function sub3(x, y, z : integer ) : uinteger;
var p: uinteger;
begin
   p := x + y + 6;
   sub3 := x - y - z + p + w;
end;

{ procedure newline(); begin writeln('') end; }
procedure newline(); var nl: char; begin nl := 10; write(nl) end;

begin
   u := 9; v := 2; w := -3;

   ans := sub3(u, v, w);
   write(ans); newline();

   write(sub3(w,v,u)); newline();
end.
