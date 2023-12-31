#pragma once


#include <ctime>
#include <map>
#include <string>
#include <unordered_map>


namespace timetron::core {

    class data_day;
    class data_work_in_period;
    
    class data_diagnostic {
      public:
        using work_period_v = std::vector<data_work_in_period>;

        // Current state of a task
        struct task {
            std::string   id, name;
            std::time_t   last_occurrence;
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

    class data_diagnostic_by_period {
      public:
        struct task_in_period : public data_diagnostic::task {
            int min_diag_period, max_diag_period;
        };

        void clear();

        std::vector<task_in_period> tasks_by_period;
    };


	enum class data_work_in_period_type {
		day,
		week_1,
		week_4,
		year
	};

    // Total work done within a time slice
    class data_work_in_period {
      public:
        using time_by_name        = std::unordered_map<std::string, float>;
        using work_in_period_type = data_work_in_period_type;

        std::string         name;
        std::time_t         start, end;
        work_in_period_type period_type;
        float               full_minutes;
        time_by_name        task_minutes;
    };

}