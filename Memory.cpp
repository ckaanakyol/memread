#define MEM_SIZE 512
#define DELAY 1000
#define INDEX_ARRAY_SIZE MEM_SIZE/8
#include "systemc.h"
#include <queue>
using namespace std;
SC_MODULE(Memory)
{
public:
    enum Function {
        FUNC_NONE,
        FUNC_READ,
        FUNC_WRITE,
        FUNC_CONT
    };
    enum RETSignal {
        RSIG_NONE,
        RSIG_READ_FIN,
        RSIG_WRITE_FIN,
        RSIG_ERROR
    };
    sc_in<bool> Port_CLK{"Port_CLK"};
    sc_in<Function> Port_Func{"Port_Func"};
    sc_in<int> Port_Addr{"Port_Addr"};
    sc_inout<int> Port_Data{"Port_Data"};
    sc_out<RETSignal> Port_DoneSig{"Port_DoneSig"};

    SC_CTOR(Memory)
    {
        SC_METHOD(execute);
        sensitive << Port_CLK;
        m_clkCnt = 0;
        m_curAddr = 0;
        m_curData = 0;
        m_curFunc = Memory::FUNC_NONE;
        m_data = new int[MEM_SIZE];
        fill(m_data);
        fillqueue(&values);
        m_writesCnt = 0;
        m_readsCnt = 0;
        m_errorsCnt = 0;
        cycle = 0;
        m_errorCode = 0;
        current = 0;
    }
    ~Memory() { delete[] m_data; }

private:
    int m_clkCnt;
    int m_curAddr;
    int m_curData;
    Function m_curFunc;
    int* m_data;
    int m_errorCode;
    int m_writesCnt;
    int m_readsCnt;
    int m_errorsCnt;
	std::queue <int> values;
	int current;
	int cycle;

    void fill(int* m_data)
    {
    	for(int i = 0 ; i < MEM_SIZE; i++)
    		m_data[i]=i;
    }

    void fillqueue(queue <int> *values)
    {
    	for(int i = 0 ; i < DELAY; i++)
    		values->push(INT_MIN);
    }

    RETSignal read(int finishedRequesting = 0)
    {
        if (finishedRequesting == 0) {
            Port_Data.write(m_data[m_curAddr]);
            current = m_data[m_curAddr];
            values.push(current);
            Port_Data.write(values.front());
            cout<< cycle <<" Mem data: "<< m_data[m_curAddr] <<endl;
            cycle ++ ;
            values.pop();
            m_readsCnt++;
            return RSIG_READ_FIN;
        }
        else {
        	//cout<<"values.front(): " << values.back() << " size: " << values.size() << "\n";
            if( values.size() > 0 ){
            	//cout << "values size: " << values.size() << endl;
	            Port_Data.write(values.front());
	            cycle ++ ;
	            values.pop();
	        }
	        else if (values.size() == 0)
	        	sc_stop();
	        return RSIG_ERROR;
        }
    }

    void execute()
    {
    	//cout<< sc_time_stamp() << "  m_curFunc: " << m_curFunc << endl ;
        if (m_curFunc != Memory::FUNC_NONE) {
            m_clkCnt++;
            //if (m_clkCnt == 100) {
                RETSignal retSig = Memory::RSIG_ERROR;
                switch (m_curFunc) {
                case Memory::FUNC_READ: {
                    retSig = read();
                    break;
                }
                case Memory::FUNC_CONT:{
                	retSig = read(1);
                	break;
                }
                /*case Memory::FUNC_WRITE: {
                    retSig = write();
                    break;
                }*/
                }
                Port_DoneSig.write(retSig);
                m_clkCnt = 0;
                m_curFunc = Memory::FUNC_NONE;
            //}
            return;
        }
        if (Port_Func.read() == Memory::FUNC_NONE) {
            return;
        }
        m_curFunc = Port_Func.read();
        m_curAddr = Port_Addr.read();
        m_curData = Port_Data.read();
        //cout << m_curData << endl;
        Port_DoneSig.write(Memory::RSIG_NONE);
    }
};

SC_MODULE(CPU)
{
public:
    sc_in<bool> Port_CLK{"Port_CLK"};
    sc_in<Memory::RETSignal> Port_MemDone{"Port_MemDone"};
    sc_out<Memory::Function> Port_MemFunc{"Port_MemFunc"};
    sc_out<int> Port_MemAddr{"Port_MemAddr"};
    sc_inout<int> Port_MemData{"Port_MemData"};
    SC_CTOR(CPU)
    {
        SC_METHOD(execCycle);
        sensitive << Port_CLK.neg();
        dont_initialize();
        SC_METHOD(memDone);
        sensitive << Port_MemDone;
        dont_initialize();
        m_waitMem = false;
        cycle = 0;
        index_array = new int[INDEX_ARRAY_SIZE];
        fillIndexArray(index_array);
    }

private:
    int* index_array;
    bool m_waitMem;
    int cycle;
    void fillIndexArray(int* index_array)
    {
    	for(int i = 0; i < INDEX_ARRAY_SIZE; i++)
    		index_array[i] = i;
    		//index_array[i] = rand() % MEM_SIZE;
    }
    
    int getRndAddress()
    {
        return (rand() % INDEX_ARRAY_SIZE);
    }
    int getRndData()
    {
        return rand();
    }
    void execCycle()
    {

	    Memory::Function f = Memory::FUNC_READ;
    	if(cycle < INDEX_ARRAY_SIZE)
    	{
	        if (m_waitMem) {
	            return;
	        }

	        int addr = index_array[cycle];
	        Port_MemFunc.write(f);
	        Port_MemAddr.write(addr);
	        //if (f == Memory::FUNC_WRITE)
	        //    Port_MemData.write(getRndData());
	        m_waitMem = true;
	        
    	}
    	else if ( cycle == INDEX_ARRAY_SIZE ){
    		cout<< "Finished requesting.....\n";
    		Port_MemFunc.write(Memory::FUNC_CONT);
    		cycle++;
    	}
    	if(Port_MemData.read() != INT_MIN)
    		cout<<cycle << " CPU data: "<< Port_MemData.read()<<endl;
	    cycle++;
    }
    void memDone()
    {
        if (Port_MemDone.read() == Memory::RSIG_NONE) {
            return;
        }
        m_waitMem = false;
        //Port_MemFunc.write(Memory::FUNC_NONE);
    }
};