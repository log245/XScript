
local a = Test_Base:new();
local ddd = "sfasdfsdf"; 
function a:GetThis( v ) 
	print( "call GetThis", v:x(), v:y(), ddd );
   return 3;
end
function a:NewThisLua() 
   return self;
end

for i, v in pairs(_G) do
    print("_G->", i)
end 

a:kkk():x(10)
print( a:x(), ddd )
a:kkk( CVector2f:new())
a:kkk():x(1000)
a:x(100)
r = a:GetNumber( CVector2f:new(), 10 )
print( r );
print( r:x(), r:y() );
print( Test_Base.GetCppName( a:kkk() ) );
print( a:TestRet64( "asdfff", "9580" ) );

