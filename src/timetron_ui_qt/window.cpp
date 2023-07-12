#include "timetron_ui_qt/window.h"
#include "version/git_version.h"

#include <qfilesystemwatcher>
#include <qfiledialog>
#include <qmessagebox>
#include <qsettings>
#include <qtabwidget>

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
    
    if (main_file_path.size() > 0) {
        // ...
    }
}


void main_window::upon_close()
{
    auto &settings = this->get_qsettings();
    settings.setValue("geometry", saveGeometry());
    settings.setValue("main_file_path", QString::fromStdString(current_file_path));
}


void main_window::upon_diag_open()
{
    std::cout << "main_window: File open dialogue" << std::endl;

    QString const in_path_q = QFileDialog::getOpenFileName(this, tr("Open tron"), "", tr("Extron file (*.xt.xml)"));
    if (in_path_q.length() == 0)
      return;

    std::string const in_path = in_path_q.toStdString();
    if (in_path == current_file_path)
      return;
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


QSettings& main_window::get_qsettings()
{
    static QSettings timetron_settings("Stout", "Timetron");
    return timetron_settings;
}


void main_window::uii_file_open()
{
    this->upon_diag_open();
}


void main_window::uii_about()
{
    this->upon_about();
}