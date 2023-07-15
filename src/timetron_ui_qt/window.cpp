#include "timetron_ui_qt/window.h"

#include "timetron_core/data_timeline.h"
#include "timetron_core/proc_timeline.h"
#include "timetron_core/data_diagnostic.h"
#include "timetron_core/proc_diagnose.h"
#include "timetron_ui_qt/proc_view.h"
#include "version/git_version.h"

#include <tinyxml2.h>

#include <qfiledialog>
#include <qfilesystemwatcher>
#include <qmessagebox>
#include <qsettings>
#include <qtabwidget>
#include <qtimer>

#include <iostream>
#include <sstream>
#include <thread>


using namespace timetron::ui_qt;


main_window::main_window()
{
    ui_window.setupUi(this);
    
    this->upon_open();
}


main_window::~main_window()
{
}


void main_window::upon_open()
{
    auto &settings = this->get_qsettings();
    this->restoreGeometry(settings.value("geometry").toByteArray());
    std::string const main_file_path = settings.value("main_file_path").toString().toStdString();
    
    // Hide the scrollbar
    this->setStyleSheet("QScrollBar:vertical {width: 0px;}");

    this->recreate_timer.reset(new QTimer());
    connect(
      this->recreate_timer.get(), &QTimer::timeout,
      this,                       &main_window::event_timer
    );
    
    if (main_file_path.size() > 0)
      this->perform_file_load(main_file_path);
}


void main_window::upon_close()
{
    std::cout << "main_window: Closing - file path is <" << current_file_path << ">" << std::endl;

    auto &settings = this->get_qsettings();
    settings.setValue("geometry", saveGeometry());
    settings.setValue("main_file_path", QString::fromStdString(current_file_path));
}


void main_window::upon_diag_open()
{
    std::cout << "main_window: File open dialogue" << std::endl;

    QString const in_path_q = QFileDialog::getOpenFileName(this, tr("Open tron"), "", tr("Timetron file (*.tt.xml)"));
    if (in_path_q.length() == 0)
      return;

    std::string const in_path = in_path_q.toStdString();
    if (in_path == current_file_path)
      return;

    this->perform_file_load(in_path.c_str());
}


void main_window::upon_about()
{
    std::cout << "main_window: Displaying main window about" << std::endl;

    QDialog *about = new QDialog();

    ui_version.setupUi(about);
    ui_version.version_main->setText(QString("Timetron\n") + gaos::version::get_git_essential_version());
    ui_version.version_compile->setText(gaos::version::get_compile_stamp());
    ui_version.version_git->setText(gaos::version::get_git_history());

    about->setWindowModality(Qt::WindowModality::ApplicationModal);
    about->show();
}


void main_window::perform_file_load(std::string const &path)
{
    current_file_path = path;
    
    std::cout << "main_window: Deserialising xml from <" << current_file_path << ">" << std::endl;

    core::proc_timeline proc{};
    switch (proc.deserialise_from_xml_safe(this->timeline, current_file_path))
    {
      case core::proc_timeline::deserialise_result::ok:
        std::cout << "main_window: Ok" << std::endl;
        break;
      case core::proc_timeline::deserialise_result::file_could_not_be_read:
        std::cout << "main_window: Could not read file!" << std::endl;
        QMessageBox msgBox;
        msgBox.setText(QString("The file could not be read."));
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setDefaultButton(QMessageBox::Ok);
        msgBox.exec();
        current_file_path = "";
        return;
    }
    
    std::cout << "main_window: Loaded " << timeline.day_data.size() << " days" << std::endl;

    this->perform_update_file_watch();
    this->perform_update_diagnostics();
}


void main_window::perform_update_file_watch()
{
    std::cout << "main_window: Updating file watch" << std::endl;

    file_watcher.reset(new QFileSystemWatcher(this));

    file_watcher->addPath(QString::fromStdString(current_file_path));

    connect(
      file_watcher.get(), &QFileSystemWatcher::fileChanged,
      this,               &main_window::event_file_changed
    );
}


void main_window::perform_update_diagnostics()
{
    std::cout << "main_window: Updating diagnostics" << std::endl;
    
    core::proc_diagnose proc_diagnose{};
    proc_diagnose.fill_diagnostic(this->timeline, this->diagnostic);
    
    std::cout << "main_window: Created diagnostic with " << diagnostic.periods.size() << " periods" << std::endl;

    perform_update_timeline_view();
    
    this->recreate_timer->start(10000);
}


void main_window::perform_update_timeline_view()
{
    std::cout << "main_window: Update timeline view" << std::endl;

    this->perform_clear_timeline_view();

    core::proc_diagnose proc_diagnose{};
    core::data_diagnostic_by_period diagnostic_by_period;
    proc_diagnose.fill_diagnostic_organised(this->diagnostic, diagnostic_by_period);
    
    ui_qt::proc_view proc_view{};
    proc_view.fill_timeline_view_blocks(this->diagnostic, diagnostic_by_period, *this->get_ui_current_projects());
    proc_view.fill_timeline_view_urgency(this->diagnostic, *this->get_ui_current_urgency());
}


void main_window::perform_clear_timeline_view()
{
    auto &ui_current_projects  = *this->get_ui_current_projects();
    auto &ui_current_effective = *this->get_ui_current_urgency();

    while (auto item = ui_current_projects.takeAt(0)) {
        QWidget *widget;
        if (widget = item->widget())
          widget->deleteLater();
        delete item;
    }
    while (auto item = ui_current_effective.takeAt(0)) {
        QWidget *widget;
        if (widget = item->widget())
          widget->deleteLater();
        delete item;
    }
}


QSettings& main_window::get_qsettings()
{
    static QSettings timetron_settings("Stout", "Timetron");
    return timetron_settings;
}


QGridLayout * main_window::get_ui_current_projects()
{
    auto widget = this->ui_window.centralWidget->topLevelWidget()->findChild<QWidget*>(QString("current_projects"));

    if (!widget)
      return nullptr;

    auto layout = qobject_cast<QGridLayout*>(widget->layout());
    return layout;
}


QVBoxLayout * main_window::get_ui_current_urgency()
{
    auto widget = this->ui_window.centralWidget->topLevelWidget()->findChild<QWidget*>(QString("current_state_short"));

    if (!widget)
      return nullptr;

    auto layout = qobject_cast<QVBoxLayout*>(widget->layout());
    return layout;
}


void main_window::uii_file_open()
{
    this->upon_diag_open();
}


void main_window::uii_about()
{
    this->upon_about();
}


void main_window::event_file_changed(QString const &file)
{
    std::cout << "main_window: File changed" << std::endl;

    if (this->current_file_path == file.toStdString())
      this->perform_file_load(this->current_file_path);
}


void main_window::event_timer()
{
    std::cout << "main_window: Timer" << std::endl;

    this->perform_update_diagnostics();
}


void main_window::closeEvent(QCloseEvent *event)
{
    this->upon_close();
    QWidget::closeEvent(event);
}