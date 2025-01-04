LLVM_FLAGS=`llvm-config --cxxflags --ldflags --system-libs --libs core passes`
proc:
	make llvmir
	make out
make-pass:
	clang -shared -fPIC -std=c++17 newFlatten.cpp -o newFlattenpass.so $(LLVM_FLAGS)
run-pass:
	opt -load ./newFlattenpass.so -load-pass-plugin ./newFlattenpass.so -passes="all-name" -sos-name=sudo_debug main.ll -S -o newMain.ll
toso:
	clang newMain.ll -c -fPIC -o sudo.o
dot:
	opt -analyze -dot-cfg main.ll
	dot -Tpng ._Z7processP4Node.dot -o 1.png
passed-dot:
	opt -analyze -dot-cfg -disable-verify newMain.ll
	dot -Tpng ._Z7processP4Node.dot -o 2.png

llvmir:
	clang -O0 -emit-llvm -S main.cpp -o main.ll
out:
	clang main.ll -o main.out
