#include "timetron_ui_qt/proc_view.h"

#include "timetron_core/data_diagnostic.h"
#include "timetron_core/proc_diagnose.h"
#include "timetron_ui_qt/data_view.h"

#include <qgridlayout>
#include <qlabel>
#include <qvboxlayout>

#include <iomanip>
#include <sstream>


using namespace timetron::ui_qt;


void proc_view::fill_timeline_view_blocks(core::data_diagnostic const           &diagnostic,
                                          core::data_diagnostic_by_period const &by_period,
                                          QGridLayout                           &layout      )
{
    timetron::core::proc_diagnose diagnose{};

    int current_row  = 0;
    int period_count = (int)diagnostic.periods.size();

    // Create head of table
    {
        std::string consumabe_string = this->create_consumable_string(diagnostic);
    
        if (consumabe_string.size() > 0) {
            QLabel *name_lbl = new QLabel();
            name_lbl->setText(QString::fromStdString(consumabe_string));
            name_lbl->setStyleSheet("QLabel { font-size : 10px; color : #acadac; }");
            name_lbl->setAlignment(Qt::AlignCenter);
            layout.addWidget(name_lbl, current_row, 0, 1, 2);
        }
    
        for (int i = 0; i < period_count; ++i) {
            auto &period = diagnostic.periods[i];

            std::stringstream ss;
            ss << diagnostic.periods[i].name;

            if (period.full_minutes >= 180.f)
              ss << "<br/><span style='font-size: 6px'>" << std::fixed << std::setprecision(0) << (period.full_minutes / 60.f) << "</span>";

            QLabel *name_lbl = new QLabel();
            name_lbl->setText(QString::fromStdString(ss.str()));
            name_lbl->setMinimumWidth(i <= 4 ? 18 : 16);
            name_lbl->setStyleSheet("QLabel { font-size : 8px; color : #acadac; }");
            name_lbl->setAlignment(Qt::AlignCenter);
            layout.addWidget(name_lbl, current_row, 2+i);
        }

        ++current_row;
    }
    
    // Create rows of table
    for (auto &task : by_period.tasks_by_period) {
        if (task.relative_weight > 0.f) {
            std::string perc_string = create_relative_weight_percentage_string(task.relative_weight);
            
            QLabel *percentage = new QLabel();
            percentage->setText(QString::fromStdString(perc_string));
            percentage->setStyleSheet("QLabel { font-size : 6px; color : #00a0a0; }");
            layout.addWidget(percentage, current_row, 0, Qt::AlignCenter);
        }

        QLabel *name = new QLabel();
        name->setText(QString::fromStdString(task.name));
        name->setStyleSheet("QLabel { font-size : 9px; color: #000000 }");
        layout.addWidget(name, current_row, 1);
        
        for (int i = 0; i < period_count; ++i) {
            if (task.min_diag_period > i)
              continue;
            if (task.max_diag_period < i)
              continue;
            
            float minutes = diagnose.get_minutes_in_period(diagnostic.periods[i], task.id);
            if (minutes == 0.f)
              continue;

            std::string hour_str   = this->create_task_hour_string(minutes);
            std::string hour_style = this->create_task_hour_style_string(diagnostic.periods[i].period_type, minutes);
        
            QLabel *hour_lbl = new QLabel();
            hour_lbl->setText(QString::fromStdString(hour_str));
            hour_lbl->setStyleSheet(QString::fromStdString(hour_style));

            layout.addWidget(hour_lbl, current_row, 2+i, Qt::AlignCenter);
        }
        
        ++current_row;
    }

    layout.addItem(new QSpacerItem(20,10, QSizePolicy::Expanding, QSizePolicy::Expanding), current_row, 2 + (int)diagnostic.periods.size());
}


void proc_view::fill_timeline_view_urgency(core::data_diagnostic const &diagnostic, QVBoxLayout &layout)
{
    data_view_urgency urgency;
    this->fill_view_urgency(diagnostic, urgency);

    int         prev_emoji_count     = -1;
    std::size_t elements_seen        = 0;
    bool        elements_seen_over_5 = false;

    for (auto const &elem : urgency.tasks) {
        elements_seen_over_5 |= ++elements_seen == 6;
        
        int margin = 0;
        
        // Determine if we want to leave a space in-between elements -
        // basically we just check if the emoji count is different; or
        // whether we switched to-or-from having peach emojis
        int const emoji_count_sum = elem.emoji_clover_count + elem.emoji_water_count + (elem.emoji_peach_count * 100) + elem.emoji_star_count;
        if (prev_emoji_count != -1 && emoji_count_sum != prev_emoji_count)
        {
            if (elem.emoji_peach_count == 1 || elements_seen_over_5)
              margin = 100;
            else
              margin = 7;
        
            elements_seen_over_5 = false;
        }
        prev_emoji_count = emoji_count_sum;
        
        std::string task_str         = this->create_task_urgency_string(elem);
        std::string task_tooltip_str = this->create_task_urgency_tooltip_string(elem);

        QLabel *label = new QLabel();
        label->setText(QString::fromStdString(task_str));
        label->setContentsMargins(0, margin, 0, 0);
        label->setToolTip(QString::fromStdString(task_tooltip_str));
        
        layout.addWidget(label);
    }
    
    layout.addSpacerItem(new QSpacerItem(0,10, QSizePolicy::Expanding, QSizePolicy::Expanding));
}


