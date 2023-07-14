#pragma once

#include "timetron_core/data_day.h"

#include <ctime>
#include <map>
#include <string>
#include <vector>


namespace timetron::core {

    class data_day;

    class data_timeline {
      public:
        void clear();

        struct timestamp {
            std::time_t time;
            std::uint32_t unique;

            bool operator()(const timestamp& lhs, const timestamp& rhs) const {
                if (lhs.time != rhs.time)
                    return lhs.time < rhs.time;
                return lhs.unique > rhs.unique;
            }
        };

        std::string                              nickname;
        std::map<timestamp, data_day, timestamp> day_data;
    };

}
