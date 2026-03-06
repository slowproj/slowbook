
#include <iostream>
#include <time.h>

#include <slowbook.hpp>

namespace sb = slowbook;


int main(void)
{
    sb::RandomWalk ch0, ch1;
    
    sb::DataStore_JsonDump ds;
    //sb::DataStore_CsvFile ds("SlowStore_");
    
    sb::SimpleNumericSchema schema("NumericData");
    
    while (true) {
        long t = time(NULL);
        double x = ch0.get();

        ds.append(sb::SlowDashDataFrame(schema).tag("ch0").time(t) << x);
        
        std::cout << t << "  " << x << std::endl;
        sleep(1);
    }
    
    return 0;
}
