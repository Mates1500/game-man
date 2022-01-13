// game-man.cpp : Defines the entry point for the application.
//

#include "game-man.h"


#include "cpu.h"
#include "file_handle.h"
#include "memory.h"

using namespace std;

int main()
{
	cout << "Hello CMake." << endl;
	auto gc = GamepadController();
	auto mem = Memory(gc);
	auto fh = FileHandle("E:\\tetris-rom\\tetris-rom.gb");
	auto& vec = fh.GetFileContentsVector();
	mem.SetRomMemory(vec);
	auto gb_cpu = Cpu(mem);
	gb_cpu.StartExecution();
	scanf_s("%s");
	return 0;
}
