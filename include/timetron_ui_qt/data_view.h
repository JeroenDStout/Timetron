#pragma once


#include <ctime>
#include <string>
#include <vector>


namespace timetron::ui_qt {


    class data_view_urgency_task {
      public:
        std::string name;
        float       penalty;
        float       abs_progress;
        float       full_progress;
        int         emoji_water_count;
        int         emoji_clover_count;
        int         emoji_peach_count;
        int         emoji_star_count;
    };

    class data_view_urgency {
      public:
        using task = data_view_urgency_task;

        void clear();

        std::vector<task> tasks;

    };


}
