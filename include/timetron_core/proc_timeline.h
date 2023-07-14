#pragma once


#include <ctime>
#include <string>
#include <vector>


namespace tinyxml2 {
	class XMLNode;
}


namespace timetron::core {

	class data_timeline;
	class data_day;

	class proc_timeline {
	  public:
	    enum class deserialise_result {
			ok,
			file_could_not_be_read
		};

		void deserialise_from_xml(data_timeline &, tinyxml2::XMLNode const *); 
		void deserialise_from_xml_day(data_timeline &, tinyxml2::XMLNode const *);

	    deserialise_result deserialise_from_xml_safe(data_timeline &, std::string const &filepath);
	};


}