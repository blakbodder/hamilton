//
//  d_brute_force_ham
//
#include <iostream>
#include <forward_list>
#include <tuple>
#include <unordered_map>

#define BAK 0
#define FORWARD 1

struct node
{
 //   int na;     // num adjacents
    std::forward_list<int> adj;
    bool neighbzero;
    bool visited;
};

struct node g[128];

struct hashtuple
{
    size_t operator()(const std::tuple<int,int>& tup) const
   {
       return std::hash<int>{}(get<0>(tup)) ^ std::hash<int>{}(get<1>(tup));
   }
};

void init_g(void)
{
    int i;
    for (i=0; i<128; i++) {
        g[i].neighbzero=false;
        g[i].visited=false;
    }
}

void mark_input_neighbs_of_zero(std::forward_list<int> inoz)
{
    std::forward_list<int>::iterator li;
    struct node *ni, *nj;
    
    ni = g;     // ->node[0]
    for (li = inoz.begin(); li != inoz.end(); li++) { nj = g+*li;  nj->neighbzero=true; }
}

void dump_adjtab(int nn)
{
    int i;
    struct node *ni;
    std::forward_list<int>::iterator li;
    printf("---------------------\n");
    for (i=0; i<nn; i++) {
        ni = g+i;
        printf("[%2d]  ",i);
        for (li = ni->adj.begin(); li!= ni->adj.end(); li++)  printf("%2d ",*li);
        printf("\n");
    }
    printf("---------------------\n");
}

void sum_of_weights(int len, int* path, std::unordered_map<std::tuple<int,int>, float, hashtuple> & ww )
{
    int i, j, k;
    float sum=0.0f;
    std::tuple<int,int> e;
    
    i=path[0];  k=1;
    while (k<=len) {
        j=path[k++];
        //printf("%d-%d ", i, j);
        e=std::make_tuple(i, j);
        sum += ww[e];
        i=j;
    }
    e = std::make_tuple(i, 0);
    sum += ww[e];
    printf("sum of weights=%.5f\n", sum);
}

int main(int argc, const char * argv[]) {
    FILE* gfile;
    int stak[128], sp, nn, n, i, j, mode;
    long numhams=0, iter=0;
    std::forward_list<int>::iterator li;
    std::forward_list<int>::iterator listk[128];
    std::forward_list<int> inoz;    // neighbs -> 0
    std::tuple<int,int> e;
    std::unordered_map<std::tuple<int,int>, float, hashtuple> ww;   // edge weights
    struct node *ni, *nj;
    bool notdone;
    float w;
    
    if (argc<2)  { printf("usage:  d_brute_force_ham <filename>");  exit(0); }
    
    gfile = fopen(argv[1], "rt");
    if (gfile==NULL) {
       printf("file not found\n");
       exit(0);
    }
    init_g();
    nn=0;
    do {        // read data.  build graph
        n = fscanf(gfile, "%d%d%f", &i, &j, &w);
        if (n==3) {
            printf("%d %d\n", i, j);
            if (i>nn)  nn=i;
            if (j>nn)  nn=j;
            ni = g+i;  nj = g+j;
            ni->adj.push_front(j);
            e = std::make_tuple(i,j);
            ww[e]=w;
            if (j==0)  inoz.push_front(i);
        }
    }  while (n==3);
    nn++;
  
    mark_input_neighbs_of_zero(inoz);
    dump_adjtab(nn);
    i=sp=0;
    ni=g+i;  li=ni->adj.begin();
    mode=FORWARD;
    notdone=true;
    while(notdone) {
        switch (mode) {
            case BAK:
               // printf("\n");
                if (sp==0) { printf("DONE\n");  notdone=false;  break; }
                ni->visited=false;
                li = listk[--sp];  i = stak[sp];  ni=g+i;
                // try next neighb.  mode=FORWARD  fall thru - no break
                
            case FORWARD:
                if (li == ni->adj.end())  { mode=BAK;  break; }
                ni->visited=true;
                if (sp==nn-1) {     // hamilton near complete
                    if (ni->neighbzero)  {
                        printf("HAM: ");
                        for (sp=0; sp<nn-1; sp++)  printf("%d ", stak[sp]);
                        printf("%d -> 0\n", i);
                        sp=nn-1;  stak[sp]=i;
                        sum_of_weights(sp, stak, ww);
                        numhams++;
                    }
                    mode=BAK;
                }
                else {
                    j = *li;  nj=g+j;  li++;
                    //printf("%d ", j);
                    if (nj->visited)  mode= FORWARD;
                    else {
                        stak[sp]=i;  listk[sp++]=li;
                        i = j;  ni = nj;  li = ni->adj.begin();
                        mode = FORWARD;
                    }
                }
        }
        ++iter;
    }
    printf("number of hamiltons=%ld    iter=%ld\n", numhams,  iter);
    return 0;
}

