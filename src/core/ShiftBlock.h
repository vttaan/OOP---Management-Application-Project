#pragma once

#include "core/User.h"
#include "utils/Config.h"
#include <QList>
#include <QDate>
#include <QTime>
#include <QString>

enum class ShiftStatus {
    Empty,
    Understaffed,
    Sufficient
};

class ShiftBlock {
private:
    QDate date;
    QTime startTime;
    QTime endTime;
    QString role;
    QList<User*> employees;

public:
    // A canonical manager/staff grid represents the whole shift, not only the
    // Manager role.  An empty role therefore uses the aggregate requirement;
    // callers may still pass a role for role-specific blocks.
    ShiftBlock(QDate d, QTime s, QTime e, QString role = QString())
        : date(d), startTime(s), endTime(e), role(role) {}

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
    QString getRole() const { return role; }

    // 1. Employee List
    QList<User*> getEmployees() const { return employees; }
    bool isEmpty() const { return employees.isEmpty(); }

    // 2. Count
    int getStaffCount() const { return employees.size(); }

    // 3. Status (Logic centralized here)
    ShiftStatus getStatus() const {
        if (employees.isEmpty()) return ShiftStatus::Empty;
        int required = 0;
        if (role.isEmpty())
        {
            for (const QString &configuredRole : Config::getAllRoles())
                required += Config::getMinStaffForRole(configuredRole);
        }
        else
        {
            required = Config::getMinStaffForRole(role);
        }
        if (employees.size() < required) return ShiftStatus::Understaffed;
        return ShiftStatus::Sufficient;
    }

    // 4. View Text
    QString getDisplayText() const {
        if (getStatus() == ShiftStatus::Empty) return "Không đạt";
        return QString("%1 nhân viên").arg(getStaffCount());
    }

    // 5. Time Text
    QString getTimeString() const {
        return startTime.toString("HH:mm") + " - " + endTime.toString("HH:mm");
    }
};
