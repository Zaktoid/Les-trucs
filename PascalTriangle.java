import java.lang.*;

class PascalTriangle
{
    private int NBlines;
    private int [][] el;
    PascalTriangle(int n)
    {
        this.NBlines=n;
        this.el =new int[NBlines][];
        this.el[0]=new int[1];
        this.el[0][0]=1;
        this.el[1]=new int[2];
        this.el[1][0]=this.el[1][1]=1;
        for(int i=2;i<NBlines;i++)
        {
            this.el[i]=new int[i+1];
            this.el[i][0]=this.el[i][i]=1;
            for(int k=1;k<i;k++)
                {
                    this.el[i][k]=this.el[i-1][k-1]+this.el[i-1][k];
                }
        }

    }
    void Display()
    {
        for(int i=0;i<NBlines;i++)
        {
            for(int k=0;k<i+1;k++)
                System.out.print(el[i][k]);
            System.out.print("\n");
        }
    }
}