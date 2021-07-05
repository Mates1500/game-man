// game-man.cpp : Defines the entry point for the application.
//

#include "game-man.h"
#include "memory.h"

using namespace std;

int main()
{
	cout << "Hello CMake." << endl;
	auto s = Memory();
	cout << sizeof(s) << endl;
	s.SetMemory16(0x4000, 258);
	auto a = s.ReadMemory16(0x4000);
	cout << std::hex << offsetof(MemoryMap, switchable_rom_bank) << endl;
	scanf_s("%s");
	return 0;
}
