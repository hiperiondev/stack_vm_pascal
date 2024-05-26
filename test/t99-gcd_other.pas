{ greatest common divisor, recursive }
function gcd(i, j:integer):integer;
begin
   if i=j then gcd:=i;
   if i>j then gcd:=gcd(i-j,j);
   if i<j then gcd:=gcd(i,j-i);
end;

begin
   write(1);
end.
