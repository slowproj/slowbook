
#include <iostream>
#include <time.h>
#include <slowbook.hpp>

namespace sb = slowbook;


int main(void)
{
    double start = 0, step = 10;
    sb::Trend<sb::TrendPointLastMinMax> trend(step, start);

    sb::DataStore_JsonDump ds;
    //sb::DataStore_CsvFile ds("SlowStore_");

    sb::SimpleObjectSchema schema("Objects");
    
    for (int i = 0; i < 3; i++) {
        // fill data
        for (double t = 0; t < 100; t++) {
            trend.fill(start+100*i+t, 3+std::sin(t));
        }

        // store the trend as a graph object (JSON string) //
        // Here enslow() converts the object into JSON string.
        ds.update(sb::SlowDashDataFrame(schema).tag("trend") << enslow(trend));
    }
    
    return 0;
}
