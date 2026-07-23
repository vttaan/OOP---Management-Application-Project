#pragma once

#include "core/User.h"
#include "utils/Config.h"
#include <QList>
#include <QDate>
#include <QTime>
#include <QString>

enum class ShiftStatus {
    Empty,          // Trống
    Understaffed,   // Thiếu người
    Sufficient      // Đủ người
};

class ShiftBlock {
private:
    QDate date;
    QTime startTime;
    QTime endTime;
    QList<User*> employees; // Polymorphic list of Staff / Manager

public:
    ShiftBlock(QDate d, QTime s, QTime e) : date(d), startTime(s), endTime(e) {}
    
    // Memory management: The ShiftBlock DOES NOT own the User objects
    // They are owned and managed by Schedule_Model
    ~ShiftBlock() {
        employees.clear();
    }

    void addStaff(User* u) { employees.append(u); }

    // Getters
    QDate getDate() const { return date; }
    QTime getStartTime() const { return startTime; }
    QTime getEndTime() const { return endTime; }

    // 1. Employee List
    QList<User*> getEmployees() const { return employees; }
    bool isEmpty() const { return employees.isEmpty(); }

    // 2. Count
    int getStaffCount() const { return employees.size(); }

    // 3. Status (Logic centralized here)
    ShiftStatus getStatus() const {
        if (employees.isEmpty()) return ShiftStatus::Empty;
        if (employees.size() < Config::getMinStaffPerShift()) return ShiftStatus::Understaffed;
        return ShiftStatus::Sufficient;
    }

    // 4. View Text
    QString getDisplayText() const {
        if (getStatus() == ShiftStatus::Empty) return "Trống";
        return QString("%1 nhân viên").arg(getStaffCount());
    }

    // 5. Time Text
    QString getTimeString() const {
        return startTime.toString("HH:mm") + " - " + endTime.toString("HH:mm");
    }
};