void proc_view::fill_view_urgency(core::data_diagnostic const &diagnostic, data_view_urgency &urgency)
{
    urgency.clear();

    for (auto const &elem : diagnostic.current_tasks) {
        auto const &task = elem.second;

        if (task.relative_weight == 0.f)
          continue;
          
        constexpr float factor_adjustment = 60.f;
        
        float const adjusted_factor         = (task.minute_progress_factor + task.minute_absence_penalty) / factor_adjustment;
        float const adjusted_absence_factor = task.minute_absence_penalty / factor_adjustment;
        float const unadjusted_factor       = task.minute_progress_factor / factor_adjustment;
        float const adjusted_factor_abs     = std::abs(adjusted_factor);
        
        int emoji_count      = 1;
        int unadjusted_count = 0;
        int penalty_count    = 0;
        
        if (adjusted_factor_abs >= 1.f) {
            emoji_count += int(std::log2(1.f + adjusted_factor_abs) * 3.f);
        }
        if (unadjusted_factor < 0.f) {
            unadjusted_count = 1;
            if (unadjusted_factor < -1.f)
              unadjusted_count += int(std::log2(1.f + -unadjusted_factor) * 3.f);
        }
        if (adjusted_absence_factor < 0.f) {
            penalty_count = 1 + int(std::log2(1.f + -adjusted_absence_factor) * 3.f);
        }

        data_view_urgency::task output_elem;
        output_elem.emoji_water_count  = 0;
        output_elem.emoji_clover_count = 0;
        output_elem.emoji_peach_count  = 0;
        output_elem.emoji_star_count   = 0;
        output_elem.name               = task.name;
        output_elem.abs_progress       = task.minute_progress_factor / 60.f;
        output_elem.full_progress      = (task.minute_progress_factor + task.minute_absence_penalty) / 60.f;
        output_elem.penalty            = task.minute_absence_penalty / 60.f;

        int const display_emoji_count   = emoji_count - 6;
        int const display_penalty_count = penalty_count - 6;

        if (display_emoji_count <= 0)
        {
            output_elem.emoji_peach_count = 1;
        }
        else if (adjusted_factor > 0.f)
        {
            output_elem.emoji_star_count  = display_emoji_count;
        }
        else
        {
            output_elem.emoji_clover_count = std::max(adjusted_absence_factor < 0.f ? 1 : 0, std::min(display_penalty_count, display_emoji_count));
            output_elem.emoji_water_count  = display_emoji_count - output_elem.emoji_clover_count;
        }
      
        urgency.tasks.emplace_back(output_elem);
    }

    std::sort(
      urgency.tasks.begin(), urgency.tasks.end(),
      [](data_view_urgency::task const &lh, data_view_urgency::task const &rh) -> bool {
        // If there are more clover and water emojis, always sort lower
        if (lh.emoji_clover_count + lh.emoji_water_count != rh.emoji_clover_count + rh.emoji_water_count)
          return lh.emoji_clover_count + lh.emoji_water_count > rh.emoji_clover_count + rh.emoji_water_count;
          
        // If there are more clovers, always sort lower
        if (lh.emoji_clover_count != rh.emoji_clover_count)
          return lh.emoji_clover_count > rh.emoji_clover_count;
          
        // If there are more water emojis, always sort lower
        if (lh.emoji_water_count != rh.emoji_water_count)
          return lh.emoji_water_count > rh.emoji_water_count;

        // If there are more star emojis, always sort higher
        if (lh.emoji_star_count != rh.emoji_star_count)
          return lh.emoji_star_count < rh.emoji_star_count;

        // If there are more peach emojis, always sort higher
        if (lh.emoji_peach_count != rh.emoji_peach_count)
          return lh.emoji_peach_count < rh.emoji_peach_count;

        // Sort by name
        return lh.name < rh.name;
    });
}


