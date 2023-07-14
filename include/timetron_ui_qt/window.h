#pragma once

#include "timetron_ui_qt/ui_window.h"
#include "timetron_ui_qt/ui_version.h"
#include "timetron_core/data_timeline.h"
#include <qmainwindow>

class QFileSystemWatcher;
class QSettings;
class QTabWidget;
class QFileSystemWatcher;


namespace timetron::ui_qt {

    // Empty window
    class main_window : public QMainWindow
    {
        Q_OBJECT

      public:
        main_window();
        ~main_window();
        
      private:
        // situations
        void upon_open();
        void upon_close();
        void upon_diag_open();
        void upon_about();

        // actions
        void perform_file_load(std::string const &);
        void perform_update_file_watch();
        void perform_update_diagnostics();

        // Qt
        QSettings&  get_qsettings();

      signals:

      public slots:
        void uii_about();
        void uii_file_open();
        void event_file_changed(QString const &file);
        void event_timer();

      public:
        void closeEvent(QCloseEvent *event) override;

      private:
        core::data_timeline timeline;

        std::string         current_file_path;

        Ui::timetron_window ui_window;
        Ui::version         ui_version;
        
        std::unique_ptr<QFileSystemWatcher> file_watcher;
        std::unique_ptr<QTimer>             recreate_timer;
    };

}