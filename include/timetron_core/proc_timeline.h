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
		void deserialise_from_xml(data_timeline &, tinyxml2::XMLNode const *); 
		void deserialise_from_xml_day(data_timeline &, tinyxml2::XMLNode const *);
	};


}