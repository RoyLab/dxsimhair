#include <iostream>
//输入两个整数m,n,然后输入该m行n列矩阵mat中的元素，如果找到mat的鞍点，就输入它的下标；
//否则，输出NOT FOUND；鞍点：一个元素在所在行是最大值，在所在列是最小值

using namespace std;

int main()
{cout<<"please input m,n: ";
//申请一个动态二维数组
int **mat,i,j,m,n;//mat是动态数组的名字
cin>>m>>n;
mat=new int *[m];//申请指向每一行首地址的指针
for(i=0;i<m;++i)//为每一行申请空间
mat[i]=new int[n];

cout<<"please input array";//为动态数组赋值
for (i=0;i<m;++i)
  for (j=0;j<n;++j)
   cin>>mat[i][j];



//寻找鞍点

bool flag=true;
   for(i=0;i<m;++i)
   {
     for 循环找最大值，取得x
     for 循环找最小值，取得y
     if (x==y)
     {
       找到了！
       break;
     }
  }
  if(flag) cout<<"not found";
//最后释放每一行，以及释放保存每一行首指针的数组
   for (i=0;i<m;++i)
     delete [] mat[i];
     delete []mat;
     return 0;
}
