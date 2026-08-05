//
//  d_cbc_hamiltonian
//
//  find optimal hamiltonian in directed graph using mixed integer linear programming (MILP)
//
//  binary variable xi_j represents edge from vtx i to vtx j
//  xi_j=1 if edge i->j in solution
//  xi_j=0 : i->j not in solution
//  for hamiltonian each vtx is connected by one incoming edge + one outgoing edge.
//  this gives constraints:
//  for each vtxi: sum_over_j(xi_j) == 1
//  for each vtxj: sum_over_i(xi_j) == 1
//  seek hamiltonian with min(sum_of_weights(included edges))
//  model this with MILP
//  this model can result in solutions with multiple smaller cycles rather than
//  one visit-every-vtx-cycle.  overcome this by adding additional constraints that
//  disallow the mini-cycles.  repeat if necessary.
//  all the hard stuff is done by coin-or optimization tools
//  input file is just a list of directed edges.  each line has three numbers:
//  <i> <j> <weight>
//  i is the from-vtx.  j is the to-vtx.  <weight> is the objective coefficient for the edge variable.

#include <iostream>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include "CoinPackedVector.hpp"
#include "CoinPackedMatrix.hpp"
#include "OsiClpSolverInterface.hpp"
#include "CbcModel.hpp"
#include "CbcSolver.hpp"

struct hashtuple
{
    size_t operator()(const std::tuple<int,int>& tup) const
   {
       return std::hash<int>{}(get<0>(tup)) ^ std::hash<int>{}(get<1>(tup));
   }
};

std::string vname(int i, int j)
{
    char v[10];
    snprintf(v, 7, "x%02d_%02d", i, j);
    return v;
}

double* colsolutions;
bool infeasible=false;
volatile bool have_result=false;

int callbak(CbcModel* cbcmodel, int wf)
{
    int ncols;
    if (cbcmodel->isProvenOptimal()) {
        printf("callbak:  obj value=%lf\n", cbcmodel->getObjValue());
        ncols=cbcmodel->getNumCols();
        memcpy(colsolutions, cbcmodel->getColSolution(), ncols*sizeof(double));
        infeasible = false;
        //have_result = true;
    }
    else {
        if (cbcmodel->isProvenInfeasible())  infeasible=true;
    }
    return false;
}

bool partite_test(int numvtx, std::unordered_map <std::tuple<int,int>, double, hashtuple> edgew)
{                            // check all vtxs reachable from 0
    bool vizited[numvtx];
    std::unordered_set<int>  outadj[numvtx];
    int i, j, numviz, fi, noofrontier, links[numvtx];
    
    for (auto e: edgew) {   // build adjacency table
        i = get<0>(e.first);
        j = get<1>(e.first);
        outadj[i].insert(j);
    }
    memset(vizited, false, sizeof(vizited));
    
    links[0]=-1;
    fi=0;       // linked list containing vtx0
    vizited[0]=true;
    numviz=1;
    noofrontier= -1;
    while (fi>-1) {
        for (int adj : outadj[fi]) {
            if (!vizited[adj]) {
                links[adj] = noofrontier;
                noofrontier = adj;
                vizited[adj] = true;
                numviz++;
            }
        }
        fi = links[fi];     // next in frontier list
        if (fi<0) {  fi=noofrontier;  noofrontier=-1; }
    }
    printf("partite_test:  numviz=%d\n", numviz);
    return (numviz < numvtx);
}

