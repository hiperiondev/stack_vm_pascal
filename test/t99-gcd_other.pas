var x: integer;
function gcd(i, j: integer) : integer;
begin
  if i=j then gcd:=i;
  if i>j then gcd:=gcd(i-j,j);
  if i<j then gcd:=gcd(i,j-i);
end;

begin
   x := gcd(1,2);
   write(x);
end.