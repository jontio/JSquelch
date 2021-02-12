# JSquelch

![CI](https://github.com/jontio/JSquelch/workflows/CI/badge.svg)

Next step of the voice detection code.

This commit I have added a moving stats library. We now have moving min, max, variance, and mean. These classes accept vectors as their input.

```C++
JDsp::VectorMovingVariance<double> mv(QPair<int,int>(4,5));
JDsp::VectorMovingMin<double> mm(QPair<int,int>(4,5));
JDsp::VectorMovingMax<double> mM(QPair<int,int>(4,5));

QVector<double> x={5,3,6,1};
qDebug()<<mm<<mM<<mv<<mv.mean();
for(int k=0;k<10;k++)
{
    mm<<x;
    mM<<x;
    mv<<x;
    qDebug()<<mm<<mM<<mv<<mv.mean();
    x[0]+=1;
    x[1]+=2;
}
```

