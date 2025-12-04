### Linux

Contruir apenas uma vez
```bash
g++ main.cpp -o cpu_sim
```

construir assembly
```bash
./cpu_sim build firmware.txt os.bin
```

rodar binário
```bash
./cpu_sim run os.bin -q
```

**-q** ou **--quiet** são formas de esconder os logs de Cache, então retire para obter tudo!