#include "Optimizer.h"
#include "utils/Config.h"
using namespace std;

static constexpr int INF = INT_MAX/2;
static constexpr int WEEKDAY_PENALTY = 20; // uu tien fill cuoi tuan truoc
static constexpr int HOUR_SCALE = 10;      // moi 10p tinh 1 lan, nhan vien it gio hon duoc uu tien


void Optimizer::init(int n){
    m_n=n;
    m_edges.clear();
    m_g.assign(n,{});
}
void Optimizer::addEdge(int u , int v , int cap, int cost){
    m_g[u].push_back(m_edges.size());
    m_edges.push_back({v,cap,cost,0});
    m_g[v].push_back(m_edges.size());
    m_edges.push_back({u,0,-cost,0});
}
bool Optimizer::spfa(int s,int t ,QVector<int>& dist,QVector<int>& prev_v, QVector<int>& prev_e ){
    dist.assign(m_n,INF);
    prev_v.assign(m_n,-1);
    prev_e.assign(m_n,-1);
    QVector<bool>inQ(m_n,false);
    dist[s]=0;
    queue<int>q;
    q.push(s);
    inQ[s]=true;
    while(!q.empty()){
        int u=q.front();
        q.pop();
        inQ[u]=false;
        for (int eid:m_g[u]){
            const Edge&e=m_edges[eid];
            if(e.cap>e.flow&&dist[u]!=INF&&dist[u]+e.cost<dist[e.to]){
                dist[e.to]=dist[u]+e.cost;
                prev_v[e.to]=u;
                prev_e[e.to]=eid;
                if(!inQ[e.to]){
                    q.push(e.to);
                    inQ[e.to]=true;
                }
            }
        }
    }
    return dist[t]<INF;
}

int Optimizer::minCostFlow(int s,int t,int maxFlow,int &outCost){
    outCost=0;
    int flow=0;
    QVector<int>dist,prev_v,prev_e;
    while(flow<maxFlow &&spfa(s,t,dist,prev_v,prev_e)){
        int pushed=maxFlow-flow;
        for(int v=t;v!=s;v=prev_v[v])
            pushed=qMin(pushed,m_edges[prev_e[v]].cap-m_edges[prev_e[v]].flow);
        for ( int v =t;v!=s ; v=prev_v[v]){
            m_edges[prev_e[v]].flow +=pushed;
            m_edges[prev_e[v]^1].flow -=pushed;
        }
        flow+=pushed;
        outCost+=pushed*dist[t];
    }
    return flow;
}

Optimizer::Optimizer(const QVector<Shift*>& shifts,
                     const QMap<User*, int>& userMinutes,
                     const QVector<ExistingAssignment>& existingAssignments)
    : shifts(shifts),
      userMinutes(userMinutes),
      existingAssignments(existingAssignments) {}

User* Optimizer::findUserById(short int id) const {
    return m_userById.value(id, nullptr);
}


bool Optimizer::solve(){
    if(shifts.isEmpty()||userMinutes.isEmpty()){
        warnings << "Khong co du lieu dang ky hoac danh sach dang rong.";
        return false;
    }


    m_userById.clear();
    for (auto it = userMinutes.constBegin(); it != userMinutes.constEnd(); ++it)
        m_userById[it.key()->getIdEmployee()] = it.key();


    QMap<QString, QVector<Shift*>> shiftsByRole;
    for (Shift* s : shifts) {
        User* u = findUserById(s->getEmployeeID());
        if (!u) continue;
        shiftsByRole[u->getRole()].push_back(s);
    }

    QMap<QString, QMap<User*,int>> minutesByRole;
    for (auto it = userMinutes.constBegin(); it != userMinutes.constEnd(); ++it)
        minutesByRole[it.key()->getRole()][it.key()] = it.value();

    bool anyFeasible = false;
    int sumFlow = 0, sumCost = 0;

    for (auto roleIt = shiftsByRole.constBegin(); roleIt != shiftsByRole.constEnd(); ++roleIt) {
        const QString& role = roleIt.key();
        RoleSolveResult r = solveForRole(role, roleIt.value(), minutesByRole.value(role));
        anyFeasible = anyFeasible || r.feasible;
        sumFlow += r.totalFlow;
        sumCost += r.totalCost;
        warnings += r.warnings;
    }

    this->feasible  = anyFeasible;
    this->totalFlow = sumFlow;
    this->totalCost = sumCost;
    return this->feasible;
}


