//
//  mkdgraph
//
// create random directed graph upto 64 nodes and 4032 edges
// output is list of tuples: <i> <j> <weight>
// each tuple defines an edge from node <i> to node <j>

#include <iostream>
#include <string>
#include <tuple>
#include <unordered_set>
#include <time.h>

void shuffle(int* a, int n)
{
    int k, t, pick;
    for (k=n-1; k>0; k--) {
        pick = random()%k;
        t = a[k]; a[k] = a[pick];  a[pick]=t;
    }
}

float uniform(void)
{
    float u = ((float) random())/ (float) RAND_MAX;
    return u;
}

struct hashtuple
{
    size_t operator()(const std::tuple<int,int>& tup) const
   {
       return std::hash<int>{}(get<0>(tup)) ^ std::hash<int>{}(get<1>(tup));
   }
};


int main(int argc, const char * argv[]) {
    int nn, ne, i, j, k, p[128], last, ek;
    int cc[4096], ck, ncc, cmap[4096], mk, f, fi, fj;
    FILE* gfile;
    std::tuple<int,int> e;
    std::unordered_set <std::tuple<int,int>, hashtuple> ee;
    time_t tim;
    
    if (argc<4) { printf("mkdgraph <filename> <numnodes> <numedges>\n");  exit(0); }
    time(&tim);
    srandom((unsigned int) tim);
    nn = atoi(argv[2]);  ne = atoi(argv[3]);
    printf("numnodes=%d    numedges=%d\n", nn, ne);
    
    if (ne > nn*(nn-1)) { printf("ERROR. ne exceeds nn*(nn-1)\n");  exit(0); }
    
    ck=0;   // list all poss edges
    for (i=0; i<nn; i++) {
        for (j=0; j<i; j++)  { cmap[ck]=ck;  cc[ck++]= ((i<<16) | j); }
        for (j=i+1; j<nn; j++)  { cmap[ck]=ck;  cc[ck++]= ((i<<16) | j); }
    }
    ncc=ck;
    //printf("ncc=%d\n", ncc);
    
    for (i=0; i<nn-1; i++) p[i] = i+1;
    shuffle(p, nn-1);
    
    last = 0;       // create random hamiltonian
    for(k=0; k<nn-1; k++) {
        i=last;  j=p[k];
        printf("%2d %2d\n", i, j);
        ee.insert(std::make_tuple(i,j));
        // remove from edges list
        mk = (nn-1)*i + j - (j>i);    // index of edge.ck in cmap
        ck = cmap[mk];
        if (cc[ck] != ((i<<16) | j))  printf("PROBLEM\n");
        f=cc[--ncc];            // filler is last in list
        cc[ck] = f;             // hole-fill.  filler has moved so update cmap
        fi = f>>16;  fj=f & 0xffff;
        mk = (nn-1)*fi + fj - (fj>fi) ;
        cmap[mk]=ck;
        
        last=j;
    }
    printf("%2d %2d\n", last, 0);
    ee.insert(std::make_tuple(0,last));
    // update map not necessary
    mk = (nn-1)*last;
    ck = cmap[mk];
    if (cc[ck] != last<<16)  printf("PROBLEM\n");
    f=cc[--ncc];            // filler is last in list
    cc[ck] = f;             // hole-fill.  filler has moved so update cmap
    fi = f>>16;  fj=f & 0xffff;
    mk = (nn-1)*fi + fj - (fj>fi);
    cmap[mk]=ck;
 
    // add random edges
    for (ek=nn; ek<=ne; ek++) {
        k = random()%ncc;
        ck = cc[k];  i = ck >> 16;  j = ck & 0xffff;
        printf("%2d %2d\n", i, j);
        e = std::make_tuple(i,j);
        ee.insert(e);
        f = cc[--ncc];  cc[k]=f;    // hole-fill
    }
    gfile = fopen(argv[1], "wt");
    for (auto t = ee.begin(); t!=ee.end(); t++) {
        e = *t;  i= get<0>(e);  j=get<1>(e);
        fprintf(gfile, "%2d %2d  %7.5f\n", i, j, uniform());
    }
    fclose(gfile);
    return 0;
}
