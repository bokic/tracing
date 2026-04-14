#include "tracing/tracing.h"

#include <unistd.h>
#include <iostream>


void funct1()
{
    trace_delay_start("usleep");
    usleep(100000); 
    trace_delay_end();
    
    trace_delay_start("usleep");
    usleep(100000); 
    trace_delay_end();
    
    trace_delay_start("usleep");
    usleep(100000); 
    trace_delay_end();
}

int main()
{
    std::cout << "Application running with automatic tracing session..." << std::endl;

    funct1();
    usleep(50000);
    funct1();

    usleep(500000);

    return 0;
}
