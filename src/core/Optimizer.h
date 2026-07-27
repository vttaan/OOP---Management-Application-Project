#ifndef OPTIMIZER_H
#define OPTIMIZER_H
#include "global.h"
#include "core/Shift.h"
#include "core/User.h"


class Optimizer
{
private:
    QVector<Shift*> shifts;
    QMap<User*, int> userMinutes;
    
    bool feasible = false;
    int totalFlow = 0;
    int totalCost = 0;
    QStringList warnings;

public:
    Optimizer(const QVector<Shift*>& shifts, const QMap<User*, int>& userMinutes);
    bool solve();
    bool isFeasible() const { return feasible; }
    int getTotalFlow() const { return totalFlow; }
    int getTotalCost() const { return totalCost; }
    QStringList getWarnings() const { return warnings; }

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
