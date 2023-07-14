#include "timetron_core/proc_timeline.h"

#include "timetron_core/data_day.h"
#include "timetron_core/data_timeline.h"
#include "timetron_core/proc_util.h"

#include <tinyxml2.h>

#include <chrono>
#include <iomanip>
#include <sstream>
#include <thread>


using namespace timetron::core;


void proc_timeline::deserialise_from_xml(data_timeline &timeline, tinyxml2::XMLNode const *root)
{
    timeline.clear();

    tinyxml2::XMLElement const *xml_meta = nullptr;
    tinyxml2::XMLElement const *xml_days = nullptr;

    for (auto entry = root->FirstChild(); entry; entry = entry->NextSibling()) {
        if (0 == strcmp(entry->Value(), "meta"))
          xml_meta = entry->ToElement();
        else if (0 == strcmp(entry->Value(), "days"))
          xml_days = entry->ToElement();
    }

    if (xml_meta) {
        tinyxml2::XMLElement const* xml_meta_properties = nullptr;
        
        for (auto entry = xml_meta->FirstChild(); entry; entry = entry->NextSibling()) {
            if (0 == strcmp(entry->Value(), "properties"))
              xml_meta_properties = entry->ToElement();
        }

        if (xml_meta_properties)
          timeline.nickname = xml_meta_properties->Attribute("nickname");
    }

    if (xml_days) {
        for (auto entry = xml_days->FirstChild(); entry; entry = entry->NextSibling()) {
          if (0 == strcmp(entry->Value(), "day")) 
            this->deserialise_from_xml_day(timeline, entry);
        }
    }
}


void proc_timeline::deserialise_from_xml_day(data_timeline &timeline, tinyxml2::XMLNode const *day)
{
    proc_util util{};

    char const *date = day->ToElement()->Attribute("date");
    if (!date || !date[0])
      return;

    std::tm tm;
    memset(&tm, 0x0, sizeof(std::tm));

    // Deserialise the date
    std::istringstream ss(date);
    ss >> std::get_time(&tm, "%Y-%m-%d");
    std::time_t time = mktime(&tm);

    // Obtain day
    data_timeline::timestamp stamp{ time, 0 };
    for (; timeline.day_data.find(stamp) != timeline.day_data.end(); )
      ++stamp.unique;
    auto& data_day = timeline.day_data[stamp];
    data_day.time = time;

    // Load day data
    for (auto entry = day->FirstChild(); entry; entry = entry->NextSibling())
    {
        // Deserialise tasks
        if (0 == strcmp(entry->Value(), "task")) {
            data_day::task task;
            task.id             = util.replace_if_null(entry->ToElement()->Attribute("id"));
            task.replace_id     = util.replace_if_null(entry->ToElement()->Attribute("replace"));
            task.name           = util.replace_if_null(entry->ToElement()->Attribute("name"));
            task.weight         = entry->ToElement()->FloatAttribute("weight");
            task.target_per_day = entry->ToElement()->FloatAttribute("target_per_day");
        
            data_day.set_tasks.push_back(task);
            continue;
        }

        // Deserialise work
        if (0 == strcmp(entry->Value(), "work")) {
            data_day::work work;
            work.put_id         = util.replace_if_null(entry->ToElement()->Attribute("id"));
            work.take_id        = util.replace_if_null(entry->ToElement()->Attribute("leech_id"));
            work.minutes        = entry->ToElement()->FloatAttribute("min");
            if (work.take_id.size() == 0)
              work.take_id = work.put_id;

            data_day.done_work.push_back(work);
            continue;
        }

        // Deserialise consumse
        if (0 == strcmp(entry->Value(), "consume")) {
            data_day::consume consumed;
            consumed.water      = entry->ToElement()->FloatAttribute("water");
            consumed.jimmy      = entry->ToElement()->FloatAttribute("jimmy");
            consumed.set_water  = entry->ToElement()->FloatAttribute("goal_water");
            consumed.set_jimmy  = entry->ToElement()->FloatAttribute("goal_jimmy");

            data_day.consumed.push_back(consumed);
            continue;
        }
    }
}


proc_timeline::deserialise_result proc_timeline::deserialise_from_xml_safe(data_timeline &timeline, std::string const &filepath)
{
    tinyxml2::XMLDocument document;
    
    bool success;

    // We retry for a while in case the file was locked
    // This sometimes happens f.i. on Google Drive
    for (int retries = 0; retries < 200; ++retries) {
        success = (tinyxml2::XML_SUCCESS == document.LoadFile(filepath.c_str()));
        if (success)
          break;

        std::this_thread::sleep_for(std::chrono::duration<double, std::milli>(100.));
    }

    if (!success)
	  return deserialise_result::file_could_not_be_read;

    deserialise_from_xml(timeline, document.RootElement());

	return deserialise_result::ok;
}