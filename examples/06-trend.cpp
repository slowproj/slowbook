
#include <iostream>
#include <slowbook.hpp>

namespace sb = slowbook;


int main(void)
{
    double start = time(NULL), step = 10;
    sb::Trend<sb::TrendPointMeanMinMax> trend(step, start);

    sb::DataStore_JsonDump ds;
    //sb::DataStore_CsvFile ds("SlowStore_");

    sb::SimpleTrendSchema<decltype(trend)::point_type> schema("RecordTrend");
    
    auto mark = trend.mark();
    for (int i = 0; i < 3; i++) {
        // fill data //
        start += 100;
        for (double t = 0; t < 100; t++) {
            trend.fill(start+t, 3+std::sin(t));
        }

        // store data //
#if 0
        // Incremental writing: Writing only the updated parts from the last one using a "mark".
        ds.append(sb::SlowDashDataFrame(schema).tag("trend") << trend.since(mark));
        mark = trend.mark();
#else
        // Alternatively, you can delete the data points that have been recorded
        ds.append(sb::SlowDashDataFrame(schema).tag("trend") << trend);
        trend.truncate();
#endif
    }
    
    return 0;
}
