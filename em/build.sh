for i in ../pxtone/*.cpp; do emcc -c $i; done
emcc -s WASM=1 -O1 -o index.html --pre-js pre.js *.o ../Main.cpp

