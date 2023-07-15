#include "timetron_core/data_diagnostic.h"


using namespace timetron::core;


void data_diagnostic::clear()
{
    this->current_tasks.clear();
    this->periods.clear();
}


void data_diagnostic_by_period::clear()
{
    this->tasks_by_period.clear();
}
