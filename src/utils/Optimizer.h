#ifndef OPTIMIZER_H
#define OPTIMIZER_H
#include<vector>
#include<queue>
#include<QVector>
#include<QDate>
#include<QTime>
#include<QString>
#include<QStringList>
struct ShiftRegistration{
    int rowId;
    int employeeId;
    QDate date;
    QTime startTime;
    QTime endTime;
};
struct EmployeeInfo{
    int employeeId;
    int totalMinutesWorked;
};
struct OptimizerInput{
    QVector<ShiftRegistration>registrations;
    QVector<EmployeeInfo>employees;
    int minPerShift=5; // tieu chi sort
    int minDaysPerEmp=4; // tieu chi sort
};
struct ShiftAssignment{
    int rowId;
    int newStatus;
};
struct OptimizerOutput{
    bool feasible=false; // neu tim duoc lich kha thi
    int totalFlow=0;    // tong luot sap xep ca lam thanh cong
    int totalCost=0;    // tong cost
    QVector<ShiftAssignment>assignments; // ket qua tung ham db
    QStringList warnings; // canh cao khi vi pham tieu chi nhung van cho xep lich

};


class Optimizer
{
public:
OptimizerOutput solve(const OptimizerInput& input);
private:
struct Edge{
    int to ;
    int cap;
    int cost;
    int flow;
};
QVector<Edge> m_edges; // danh sach canh
QVector<QVector<int>>m_g; // adj list
int m_n=0; // tong node
void init(int n);
void addEdge(int u,int v,int cap,int cost);
bool spfa(int s, int t, QVector<int>& dist, QVector<int>& prev_v, QVector<int>& prev_e);
int minCostFlow(int s,int t,int maxFlow,int &outCost);

};

#endif // OPTIMIZER_H
