cd gas
del *.o
del *.irx
make chatty PROJECT=KELLY_SLATER
copy gas.irx c:\ks\data
cd ..