// solveForRole() - logic
//   Phase 1: arrange for full time (5 day)
//   Phase 2: arrange for full time (2 day 6-7) if assign
//   Phase 3: part time


Optimizer::RoleSolveResult Optimizer::solveForRole(const QString& role,
                                                   const QVector<Shift*>& roleShifts,
                                                   const QMap<User*, int>& roleUserMinutes)
{
    RoleSolveResult result;
    if (roleShifts.isEmpty() || roleUserMinutes.isEmpty()) return result;

    // List Employee in Specific Role
    QMap<User*, int> empIdx;
    QVector<User*> empList = roleUserMinutes.keys().toVector();
    for (int i = 0; i < empList.size(); ++i) empIdx[empList[i]] = i;
    const int E = empList.size();

    QVector<Shift*> fixedShifts, partTimeShifts;
    for (Shift* s : roleShifts) {
        User* u = findUserById(s->getEmployeeID());
        if (!u || !empIdx.contains(u)) continue;
        if (u->getIsFixedEmployee()) fixedShifts.push_back(s);
        else partTimeShifts.push_back(s);
    }

    // Init Graph
    const int K = 3; // Morning, afternoon, and evening canonical blocks.
    const QString canonicalRole = Config::canonicalRoleName(role);
    QMap<int, int> existingAssignedCount;
    QMap<QPair<int, QDate>, QVector<QPair<QTime, QTime>>> existingByEmployeeDate;

    auto overlaps = [](const QTime& firstStart, const QTime& firstEnd,
                       const QTime& secondStart, const QTime& secondEnd) {
        return firstStart < secondEnd && firstEnd > secondStart;
    };

    for (const ExistingAssignment& assignment : existingAssignments) {
        if (!assignment.date.isValid() || !assignment.startTime.isValid() ||
            !assignment.endTime.isValid() ||
            assignment.startTime >= assignment.endTime)
            continue;

        existingByEmployeeDate[{assignment.employeeId, assignment.date}]
            .append({assignment.startTime, assignment.endTime});

        if (Config::canonicalRoleName(assignment.role).compare(
                canonicalRole, Qt::CaseInsensitive) != 0)
            continue;

        const int day = assignment.date.dayOfWeek() - 1;
        if (day < 0 || day >= 7)
            continue;
        for (int block = 0; block < K; ++block) {
            if (overlaps(assignment.startTime, assignment.endTime,
                         Config::getShiftStartTime(block),
                         Config::getShiftEndTime(block)))
                ++existingAssignedCount[day * K + block];
        }
    }

    auto overlapsExistingAssignment = [&](int employeeId, const QDate& date,
                                          const QTime& start,
                                          const QTime& end) {
        const auto intervals = existingByEmployeeDate.value({employeeId, date});
        for (const auto& interval : intervals)
            if (overlaps(start, end, interval.first, interval.second))
                return true;
        return false;
    };

    const int S = 0, T = 1;
    auto nEmp  = [&](int i)          { return 2 + i; };
    auto nDay  = [&](int i, int d)   { return 2 + E + i * 7 + d; };
    auto nSlot = [&](int d, int blk) { return 2 + 8 * E + d * K + blk; };
    init(2 + 8 * E + 7 * K);

    for (int d = 0; d < 7; ++d) {
        for (int blk = 0; blk < K; ++blk) {
            const int remainingCapacity = qMax(
                0, Config::getMaxStaffForRole(role) -
                       existingAssignedCount.value(d * K + blk, 0));
            addEdge(nSlot(d, blk), T, remainingCapacity, 0);
        }
    }

    struct AssignEdge {
        Shift* shift;
        int edgeIdx;
        int empIdx;
        int day;
        int blk;
        QTime assignStart, assignEnd;
    };
    QVector<AssignEdge> assignEdges;

    QMap<QPair<int,int>, bool> dayCapOpened;
    auto ensureDayCap = [&](int ei, int d) {
        QPair<int,int> key(ei, d);
        if (!dayCapOpened.contains(key)) {
            addEdge(nEmp(ei), nDay(ei, d), 1, 0);
            dayCapOpened[key] = true;
        }
    };

    auto costOf = [&](User* u, int d) {
        bool isWeekend = (d >= 5);
        return roleUserMinutes.value(u) / HOUR_SCALE + (isWeekend ? 0 : WEEKDAY_PENALTY);
    };
    auto blockOfFixed = [&](const QTime& start) -> int {
        if(start<QTime(12,0)) return 0; // ca sang
        if(start<QTime(17,0)) return 1; // ca chieu
        return 2; // ca toi
    };


    QMap<int,int> registeredDaysFixed;
    for (Shift* s : fixedShifts) {
        User* u = findUserById(s->getEmployeeID());
        int ei = empIdx[u];
        int d   = s->getDate().dayOfWeek() - 1;
        if (d < 0 || d >= 7 ||
            overlapsExistingAssignment(s->getEmployeeID(), s->getDate(),
                                       s->getStartTime(), s->getEndTime()))
            continue;

        registeredDaysFixed[ei] = registeredDaysFixed.value(ei, 0) + 1;
        int blk = blockOfFixed(s->getStartTime());
        ensureDayCap(ei, d);

        int cost = costOf(u, d);
        int eid = m_edges.size();
        addEdge(nDay(ei, d), nSlot(d, blk), 1, cost);
        assignEdges.push_back({s, eid, ei, d, blk, s->getStartTime(), s->getEndTime()});
    }

    // --- PHASE 1
    const int floorDays = Config::getGuaranteedDaysPerWeek_FT();
    for (int i = 0; i < E; ++i) {
        if (!empList[i]->getIsFixedEmployee()) continue;
        int cap = qMin((int)floorDays, registeredDaysFixed.value(i, 0));
        if (cap > 0) addEdge(S, nEmp(i), cap, 0);
    }

    // --- PHASE 2
    for (int i = 0; i < E; ++i) {
        if (!empList[i]->getIsFixedEmployee()) continue;
        int total = registeredDaysFixed.value(i, 0);
        int floorCap = qMin((int)floorDays, total);
        int extra = total - floorCap;
        if (extra > 0) addEdge(S, nEmp(i), extra, 10000);
    }

// ---PHASE 3
    const int minMinutesPT = Config::getMinimumHourWorkPerDay_PT() * 60;
    const int maxMinutesPT = Config::getMaximumHourWorkPerDay_PT() * 60;

    QMap<int,int> registeredDaysPT;
    for (Shift* s : partTimeShifts) {
        User* u = findUserById(s->getEmployeeID());
        int ei = empIdx[u];
        int d  = s->getDate().dayOfWeek() - 1;
        bool coCandidate = false;

        for (int blk = 0; blk < K; ++blk) {
            QTime blkStart = Config::getShiftStartTime(blk);
            QTime blkEnd = Config::getShiftEndTime(blk);
            QTime ovStart  = qMax(s->getStartTime(), blkStart);
            QTime ovEnd    = qMin(s->getEndTime(),   blkEnd);
            if (ovStart >= ovEnd) continue;

            int ovMinutes = ovStart.secsTo(ovEnd) / 60;
            if (ovMinutes < minMinutesPT) continue;

            int usedMinutes = qMin(ovMinutes, maxMinutesPT);
            QTime realEnd = ovStart.addSecs(usedMinutes * 60);
            if (overlapsExistingAssignment(s->getEmployeeID(), s->getDate(),
                                           ovStart, realEnd))
                continue;

            int cost = costOf(u, d);
            int eid = m_edges.size();
            addEdge(nDay(ei, d), nSlot(d, blk), 1, cost);
            assignEdges.push_back({s, eid, ei, d, blk, ovStart, realEnd});
            coCandidate = true;
        }

        if (coCandidate) {
            ensureDayCap(ei, d);
            registeredDaysPT[ei] = registeredDaysPT.value(ei, 0) + 1;
        }
    }
    for (int i = 0; i < E; ++i) {
        if (empList[i]->getIsFixedEmployee()) continue;
        int cap = registeredDaysPT.value(i, 0);
        if (cap > 0) addEdge(S, nEmp(i), cap, 20000);
    }
    
    // RUN OPTIMIZATION ONCE
    int totalGraphCost = 0;
    minCostFlow(S, T, INF, totalGraphCost);

    QMap<int,int> assignedCount;
    QMap<int, QSet<int>> empDaysAssigned;
    QSet<Shift*> assignedShifts;

    for (auto& ae : assignEdges) {
        bool assigned = (m_edges[ae.edgeIdx].flow == 1);
        if (assigned) {
            assignedShifts.insert(ae.shift);
            ae.shift->setAssignedTime(ae.assignStart, ae.assignEnd);
            assignedCount[ae.day * K + ae.blk] = assignedCount.value(ae.day * K + ae.blk, 0) + 1;
            empDaysAssigned[ae.empIdx].insert(ae.day);
        }
    }

    for (Shift *shift : roleShifts)
        if (shift)
            shift->setStatus(assignedShifts.contains(shift) ? 1 : -1);

    int flowSum = 0;
    for (auto v : assignedCount) flowSum += v;
    result.totalFlow = flowSum;
    result.totalCost = totalGraphCost;
    result.feasible  = flowSum > 0;

    // insufficient staff
    static const QStringList BLOCK_NAMES = {"Sáng","Chiều", "Tối"};
    for (int d = 0; d < 7; ++d) {
        for (int blk = 0; blk < K; ++blk) {
            int cnt = existingAssignedCount.value(d * K + blk, 0) +
                      assignedCount.value(d * K + blk, 0);
            int minNeeded = Config::getMinStaffForRole(role);
            if (cnt == 0) {
                result.warnings << QString("[%1] Ngày %2 - Ca %3: không có nhân viên nào được xếp.")
                                       .arg(role).arg(d + 1).arg(BLOCK_NAMES[blk]);
            } else if (cnt < minNeeded) {
                result.warnings << QString("[%1] Ngày %2 - Ca %3: chỉ xếp được %4/%5 người.")
                                       .arg(role).arg(d + 1).arg(BLOCK_NAMES[blk]).arg(cnt).arg(minNeeded);
            }
        }
    }

    // ERROR ASSIGN
    for (int i = 0; i < E; ++i) {
        if (!empList[i]->getIsFixedEmployee()) continue;
        int target = qMin((int)floorDays, registeredDaysFixed.value(i, 0));
        int days = empDaysAssigned.value(i).size();
        if (days < target) {
            result.warnings << QString("[%1] Nhân viên ID %2: chỉ được xếp %3/%4 ngày sàn quy định.")
                                   .arg(role).arg(empList[i]->getIdEmployee()).arg(days).arg(target);
        }
    }

    // PART TIME ERROR
    for (int i = 0; i < E; ++i) {
        if (empList[i]->getIsFixedEmployee()) continue;
        int days = empDaysAssigned.value(i).size();
        if (days > 0 && days < Config::getMinimumDaysWorkPerWeek_PT()) {
            result.warnings << QString("[%1] Nhân viên ID %2 (Part-time): chỉ được xếp %3/%4 ngày tối thiểu.")
                                   .arg(role).arg(empList[i]->getIdEmployee()).arg(days).arg(Config::getMinimumDaysWorkPerWeek_PT());
        }
    }

    return result;
}
