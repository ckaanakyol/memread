#include "systemc.h"
#include "Memory.cpp"
int sc_main(int argc, char* argv[])
{
	cout << "\n\nCreating Modules............";
	/* Instantiate Modules */
	Memory mem("main_memory");
	CPU cpu("cpu");
	/* Signals */
	sc_signal<Memory::Function, SC_MANY_WRITERS> sigMemFunc{"sigMemFunc"};
	sc_signal<Memory::RETSignal> sigMemDone{"sigMemDone"};
	sc_signal<int> sigMemAddr{"sigMemAddr"};
	sc_signal<int, SC_MANY_WRITERS> sigMemData{"sigMemData"};
	/* The clock that will drive the CPU and Memory*/
	sc_clock clk;
	cout << "DONE\nConnecting Modules' Ports...";
	/* Connecting module ports with signals */
	mem.Port_Func(sigMemFunc);
	mem.Port_Addr(sigMemAddr);
	mem.Port_Data(sigMemData);
	mem.Port_DoneSig(sigMemDone);
	cpu.Port_MemFunc(sigMemFunc);
	cpu.Port_MemAddr(sigMemAddr);
	cpu.Port_MemData(sigMemData);
	cpu.Port_MemDone(sigMemDone);
	mem.Port_CLK(clk);
	cpu.Port_CLK(clk);
	cout << "DONE\n\n\nRunning (press CTRL+C to exit)... \n";
	/* Start Simulation */
	sc_start(500000.0, SC_PS);
	return 0;
}