var x : integer;  a: array[5] of integer; z: integer;
procedure newline(); var nl: char; begin nl := 10; write(nl) end;

begin
   a[0] := 10;
   a[1] := 11;
   a[2] := 12;
   a[3] := 13;
   z:= 23 + a[2];
   a[4] := z;

   write(-a[1]); newline();
   write(-a[1]+a[3]); newline();

   x := -a[2] * a[3];
   write(x); newline();

end.
