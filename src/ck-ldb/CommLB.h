#ifndef _COMMLB_H_
#define _COMMLB_H_
#endif

#include <CentralLB.h>
#include "CommLB.decl.h"

#include "heap.h"
#define CUT_OFF_FACTOR 1.200

struct obj_id{
  LDObjid oid;
  LDOMid mid;
};

struct graph{
  int id;
  int data;
  int nmsg;
  struct graph * next;
};

void CreateCommLB();

class CommLB : public CentralLB {
public:
  int nobj,npe;
  double ** alloc_array;
  graph * object_graph;
  obj_id * translate;
  int * htable;
  CommLB();
private:
  CmiBool QueryBalanceNow(int step);
  CLBMigrateMsg* Strategy(CentralLB::LDStats* stats, int count);
  void alloc(int pe, int id, double load);
  double compute_com(int id,int pe); 
  int search(LDObjid oid, LDOMid mid);
  void add_graph(int x, int y, int data, int nmsg);
  void make_hash();
};
