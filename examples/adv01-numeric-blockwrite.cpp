
#include <iostream>
#include <time.h>
#include <slowbook.hpp>

namespace sb = slowbook;


int main(void)
{
    double t0 = time(NULL);

    sb::DataStore_JsonDump ds;
    //sb::DataStore_CsvFile ds("SlowStore_");
    
    sb::SimpleNumericSchema schema("NumericData");
    
    for (int i = 0; i < 3; i++) {
        // DataFrame: set of Records, to be written at once //
        sb::SlowDashDataFrame frame(schema);
        frame.tag("ch0");

        // data block //
        for (int j = 0; j < 10; j++) {
            // fill data //
            double t = t0 + 10*i + j;
            double x = 100*i + j*j;
            frame.add(sb::Record(schema).time(t).value(x));
        }
        
        // store data //
        ds.append(frame);
    }
    
    return 0;
}
