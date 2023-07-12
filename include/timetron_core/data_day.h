#pragma once


#include <ctime>
#include <string>
#include <vector>


namespace timetron::core {


    class data_day
    {
      public:
        class task {
          public:
            std::string  id, name;
            std::string  replace_id, leech_id;
            float        weight;
            float        target_per_day;
        };

        class work {
          public:
            std::string  take_id, put_id;
            float        minutes;
        };

        class consume {
          public:
            float        water, set_water;
            float        jimmy, set_jimmy;
        };

        std::time_t	         time;
        std::vector<task>    set_tasks;
        std::vector<work>    done_work;
        std::vector<consume> consumed;
    };


}