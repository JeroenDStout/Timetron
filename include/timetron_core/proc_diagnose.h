#pragma once


#include <ctime>
#include <string>
#include <vector>


namespace timetron::core {

	class data_timeline;
	class data_diagnostic;
	class data_diagnostic_by_period;
	class data_work_in_period;

	class proc_diagnose {
	  public:
		void  fill_diagnostic(data_timeline const &, data_diagnostic &);
		void  fill_diagnostic_organised(data_diagnostic const &, data_diagnostic_by_period &);
			  
		void  create_periods(data_diagnostic &, std::time_t const &reference_time);
		float get_minutes_in_period(data_work_in_period const &, std::string const &taskName);
	};


}