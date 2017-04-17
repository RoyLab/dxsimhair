#include "stdafx.h"

#ifdef COMPILE_AS_EXE

#include "HairAnim2Simulator.h"
using namespace HairCore;

int main() {
	HairConfiguration conf({});
	HairAnim2Simulator simulator(conf);
	float trans[16];
	simulator.on_frame(trans);
	return 0;
}

#endif // COMPILE_AS_EXE
