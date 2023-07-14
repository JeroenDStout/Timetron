#pragma once


#include <ctime>
#include <string>
#include <vector>


namespace timetron::core {

	class data_timeline;
	class data_diagnostic;

	class proc_util {
	  public:
		char const * replace_if_null(char const *);
	};


}