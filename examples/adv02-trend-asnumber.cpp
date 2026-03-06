
#include <iostream>
#include <time.h>
#include <slowbook.hpp>

namespace sb = slowbook;


int main(void)
{
    double start = time(NULL), step = 10;
    sb::Trend<sb::TrendPointLastMinMax> trend(step, start);

    sb::DataStore_JsonDump ds;
    //sb::DataStore_CsvFile ds("SlowStore_");

    sb::SimpleNumericSchema schema("NumericTrend");
    
    auto mark = trend.mark();
    for (int i = 0; i < 3; i++) {
        // fill data //
        for (double t = 0; t < 100; t++) {
            trend.fill(start+100*i+t, 3+std::sin(t));
        }

        // store data //
        // By using the SimpleNumericSchema, which has only one numeric value column,
        // only the "main" trend field is recorded. To record all the fields, use SimpleTrendSchema instead.
        ds.append(sb::SlowDashDataFrame(schema).tag("trend") << trend.since(mark));
        mark = trend.mark();
    }
    
    return 0;
}
