#ifndef MAIN_CONTROL_H
#define MAIN_CONTROL_H



#include <QObject>

class Main_View;

class Main_Control : public QObject
{
    Q_OBJECT
public:
    explicit Main_Control(QObject *parent = nullptr);
    ~Main_Control();

    void init();

private:
    Main_View* view = nullptr;
};

#endif // MAIN_CONTROL_H