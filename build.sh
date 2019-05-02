for i in ./pxtone/*.cpp; do emcc -c $i; done
emcc --bind -s "EXTRA_EXPORTED_RUNTIME_METHODS=['ccall']" -o docs/pxtone.js -O2 *.o emscripten_bindings.cpp
