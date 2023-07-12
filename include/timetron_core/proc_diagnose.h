#pragma once


#include <ctime>
#include <string>
#include <vector>


namespace timetron::core {

	class data_timeline;
	class data_diagnostic;

	class proc_diagnose {
	  public:
		void fill_diagnostic(data_timeline const &timeline, data_diagnostic &diagnostic);
		void create_periods(data_diagnostic &diagnostic, std::time_t const &reference_time);
	};


}