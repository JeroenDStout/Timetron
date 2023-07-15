#pragma once

#include <string>

class QGridLayout;
class QLabel;
class QVBoxLayout;


namespace timetron::core {
	class data_diagnostic;
	class data_diagnostic_by_period;
	enum class data_work_in_period_type;
}


namespace timetron::ui_qt {

	class data_view_urgency;
	class data_view_urgency_task;

	class proc_view {
	  public:
		void fill_timeline_view_blocks(core::data_diagnostic const &, core::data_diagnostic_by_period const &, QGridLayout &);
		void fill_timeline_view_urgency(core::data_diagnostic const &, QVBoxLayout &);

	    void fill_view_urgency(core::data_diagnostic const &, data_view_urgency &);

		std::string create_consumable_string(core::data_diagnostic const &);
		std::string create_relative_weight_percentage_string(float);
		std::string create_task_hour_string(float time_in_minutes);
		std::string create_task_hour_style_string(core::data_work_in_period_type, float time_in_minutes);
		std::string create_task_urgency_string(data_view_urgency_task const &);
		std::string create_task_urgency_tooltip_string(data_view_urgency_task const &);

		std::string to_hex(std::uint8_t);
	};

}
