#pragma once

#include "timetron_ui_qt/ui_window.h"
#include "timetron_ui_qt/ui_version.h"
#include <qmainwindow>

class QFileSystemWatcher;
class QSettings;
class QTabWidget;


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
        
        // Qt
        QSettings&  get_qsettings();

      signals:

      public slots:
        void uii_about();
        void uii_file_open();

      private:
        std::string         current_file_path;

        Ui::timetron_window ui_window;
        Ui::version         ui_version;
    };

}