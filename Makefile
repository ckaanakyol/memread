export SYSTEMC_HOME=/home/tools/systemc-2.3.1
g++ -I. -I$SYSTEMC_HOME/include -L. -L$SYSTEMC_HOME/lib-linux -o mem Memory.cpp sc_main.cpp -lsystemc -lm