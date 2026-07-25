#include "Optimizer.h"
#include<QMap>
#include<QPair>
#include<QSet>
#include<queue>
#include<climits>
using namespace std;
static constexpr int INF =INT_MAX/2;
static constexpr int WEEKDAY_PENALTY=20; /// uu tien fill cuoi tuan truoc
static constexpr int HOUR_SCALE=10; // moi 10p tinh 1 lan , nhan vien it gio hon duoc uu tien
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
OptimizerOutput Optimizer::solve(const OptimizerInput&input){
    OptimizerOutput out;
    if(input.registrations.isEmpty()||input.employees.isEmpty()){
        out.warnings << " Khong co du lieu dang ki hoac danh sach dang rong ";
        return out;
    }
    QMap<int,int>empIdx;
    for(int i = 0;i<input.employees.size();++i)
        empIdx[input.employees[i].employeeId]=i;
    const int E=input.employees.size();

    QMap<QString,int>slotIdx;
    QVector<QString> slotKeys;
    auto makeSlotKey =[](const ShiftRegistration&r){
        return r.date.toString(Qt::ISODate) + "|"
               + r.startTime.toString("HH:mm") + "|"
               + r.endTime.toString("HH:mm");

};
for (const auto& r : input.registrations) {
    QString key = makeSlotKey(r);
    if (!slotIdx.contains(key)) {
        slotIdx[key] = slotKeys.size();
        slotKeys.push_back(key);
    }
}
const int K = slotKeys.size();

const int S=0,T=1;
auto nEmp  = [&](int i) { return 2 + i; };
auto nDay  = [&](int i, int d) { return 2 + E + i * 7 + d; };
auto nSlot = [&](int j) { return 2 + 8 * E + j; };
init(2 + 8 * E + K);
QMap<int,int> empRegCount;
QMap<QPair<int,int>,int> dayRegCount;
QMap<int,int> slotRegCount;
for( const auto&r:input.registrations){
    if (!empIdx.contains(r.employeeId)) continue;
    int ei = empIdx[r.employeeId];
    int d  = r.date.dayOfWeek() - 1;
    int si = slotIdx[makeSlotKey(r)];
    empRegCount[ei]++;
    dayRegCount[{ei, d}]++;
    slotRegCount[si]++;
}
for (int i = 0; i < E; ++i) {
    int cap = empRegCount.value(i, 0);
    if (cap > 0)
        addEdge(S, nEmp(i), cap, 0);
}
for (auto it = dayRegCount.constBegin(); it != dayRegCount.constEnd(); ++it) {
    int ei = it.key().first;
    int d  = it.key().second;
    addEdge(nEmp(ei), nDay(ei, d), it.value(), 0);
}
struct RegEdge {
    int rowId;
    int edgeIdx;
    int empIdx;    // local index nhân viên
    int slotIdx;   // local index slot
    int dayOfWeek;
};
QVector<RegEdge> regEdges;
regEdges.reserve(input.registrations.size());
for (const auto& r : input.registrations) {
    if (!empIdx.contains(r.employeeId)) continue;
    int ei = empIdx[r.employeeId];
    int d  = r.date.dayOfWeek() - 1;
    int si = slotIdx[makeSlotKey(r)];
    // Priority cost: ít giờ làm lịch sử → cost thấp → ưu tiên cao
    bool isWeekend = (d >= 5);
    int  cost = input.employees[ei].totalMinutesWorked / HOUR_SCALE
               + (isWeekend ? 0 : WEEKDAY_PENALTY);
    int eid = m_edges.size();
    addEdge(nDay(ei, d), nSlot(si), 1, cost);
    regEdges.push_back({r.rowId, eid, ei, si, d});
}
for (int j = 0; j < K; ++j) {
    int cap = slotRegCount.value(j, 0);
    if (cap > 0)
        addEdge(nSlot(j), T, cap, 0);
}
int totalCost = 0;
int totalFlow = minCostFlow(S, T, INF, totalCost);
out.feasible  = (totalFlow > 0);
out.totalFlow = totalFlow;
out.totalCost = totalCost;
QMap<int, int>      assignedPerSlot;
QMap<int, QSet<int>> empDaysAssigned;
for (const auto& re : regEdges) {
    bool assigned = (m_edges[re.edgeIdx].flow == 1);
    out.assignments.push_back({re.rowId, assigned ? 1 : -1});
    if (assigned) {
        assignedPerSlot[re.slotIdx]++;
        empDaysAssigned[re.empIdx].insert(re.dayOfWeek);
    }
}
for (int j = 0; j < K; ++j) {
    int cnt = assignedPerSlot.value(j, 0);
    const QStringList parts = slotKeys[j].split("|");
    if (cnt == 0) {
        out.warnings << QString("[Cảnh báo] Ca %1 (%2 - %3): Không có nhân viên nào được xếp.")
                            .arg(parts[0]).arg(parts[1]).arg(parts[2]);
    } else if (cnt < input.minPerShift) {
        out.warnings << QString("[Cảnh báo] Ca %1 (%2 - %3): Chỉ xếp được %4/%5 nhân viên.")
                            .arg(parts[0]).arg(parts[1]).arg(parts[2])
                            .arg(cnt).arg(input.minPerShift);
    }
}
// Cảnh báo tiêu chí 5: nhân viên có ít hơn minDaysPerEmp ngày được xếp
for (int i = 0; i < E; ++i) {
    int days = empDaysAssigned.value(i).size();
    if (days > 0 && days < input.minDaysPerEmp) {
        out.warnings << QString("[Cảnh báo] Nhân viên ID %1: Chỉ được xếp %2/%3 ngày trong tuần.")
                            .arg(input.employees[i].employeeId)
                            .arg(days).arg(input.minDaysPerEmp);
    }
}
return out;
}