std::string proc_view::create_consumable_string(core::data_diagnostic const &diagnostic)
{
    std::stringstream debt_ss;
    debt_ss << std::fixed << std::setprecision(2);

    float water_fac = std::log2(std::max(1e-5f, diagnostic.water_fac * 2.f));
    float jimmy_fac = std::log2(std::max(1e-5f, diagnostic.jimmy_fac * 2.f));

    if (water_fac <= .3f)
      debt_ss << u8"<span style='color: #000000; font-weight: bold'>" << (100.f * water_fac) << "%</span>";
    else if (water_fac <= .6f)
      debt_ss << u8"<span style='color: #a0a0a0'>" << (100.f * water_fac) << "%</span>";
    else
      debt_ss << "<span style='color: #aedddc'>" << (100.f * water_fac) << "%</span>";

    debt_ss << "  ";

    if (jimmy_fac <= .3f)
      debt_ss << u8"<span style='color: #000000; font-weight: bold'>" << (100.f * jimmy_fac) << "%</span>";
    else if (jimmy_fac <= .6f)
      debt_ss << u8"<span style='color: #a0a0a0'>" << (100.f * jimmy_fac) << "%</span>";
    else
      debt_ss << "<span style='color: #aedddc'>" << (100.f * jimmy_fac) << "%</span>";

    return debt_ss.str();
}


std::string proc_view::create_relative_weight_percentage_string(float fraction)
{
    std::stringstream perc;
    perc << std::fixed << std::showpoint << std::setprecision(1) << (std::floor(fraction * 200.f) / 2.f) << "% ";

    return perc.str();
}


std::string proc_view::create_task_hour_string(float time_in_minutes)
{
    std::stringstream hour;
    hour << std::fixed << std::showpoint << std::setprecision(1) << (time_in_minutes / 60.f) ;

    // Remove leading (symbol) zero for fractions
    std::string hour_str = hour.str();
    if (hour_str[0] == '0')
      hour_str.erase(0, 1);

    return hour.str();
}


std::string proc_view::create_task_hour_style_string(core::data_work_in_period_type type, float time_in_minutes)
{
    float const base_fac = std::min( 1.f,  time_in_minutes / 60.f);
    float const fac_log2 = std::log2(1.f + time_in_minutes / 120.f);

    std::string const colour_a = this->to_hex(std::uint8_t(225.f * std::min(1.f, std::pow(.25f, fac_log2))));
    std::string const colour_b = this->to_hex(std::uint8_t(255.f * std::min(1.f, std::pow(.40f, fac_log2))));
    std::string const colour_c = this->to_hex(std::uint8_t(255.f * std::min(1.f, std::pow(.50f, fac_log2))));

    std::string font_colour    = "000000";
    float       font_size_base = 12.0f;
    float       font_size_bfac =  1.0f;
    float       font_size_lfac = 60.0f;
    switch (type)
    {
      case core::data_work_in_period_type::day:
      case core::data_work_in_period_type::week_1:
        font_colour    = colour_a + colour_a + colour_a;
        font_size_base =   6.0f;
        font_size_bfac =   2.0f;
        font_size_lfac =  60.0f;
        break;
      case core::data_work_in_period_type::week_4:
        font_colour    = colour_a + colour_b + colour_c;
        font_size_base =   5.5f;
        font_size_bfac =   1.0f;
        font_size_lfac =  60.0f;
        break;
      case core::data_work_in_period_type::year:
        font_colour    = colour_c + colour_a + colour_b;
        font_size_base =   5.0f;
        font_size_bfac =   1.0f;
        font_size_lfac = 120.0f;
        break;
    }

    std::string font_size = std::to_string(
      int(font_size_base + (font_size_bfac * base_fac)
                         + std::log2(1.f + time_in_minutes / font_size_lfac)
    ));

    std::stringstream style;
    style << "QLabel { font-size : " << font_size << "px; color : #" << font_colour << " }";

    return style.str();
}


std::string proc_view::create_task_urgency_string(data_view_urgency_task const &task)
{
    std::stringstream ss;
        
    ss << task.name << " ";
        
    for (int i = 0; i < task.emoji_clover_count; ++i)
        ss << u8"🍀";
    for (int i = 0; i < task.emoji_water_count; ++i)
        ss << u8"🌊";
    for (int i = 0; i < task.emoji_peach_count; ++i)
        ss << u8"🍑";
    for (int i = 0; i < task.emoji_star_count; ++i)
        ss << u8"⭐";

    return ss.str();
}


std::string proc_view::create_task_urgency_tooltip_string(data_view_urgency_task const &task)
{
    std::stringstream ss;

    ss << std::fixed << std::setprecision(1);
    ss << task.name << ((task.full_progress >= 0.05f) ? " +" : " ") << task.full_progress << "h";
    if (task.penalty < -.1f || task.emoji_clover_count > 0) {
      ss << "\n    (";
      ss << ((task.abs_progress >= 0.05f) ? " +" : " ") << task.abs_progress << "h ";
      ss << ((task.penalty >= 0.05f) ?      " +" : " ") << task.penalty << "h";
      ss << ")";
    }

    return ss.str();
}


std::string proc_view::to_hex(std::uint8_t i)
{
    std::uint8_t a = i / 16;
    std::uint8_t b = i % 16;
    
    char ret[3] = {
      a < 10 ? '0' + a : 'A' + (a-10),
      b < 10 ? '0' + b : 'A' + (b-10),
      0
    };

    return ret;
}