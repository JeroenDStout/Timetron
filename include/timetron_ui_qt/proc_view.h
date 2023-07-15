#pragma once

#include <string>

class QGridLayout;
class QLabel;


namespace timetron::core {
	class data_diagnostic;
	class data_diagnostic_by_period;
	enum class data_work_in_period_type;
}


namespace timetron::ui_qt {

	class proc_view {
	  public:
		void		fill_timeline_view_blocks(core::data_diagnostic const &, core::data_diagnostic_by_period const &, QGridLayout &);

		std::string create_consumable_string(core::data_diagnostic const &);
		std::string create_relative_weight_percentage_string(float);
		std::string create_task_hour_string(float time_in_minutes);
		std::string create_task_hour_style_string(core::data_work_in_period_type, float time_in_minutes);

		std::string to_hex(std::uint8_t);
	};

}
