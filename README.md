1. 每个文件都是可以独立运行的，在项目目录下使用make run，后面用TARGET=文件名即可单独编译对应文件并运行。输出均为main.exe。

in home folder: 
run: 
make run TARGET=*.cpp 
make run TARGET=hello_world.cpp 
or 
mingw32-make run TARGET=hello_world.cpp 

2. 可以给src文件夹下的每个文件单独生成可执行文件（TARGET是源文件，EXECUTABLE是可执行文件名）：
make run TARGET=xx.cpp EXECUTABLE=xx