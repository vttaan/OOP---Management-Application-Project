#ifndef MAIN_CONTROL_H
#define MAIN_CONTROL_H

// ---------------------------------------------------------------
// NOTE: This file is a legacy stub kept for backward compatibility
// after the merge that renamed Main_View → Dashboard_View and
// introduced the Control_Navigator / View_Navigator architecture.
//
// The old Main_Control class referenced "view/main_view.h" which
// no longer exists, causing "invalid use of class View_Navigator"
// and similar compile errors.
//
// All functionality previously in Main_Control has been moved to:
//   - Control_Navigator   (src/control/Control_Navigator.h)
//   - Dashboard_Control   (src/control/Dashboard_Control.h)
//
// This stub prevents include-path errors without removing the file.
// ---------------------------------------------------------------

#include <QObject>

// Forward-declare (not include) to avoid broken header chain
class Main_View;

class Main_Control : public QObject
{
    Q_OBJECT
public:
    explicit Main_Control(QObject *parent = nullptr) : QObject(parent) {}
    ~Main_Control() {}

    void init() {} // no-op: use Control_Navigator instead

private:
    Main_View* view = nullptr; // kept to avoid redefinition errors
};

#endif // MAIN_CONTROL_H