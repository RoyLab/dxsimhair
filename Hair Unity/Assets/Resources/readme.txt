Here's the static hair model of all results that presented in the paper: A Reduced Model for Interactive Hairs (ACM Transactions on Graphics, Siggraph 2014)

The binary file format:

int : total particle size
total particle size * float * 3: all particle positions
int : total strand size
total strand size * int : particle size for each strand (use this to get the offset in all particle positions)