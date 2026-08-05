# hamiltonian cycles in graphs

mkugraph generates a random undirected graph with N vertices + E edges
containing at least one hamiltonian.

mkdgraph outputs a random directed graph with N vertices + E edges
containing at least one hamiltonian.

u_brute_force_ham finds hamiltonian(s) in an undirected graph.\
d_brute_force_ham finds hamiltonian(s) in a directed graph.\
the brute force programs can only handle smallish graphs (eg N=12 E=50).

u_cbc_hamiltonian finds optimal hamiltonian in undirected graph.\
d_cbc_hamiltonian finds optimal hamiltonian in directed graph.\
these programs can cope with large problems.  they use mixed integer
linear programming tools to do the donkey work.  they depend on the
following coin-or libraries + executable:

libCoinUtils\
libOsi\
libOsiClp\
libClpSolver\
libCbcSolver\
libCbc\
cbc

the dependencies can be downloaded from (https://github.com/coin-or)
 
hardnut.ug is a nasty graph with many small low-weight sub-cycles.
these are stitched together in a heirarchy using beefier edges.
this kind of graph forces u_cbc_hamiltonian to do multiple passes
that add extra contraints to eliminate the sub-cycles.

the .mk files work on old x86 macbook but probably need tweaking
for other kit/operating system.
