#include "BspcLibIncludes.hpp"

int main()
{
	Q1_AllocMaxBSP();

	Q1_LoadBSPFile(const_cast<char*>("e1m1.bsp"), 0, 1365176);
	Q1_FreeMaxBSP();

	Q3_WriteBSPFile(const_cast<char*>("q3e1m1.bsp"));
}
