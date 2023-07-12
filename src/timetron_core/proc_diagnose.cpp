#include "timetron_core/proc_diagnose.h"

#include "timetron_core/data_day.h"
#include "timetron_core/data_diagnostic.h"
#include "timetron_core/data_timeline.h"

#include <algorithm>
#include <chrono>
#include <sstream>
#include <iomanip>


using namespace timetron::core;


void proc_diagnose::fill_diagnostic(data_timeline const& timeline, data_diagnostic& diagnostic)
{
    constexpr int day_length  = 24 * 3600;

    diagnostic.clear();

    if (timeline.day_data.size() == 0)
      return;

    this->create_periods(diagnostic, timeline.day_data.rbegin()->first.time);
    
    // Get the current time
    std::time_t const current_time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

    // Helper struct to keep track of tasks as they are updated
    struct task_data {
        std::string name;
        
        bool        use_target_per_day;
                    
        float       absolute_weight, absolute_per_day;
                    
        float       relative_weight;
        float       minute_weight, minute_progress;
        float       absence_penalty;
                    
        float       computed_progress;
        
        std::time_t last_occurrence;
        
        task_data()
        : use_target_per_day(false)
        , absolute_weight(0.f), absolute_per_day(0.f)
        , relative_weight(0.f), minute_weight(0.f), minute_progress(0.f), absence_penalty(0.f)
        , computed_progress(0.f)
        , last_occurrence(0)
        { ; }
    };
    std::unordered_map<std::string, task_data> tasks;

    float              sum_day_weight_absolute      = 0.f;
    float              avg_minute_progress          = 0.f;
    std::vector<float> sorted_minute_progress;
    std::vector<float> sorted_minute_progress_penalty;
    float              med_minute_progress;

    std::time_t        previous_time = (
      timeline.day_data.begin()->first.time - day_length
    );

    // Helper function to update the averages
    auto fn_update_avg = [&](bool include_penalty = false) { 
        avg_minute_progress = 0.f;
        sorted_minute_progress.clear();
        sorted_minute_progress_penalty.clear();
        
        if (tasks.size() == 0)
          return;
        
        for (auto &task : tasks) {
            if (   task.second.minute_weight    == 0.f
                && task.second.absolute_per_day == 0.f )
            {
                task.second.computed_progress = 0.f;
                continue;
            }
            task.second.computed_progress = task.second.minute_progress;
            
            avg_minute_progress   += task.second.computed_progress;
            if (include_penalty && task.second.use_target_per_day)
              avg_minute_progress += task.second.absence_penalty;
            
            sorted_minute_progress.push_back(task.second.computed_progress);
            sorted_minute_progress_penalty.push_back(task.second.computed_progress + task.second.absence_penalty);
        }
        
        avg_minute_progress /= float(tasks.size());
        
        std::sort(sorted_minute_progress.begin(), sorted_minute_progress.end(), std::less<float>());
        std::sort(sorted_minute_progress_penalty.begin(), sorted_minute_progress_penalty.end(), std::less<float>());
        
        // Find the median minute progress - take care of the edgecase in which there is an even amount
        med_minute_progress = sorted_minute_progress[sorted_minute_progress.size() / 2];
        if (sorted_minute_progress.size() % 1 == 0) {
            med_minute_progress += sorted_minute_progress[sorted_minute_progress.size() / 2 + 1];
            med_minute_progress /= 2.f;
        }
    };

    // Per day, process the data into the diagnostic
    for (auto &data : timeline.day_data)
    {
        // Increase penalties based on how long it has been since the previous day
        float const day_since_prev_weight = float(data.first.time - previous_time) / float(day_length);
        sum_day_weight_absolute += day_since_prev_weight;
        
        for (auto &task : tasks)
          task.second.absence_penalty -= task.second.absolute_per_day * day_since_prev_weight;
        
        // Set the previous time to today
        previous_time = data.first.time;

        // If tasks have been updated on this day we update the averages;
        // they are relevant to adding new tasks
        bool const tasks_updated = data.second.set_tasks.size() > 0;
        if (tasks_updated)
          fn_update_avg();

        // Update all the new tasks
        for (auto &set_task : data.second.set_tasks) {
            auto &task_data = tasks[set_task.id];
            
            // If this task sets a name for the first time it is seen as this task
            // being initialised; we set its minute progress to the average, which
            // will make it show up right in the middle of the progress
            if (   set_task.name.length() != 0
                && task_data.name.length() == 0)
            {
                task_data.name            = set_task.name;
                task_data.minute_progress = (set_task.id.find(':') == std::string::npos) ? avg_minute_progress : 0.f;
            }
            
            // Update the weights - if replace_id is set, we let this task replace
            // a previous task, copying over all of its progress and penalties
            if (set_task.replace_id.length() == 0) {
                task_data.absolute_weight    = set_task.weight;
                task_data.absolute_per_day   = set_task.target_per_day;
                task_data.use_target_per_day = task_data.absolute_per_day > 0.f;
            }
            else {
                auto &replace_task_data = tasks[set_task.replace_id];
              
                task_data.absolute_weight    = replace_task_data.absolute_weight;
                task_data.absolute_per_day   = replace_task_data.absolute_per_day;
                task_data.use_target_per_day = replace_task_data.use_target_per_day;
                task_data.minute_progress    = replace_task_data.minute_progress;
                task_data.absence_penalty    = replace_task_data.absence_penalty;
            
                // The task that has been replaced has its weights & progress cleared
                replace_task_data.absolute_weight  = 0.f;
                replace_task_data.absolute_per_day = 0.f;
                replace_task_data.minute_progress  = 0.f;
            }
        }

        // If tasks were updated, their relative weights will probably
        // have changed; so we update them and recompute the average
        if (tasks_updated) {
            float sum_absolute_weight = 0.f;
            for (auto &task : tasks) {
              sum_absolute_weight += task.second.absolute_weight;
            }
            
            for (auto &task : tasks) {
                task.second.relative_weight = task.second.absolute_weight / sum_absolute_weight;
                task.second.minute_weight = task.second.relative_weight;
            }
            
            fn_update_avg();
        }
        
        // For all done work update the progress of the tasks
        for (auto &done_work : data.second.done_work) {
            auto &task_data = tasks[done_work.take_id];
            
            task_data.last_occurrence = data.first.time;
            task_data.minute_progress += done_work.minutes;
            
            for (auto &task : tasks)
              task.second.minute_progress -= task.second.minute_weight * done_work.minutes;
        }

        // For all periods add the done work
        for (auto &period : diagnostic.periods) {
            if (period.start > data.first.time || period.end <= data.first.time)
              continue;
            
            for (auto &done_work : data.second.done_work)
              period.minutes[done_work.put_id] += done_work.minutes;
        }
    }

    float goal_water       = 1.f;
    float goal_jimmy       = 1.f;
    float jimmy_fac        = 0.f;
    float water_fac        = 0.f;
    float jimmy_strong_fac = 0.f;
    float water_strong_fac = 0.f;
    
    // Compute the water and jimmy data
    for (auto &data : timeline.day_data) {
        if (data.second.consumed.size() == 0)
          continue;

        // The day weight is computed the integral of the falloff over the day
        // The strong day weight is simply a linear map from 1 to 0 on the following day
        float const days_in_past      = float(current_time - data.first.time) / float(day_length);
        float       day_weight        = float(
            (-std::exp(-0.05 * std::max(0.f, days_in_past      )))
          + ( std::exp(-0.05 * std::max(0.f, days_in_past - 1.f)))
        );
        float       day_weight_strong = std::min(1.f, std::max(0.f, 2.f - days_in_past));

        // If the day is *today* we add some weight to it
        if (days_in_past < 1.f)
          day_weight += std::max(0.f, 1.f - days_in_past);

        for (auto &consume : data.second.consumed) {
            // Handle setting
            if (consume.set_water > 0.f)
              goal_water = consume.set_water;
            if (consume.set_jimmy > 0.f)
              goal_jimmy = consume.set_jimmy;
            
            // Add the factors
            jimmy_fac        += consume.jimmy / goal_jimmy * day_weight;
            water_fac        += consume.water / goal_water * day_weight;
            jimmy_strong_fac += consume.jimmy / goal_jimmy * day_weight_strong;
            water_strong_fac += consume.water / goal_water * day_weight_strong;
        }
    }
    
    // Update the averages including the penalties
    fn_update_avg(true);

    // Create a stable average range by selecting 3/4ths of all progress values
    int const minute_progress_count = int(sorted_minute_progress.size());
    int       stable_avg_count      = std::max(
      std::min(5, minute_progress_count),
      (minute_progress_count * 3) / 4
    );
    if ((stable_avg_count & 0x1) != (minute_progress_count & 0x1))
      stable_avg_count += 1;

    // Set the stable average
    float stable_average = 0.f;
    for (int i = 0, q = (minute_progress_count - stable_avg_count) / 2; i < stable_avg_count; i++, q++)
      stable_average += sorted_minute_progress[q];
    stable_average /= float(stable_avg_count);

    // Set the diagnostic tasks
    for (auto &task : tasks) {
        data_diagnostic::task new_task;
        new_task.id                     = task.first;
        new_task.name                   = task.second.name;
        new_task.last_occurrence        = task.second.last_occurrence;
        new_task.relative_weight        = task.second.relative_weight;
        new_task.minute_progress_factor = task.second.computed_progress - stable_average;
        new_task.minute_absence_penalty = task.second.absence_penalty;

        diagnostic.current_tasks[task.first] = new_task;
    }

    // Set the disagnostic consumables
    diagnostic.jimmy_fac = .75f * jimmy_fac + .25f * jimmy_strong_fac;
    diagnostic.water_fac = .50f * water_fac + .50f * water_strong_fac;
}


