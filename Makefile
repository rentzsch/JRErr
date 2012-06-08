test: build
	build/TestJRErrGCC
	build/TestJRErrClang
	
build: *.m
	mkdir build
	gcc -o build/TestJRErrGCC *.m -framework Foundation
	clang -o build/TestJRErrClang *.m -framework Foundation

.PHONY: clean

clean:
	rm -rf build
