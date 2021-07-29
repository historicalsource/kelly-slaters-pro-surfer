cd gas
del *.o
del *.irx
make release PROJECT=KELLY_SLATER
copy gas.irx c:\ks\data
cd ..