void proc_diagnose::create_periods(data_diagnostic& diagnostic, std::time_t const &reference_time)
{
    constexpr int day_length  = 24 * 3600;
    constexpr int week_length = 7 * day_length;

    constexpr int period_week_count  = 4;
    constexpr int period_4week_count = 6;
    constexpr int period_year_count  = 2;

    // Get the current time
    std::time_t const current_day    = reference_time + day_length;
    std::time_t const current_week   = current_day  - ((current_day  - 4 * day_length) % week_length);
    
    // Create a period for today
    {
        std::tm current_day_tm;
        gmtime_s(&current_day_tm, &current_day);

        std::stringstream ss;
        ss << std::put_time(&current_day_tm, "%a");

        data_diagnostic::work_period period_today;
        period_today.name  = ss.str();
        period_today.start = current_day - 1 * day_length;
        period_today.end   = std::numeric_limits<std::time_t>::max();
        diagnostic.periods.push_back(period_today);
    }
    
    std::time_t parse_week = current_week;

    // Create a period per week (including the current)
    {
        if (parse_week >= current_day - day_length)
          parse_week -= week_length;

        for (int i = 0; i < period_week_count; ++i) {
            if (i > 0)
              parse_week -= week_length;
            
            std::time_t end_t = std::min(current_day - day_length, parse_week + 6 * day_length);
            auto const length = end_t - parse_week;
            
            std::tm parse_week_tm;
            gmtime_s(&parse_week_tm, &parse_week);

            std::stringstream ss;
            if (length < day_length)
            {
                // Write week day name
                ss << std::put_time(&parse_week_tm, "%a");
            }
            else if (length >= 6 * day_length)
            {
                // Write week number
                ss << std::put_time(&parse_week_tm, "%V");
            }
            else
            {
                // Write week day range
                ss << std::put_time(&parse_week_tm, "%a") << "-";
                gmtime_s(&parse_week_tm, &parse_week);
                ss << std::put_time(&parse_week_tm, "%a");
            }
            
            data_diagnostic::work_period period_week;
            period_week.name  = ss.str();
            period_week.start = parse_week - day_length;
            period_week.end   = std::min(current_day - day_length, parse_week + 6 * day_length);
            diagnostic.periods.push_back(period_week);
        }
    }

    std::time_t parse_4week = parse_week - ((parse_week - 4 * day_length) % (4 * week_length));
    if (parse_4week == parse_week)
      parse_4week -= 4 * week_length;
      

    // Create a period per 4 weeks
    {
        for (int i = 0; i < period_4week_count; ++i) {
            if (i > 0)
              parse_4week -= 28 * day_length;
            
            std::time_t end_t = std::min(parse_week - day_length, parse_4week + 27 * day_length);
            
            std::tm parse_4week_tm;
            gmtime_s(&parse_4week_tm, &parse_4week);

            std::stringstream ss;
            ss << std::put_time(&parse_4week_tm, "%V");
            if ((end_t - parse_4week) > week_length) {
                gmtime_s(&parse_4week_tm, &end_t);
                ss << "-" << std::put_time(&parse_4week_tm, "%V");
            }
            
            data_diagnostic::work_period period_4week;
            period_4week.name  = ss.str();
            period_4week.start = parse_4week - day_length;
            period_4week.end   = std::min(parse_week - day_length, parse_4week + 27 * day_length);
            diagnostic.periods.push_back(period_4week);
        }
    }
      
    std::time_t year_time_end   = parse_4week;
    std::time_t year_time_start = year_time_end;
    std::tm year_tm_parse;
    gmtime_s(&year_tm_parse, &year_time_end);
    year_tm_parse.tm_mon = 0;
    year_tm_parse.tm_mday = 1;
    year_tm_parse.tm_yday = 1;
      
    // Create a period per year
    {
        for (int i = 0; i < period_year_count; ++i) {
            if (i > 0) {
                year_time_end = year_time_start;
                year_tm_parse.tm_year -= 1;
            }
            year_time_start = _mkgmtime(&year_tm_parse);
            
            std::stringstream ss;
            ss << std::put_time(&year_tm_parse, "%Y");
            
            data_diagnostic::work_period period_year;
            period_year.name  = ss.str();
            period_year.start = year_time_start;
            period_year.end   = year_time_end;
            diagnostic.periods.push_back(period_year);
        }
    }
}