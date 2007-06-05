/** \file TopoManager.h
 *  Author: Abhinav S Bhatele
 *  Date Created: March 19th, 2007
 *
 *  This would be the top level interface for all topology managers we
 *  will write for different machines (cray, bg/l ... for tori, meshes ...)
 *  Current plan is to have functionality for Blue Gene/L, Cray XT3,
 *  BigSim and non-topo machines.
 */

#ifndef _TOPO_MANAGER_H_
#define _TOPO_MANAGER_H_

#include "charm.h"

#ifdef CMK_VERSION_BLUEGENE
#include "bgltorus.h"
#elif CMK_BLUEGENEP
#include "BGPTorus.h"
#elif CMK_XT3
#include "CrayTorus.h"
#endif

class TopoManager {
  private:
    int dimX;	// dimension of the allocation in X (processors)
    int dimY;	// dimension of the allocation in Y (processors)
    int dimZ;	// dimension of the allocation in Z (processors)
    int dimNX;	// dimension of the allocation in X (nodes)
    int dimNY;	// dimension of the allocation in Y (nodes)
    int dimNZ;	// dimension of the allocation in Z (nodes)
    int torusX, torusY, torusZ, torusT; 
    int procsPerNode;

#ifdef CMK_VERSION_BLUEGENE
    BGLTorusManager *bgltm;
#elif CMK_BLUEGENEP
    BGPTorusManager *bgptm;
#elif CMK_XT3
    CrayTorusManager *crtm;
#endif

  public:

    TopoManager() {
#ifdef CMK_VERSION_BLUEGENE
      bgltm = BGLTorusManager::getObject();
  
      dimX = bgltm->getXSize();
      dimY = bgltm->getYSize();
      dimZ = bgltm->getZSize();
    
      dimNX = bgltm->getXNodeSize();
      dimNY = bgltm->getYNodeSize();
      dimNZ = bgltm->getZNodeSize();

      if(bgltm->isVnodeMode())
        procsPerNode = 2;
      else
        procsPerNode = 1;
#elif CMK_BLUEGENEP
      bgptm = new BGPTorusManager();
  
      dimX = bgptm->getDimX();
      dimY = bgptm->getDimY();
      dimZ = bgptm->getDimZ();
    
      dimNX = bgptm->getDimNX();
      dimNY = bgptm->getDimNY();
      dimNZ = bgptm->getDimNZ();

      procsPerNode = bgptm->getProcsPerNode();
      int *torus = (int *)malloc(sizeof(int)*4);
      torus = bgptm->isTorus();
      torusX = torus[0];
      torusY = torus[1];
      torusZ = torus[2];
      torusT = torus[3];

#elif CMK_XT3

#else
      dimX = CkNumPes();
      dimY = 1;
      dimZ = 1;

      dimNX = dimX;
      dimNY = 1;
      dimNZ = 1;

      procsPerNode = 1;
#endif
    }

    TopoManager(int X, int Y, int Z, int NX, int NY, int NZ ) : dimX(X), dimY(Y), dimZ(Z),  dimNX(NX), dimNY(NY), dimNZ(NZ)
    {
      // we rashly assume only one dimension is expanded 
      procsPerNode = dimX/dimNX;
      procsPerNode = (dimY/dimNY >procsPerNode) ? dimY/dimNY: procsPerNode;
      procsPerNode = (dimZ/dimNZ >procsPerNode) ? dimZ/dimNZ: procsPerNode;
    }

    ~TopoManager() {
#ifdef CMK_BLUEGENEP
       delete bgptm;
#endif
     }

    inline int getDimX() { return dimX; }
    inline int getDimY() { return dimY; }
    inline int getDimZ() { return dimZ; }

    inline int getDimNX() { return dimNX; }
    inline int getDimNY() { return dimNY; }
    inline int getDimNZ() { return dimNZ; }

    inline int absX(int x) {
      int px = abs(x);
      int sx = dimX - px;
      CmiAssert(sx>=0);
      if(torusX)
        return ((px>sx) ? sx : px);
      else
        return px;
    }
    
    inline int absY(int y) {
      int py = abs(y);
      int sy = dimY - py;
      CmiAssert(sy>=0);
      if(torusY)
        return ((py>sy) ? sy : py);
      else
        return py;
    }

    inline int absZ(int z) {
      int pz = abs(z);
      int sz = dimZ - pz;
      CmiAssert(sz>=0);
      if(torusZ)
        return ((pz>sz) ? sz : pz);
      else
        return pz;
    }
    
    int hasMultipleProcsPerNode();
    void rankToCoordinates(int pe, int &x, int &y, int &z);
    int coordinatesToRank(int x, int y, int z);
    int getHopsBetweenRanks(int pe1, int pe2);
    void sortRanksByHops(int pe, int *pes, int *idx, int n); 
    int pickClosestRank(int mype, int *pes, int n);
    int areNeighbors(int pe1, int pe2, int pe3, int distance);

  private:
    void quicksort(int pe, int *pes, int *arr, int left, int right);
    int partition(int pe, int *pes, int *idx, int left, int right);

};

#endif //_TOPO_MANAGER_H_
