#include "Setting_Model.h"

Setting_Model::Setting_Model() {}
bool Setting_Model::loadData(short &openHour, short &closeHour, Qt::DayOfWeek &dayOpenRegis,
                             QMap<QString, QPair<short, short>> &roles,
                             short &maxLeaveFT, short &maxDaysPT, short &maxHourPT)
{
    QSqlDatabase db = Database::getInstance()->getDbConnect();
    if (!db.isOpen()) return false;

    QSqlQuery querySys(db);
    QString sqlSys = "SELECT openHour, closeHour, dayOpenRegisShift, "
                     "MaximumLeavePerMonth_FT, MinximunDaysWorkPerWeek_PT, MaximumHourWorkPerDay_PT "
                     "FROM SYSTEM_CONFIG";

    if (querySys.exec(sqlSys)) {
        if (querySys.next()) {
            openHour = querySys.value("openHour").toInt();
            closeHour = querySys.value("closeHour").toInt();
            dayOpenRegis = static_cast<Qt::DayOfWeek>(querySys.value("dayOpenRegisShift").toInt());
            maxLeaveFT = querySys.value("MaximumLeavePerMonth_FT").toInt();

            // Map maxDaysPT của View vào MinximunDaysWorkPerWeek_PT của Database
            maxDaysPT = querySys.value("MinximunDaysWorkPerWeek_PT").toInt();
            maxHourPT = querySys.value("MaximumHourWorkPerDay_PT").toInt();
        }
    } else {
        qDebug() << "Lỗi load SYSTEM_CONFIG:" << querySys.lastError().text();
        return false;
    }

    roles.clear();
    QSqlQuery queryRole(db);
    if (queryRole.exec("SELECT roleName, MinimumNumberOfEmployee, MaximumNumberOfEmployee FROM PolicyForRole_CONFIG")) {
        while (queryRole.next()) {
            QString role = queryRole.value("roleName").toString();
            short minVal = queryRole.value("MinimumNumberOfEmployee").toInt();
            short maxVal = queryRole.value("MaximumNumberOfEmployee").toInt();
            roles[role] = qMakePair(minVal, maxVal);
        }
    } else {
        qDebug() << "Lỗi load PolicyForRole_CONFIG:" << queryRole.lastError().text();
        return false;
    }
    Config::setOpenHour(openHour);
    Config::setCloseHour(closeHour);
    Config::setDayOpenRegisShift(dayOpenRegis);
    Config::setMaximumLeavePerMonth_FT(maxLeaveFT);
    Config::setMinximumDaysWorkPerWeek_PT(maxDaysPT);
    Config::setMaximumHourWorkPerDay_PT(maxHourPT);
    Config::setRoles(roles);
    Config::setGuaranteedDaysPerWeek_FT(7 - maxLeaveFT);
    return true;
}

bool Setting_Model::saveData(short openHour, short closeHour, Qt::DayOfWeek dayOpenRegis,
                             const QMap<QString, QPair<short, short>>& roles,
                             short maxLeaveFT, short maxDaysPT, short maxHourPT)
{
    QSqlDatabase db = Database::getInstance()->getDbConnect();
    if (!db.isOpen()) return false;

    db.transaction();

    QSqlQuery qSys(db);
    qSys.prepare("UPDATE SYSTEM_CONFIG SET "
                 "openHour = :openHour, "
                 "closeHour = :closeHour, "
                 "dayOpenRegisShift = :dayOpen, "
                 "MaximumLeavePerMonth_FT = :maxLeaveFT, "
                 "MinximunDaysWorkPerWeek_PT = :maxDaysPT, "
                 "MaximumHourWorkPerDay_PT = :maxHourPT");

    qSys.bindValue(":openHour", openHour);
    qSys.bindValue(":closeHour", closeHour);
    qSys.bindValue(":dayOpen", static_cast<int>(dayOpenRegis));
    qSys.bindValue(":maxLeaveFT", maxLeaveFT);
    qSys.bindValue(":maxDaysPT", maxDaysPT);
    qSys.bindValue(":maxHourPT", maxHourPT);

    if (!qSys.exec()) {
        qDebug() << "Lỗi cập nhật SYSTEM_CONFIG:" << qSys.lastError().text();
        db.rollback();
        return false;
    }

    QSqlQuery qRole(db);
    qRole.prepare("INSERT OR REPLACE INTO PolicyForRole_CONFIG (roleName, MinimumNumberOfEmployee, MaximumNumberOfEmployee) "
                  "VALUES (:role, :min, :max)");

    for (auto it = roles.constBegin(); it != roles.constEnd(); ++it) {
        qRole.bindValue(":role", it.key());
        qRole.bindValue(":min", it.value().first);
        qRole.bindValue(":max", it.value().second);

        if (!qRole.exec()) {
            qDebug() << "Lỗi cập nhật Role:" << it.key() << "-" << qRole.lastError().text();
            db.rollback();
            return false;
        }
    }
    if (db.commit()) {
        Config::setOpenHour(openHour);
        Config::setCloseHour(closeHour);
        Config::setDayOpenRegisShift(dayOpenRegis);
        Config::setMaximumLeavePerMonth_FT(maxLeaveFT);
        Config::setMinximumDaysWorkPerWeek_PT(maxDaysPT);
        Config::setMaximumHourWorkPerDay_PT(maxHourPT);
        Config::setRoles(roles);
        return true;
    }
    return false;
}