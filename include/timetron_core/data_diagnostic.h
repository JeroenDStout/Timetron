#pragma once


#include <ctime>
#include <map>
#include <string>
#include <unordered_map>


namespace timetron::core {

    class data_day;
    
    class data_diagnostic {
      public:
        using time_by_name = std::unordered_map<std::string, float>;

        // Total work done within a time slice
        struct work_period {
          std::string   name;
          std::time_t   start, end;
          time_by_name  minutes;
        };
        using work_period_v = std::vector<work_period>;

        // Current state of a task
        struct task {
          std::string   id, name;
          std::time_t   last_occurrence;
          bool          is_extra;
          float         relative_weight;
          float         minute_progress_factor;
          float         minute_absence_penalty;
        };
        using task_by_name = std::unordered_map<std::string, task>;

        void clear();

        float          jimmy_fac;
        float          water_fac;
                       
        task_by_name   current_tasks;
        work_period_v  periods;
    };

}