@echo off

echo Outputting Python bindings
swig.exe -python -c++ -outdir bind -o src/py_swig.cpp .\swig.i

echo Outputting Node bindings
swig.exe -javascript -node -c++ -outdir bind -o src/js_swig.cpp .\swig.i