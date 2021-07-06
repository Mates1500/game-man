// game-man.cpp : Defines the entry point for the application.
//

#include "game-man.h"

#include "file_handle.h"
#include "memory.h"

using namespace std;

int main()
{
	cout << "Hello CMake." << endl;
	auto s = Memory();
	auto fh = FileHandle("E:\\tetris-rom\\tetris-rom.gb");
	auto& vec = fh.GetFileContentsVector();
	s.SetRomMemory(vec);
	cout << std::hex << offsetof(MemoryMap, switchable_rom_bank) << endl;
	scanf_s("%s");
	return 0;
}
