#ifndef OVERVIEW_CONTROL_H
#define OVERVIEW_CONTROL_H

#include <QObject>

class Overview_View;

class Overview_Control : public QObject
{
    Q_OBJECT
public:
    explicit Overview_Control(QObject *parent = nullptr);
    ~Overview_Control();

    void init();
    Overview_View* getView() const;

private:
    Overview_View* view;
};

#endif