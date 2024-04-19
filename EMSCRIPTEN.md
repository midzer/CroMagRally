# Emscripten

## Compile

```
mkdir build
cd build
emcmake cmake .. -DCMAKE_BUILD_TYPE=Release
```

## Link

```
em++ -flto -O3 */*.o *.o libPomme.a libGL.a -o index.html -sUSE_SDL=2 -sFULL_ES2=1 -lGL -sASYNCIFY -sINITIAL_MEMORY=1024mb --preload-file Data/ --closure 1
```