int main(int argc, const char * argv[]) {
    FILE* gfile;
    int ni, i, j, k, numedges=0, numvtx=0;
    double w;
    std::unordered_map <std::tuple<int,int>, double,  hashtuple> edgew;
    std::tuple<int,int> t;
    OsiClpSolverInterface* si = new OsiClpSolverInterface();
    char rowname[10];
    
    if (argc <2)  { printf("usage:  d_cbc_hamiltonian <filename>\n");  exit(0); }
    gfile = fopen(argv[1], "rt");
    if (gfile==NULL) {
        printf("file not found\n");
        exit(0);
    }    
    do {            // read data
        ni = fscanf(gfile, "%d %d %lf", &i, &j, &w);
        if (ni==3) {
           // printf("%d %d %lf\n", i, j, w);
            t = std::make_tuple(i, j);
            edgew[t] = w;
            numedges++;
            if (i>=numvtx)  numvtx=i+1;
            if (j>=numvtx)  numvtx=j+1;
        }
    }  while(ni==3);
    printf("numedges=%d  numvtx=%d\n", numedges, numvtx);
    fclose(gfile);
    
    if (partite_test(numvtx, edgew))  {
        printf("partite test FAILED therefore no hamiltonian.\n");
        exit(0);
    }
    // TODO check each vtx has >= 2 neighbs
    colsolutions = (double*) malloc(numedges * sizeof(double));
   
    double objective[numedges];
    std::tuple<int,int> ee[numedges];
    std::vector<std::string> varnames;
    int ne=2, indx[ne], nrows;
    double el[] = { 1.0, 1.0 };
    double colupperbounds[numedges];
 
    CoinPackedMatrix cpmat;   // build matrix column by column
    k=0;                      // two rows for each vtx v
                              // row 2v : inputs==1
                              // row 2v+1:  outputs==1
    for (auto e : edgew) {
        i=get<0>(e.first);  j=get<1>(e.first);
        w = e.second;
        std::cout << i << "_" << j << "   " << w << "\n";
        ee[k]=e.first;
        objective[k]=w;
        varnames.push_back(vname(i,j));
        indx[0]=2*i;  indx[1]=2*j+1;
        CoinPackedVector cpvec(ne, indx, el);
        cpmat.appendCol(cpvec);
        colupperbounds[k]=1.0;
        k++;
    }
    nrows = cpmat.getNumRows();
    double rowbounds[nrows];
    for (k=0; k<nrows; k++)  rowbounds[k]=1.0;  // hamiltonian has 1 input and 1 output per vtx
                                                // equality: lobound=upbound
    si->loadProblem(cpmat, NULL, colupperbounds, objective, rowbounds, rowbounds);
    si->setObjective(objective);
    si->setColNames(varnames, 0, numedges, 0);
    for (k=0; k<numedges; k++)  si->setInteger(k);  // all variables binary
    
    for (k=0; k<nrows; k++) {
        if (k & 0x01)   snprintf(rowname, 7, "OUT%02d", k/2);
        else  snprintf(rowname, 7, "IN%02d", k/2);
        si->setRowName(k, rowname);
    }
    si->setObjName("OBJ");
    //si->setObjSense(-1); // uncomment to maximize objective
    //si->writeMps("/Users/mongoose/g12_59");
    
    CbcModel model(*si);
    callCbc1("-solve -quit", model, callbak);
    
    if (infeasible) {
        printf("MILP infeasable.  graph has no hamiltonian cycle\n");
        exit(0);
    }
    
    int ncycles, subcyclecond=0, iter=1;
    do {
        int output[numvtx];
        int ocolk[numvtx];   // might need to get from edge back to column indx
        for (k=0; k<numedges; k++) {
            printf("%2d  %s  %.1lf\n", k, varnames[k].c_str(), colsolutions[k]);
            if (colsolutions[k] > 0.99) {
                i = get<0>(ee[k]);  j = get<1>(ee[k]);
                output[i]=j;
                ocolk[i]=k;
            }
        }
        
        std::unordered_set<int> unvisited;
        for (k=0; k<numvtx; k++)  unvisited.insert(k);
        int v, nxt, org=0, cycle[numvtx], cyclestarts[numvtx];
        int edge_indx[numvtx];  // indices of edges that form cycle
        ncycles=0;
        k=0; v=org;  cyclestarts[ncycles]=k;
        
        // construct cycles from output list
        printf("----------------------\n");
        while (k<numvtx) {
            cycle[k]=v;
            printf("%d  ", v);
            unvisited.erase(v);
            nxt = output[v];  edge_indx[k] = ocolk[v];
            v=nxt;
            k++;
            if (v==org) {  //cycle complete
                ncycles++;
                cyclestarts[ncycles]=k;
                if (k==numvtx && org==0 ) {
                    printf("-> %d  is optimal HAMILTONIAN\n", org);
                }
                else {
                    printf("\n___________________________\n");
                    if (k<numvtx) {
                        v = org = *unvisited.begin();   // start new cycle with unvisited vtx
                    }
                }
            }
        }
        if (ncycles > 1) {
            printf("pseudo solution has %d cycles\n", ncycles);
            printf("adding constraints to clobber subset-cycles\n");
            double elems[numvtx];
            for (k=0; k<numvtx; k++)  elems[k]=1.0;
            for (k=0; k<ncycles; k++) {
                int cycle_len = cyclestarts[k+1] - cyclestarts[k];
                int* colindx = edge_indx + cyclestarts[k];
                CoinPackedVector bvec(cycle_len, colindx, elems);
                //int *bvis = bvec.getIndices(), ii;
                //for (ii=0; ii<cycle_len; ii++)  printf("%d  ", bvis[ii]);
                //printf("\n");
                snprintf(rowname, 7, "CYBR%02d", subcyclecond++);
                si->addRow(bvec, 0.0, cycle_len-1, rowname);
            }
            CbcModel model(*si);
            have_result=false;
            callCbc1("-solve -quit", model, callbak);
            if (infeasible) {
                printf("MILP infeasible.  graph has no hamiltonian cycle\n");
                exit(0);
            }
            iter++;
        }
    } while (ncycles>1 && iter<64);
    if (iter>63)  printf("too many iterations\n");
    return 0;
}
