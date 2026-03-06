
#include <iostream>
#include <time.h>

#include <slowbook.hpp>

namespace sb = slowbook;


int main(void)
{
    sb::RandomWalk ch0, ch1;
    
    sb::DataStore_JsonDump ds;
    //sb::DataStore_CsvFile ds("SlowStore_");
    
#if 0
    sb::SimpleNumericSchema schema("NumericData");
#else
    // the line above is equivalent to using the plain schema with column definitions below
    sb::Schema schema("NumericData");
    schema.add_time<long>("time");
    schema.add_tag<std::string>("channel");
    schema.add_field<double>("value");
#endif

    
    while (true) {
        long t = time(NULL);
        double x = ch0.get();

        // store data //
#if 0
        ds.append(sb::SlowDashDataFrame(schema).tag("ch0").time(t) << x);
#else
        // alternative way to store data (works only for numeric values) //
        ds.append(sb::Record(schema).tag("ch0").time(t).value(x));
#endif
        
        std::cout << t << "  " << x << std::endl;
        sleep(1);
    }
    
    return 0;
}
