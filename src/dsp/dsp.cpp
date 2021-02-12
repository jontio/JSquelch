#include "dsp.h"
#include <math.h>
#include "../util/RuntimeError.h"

namespace JDsp
{

//----start of VectorMovingMax

//define class for int, double.
template class VectorMovingMax<int>;
template class VectorMovingMax<double>;

template <class T>
VectorMovingMax<T>::VectorMovingMax(const QPair<int,int> mn)
{
    setSize(mn);
}
template <class T>
const QVector<T> &VectorMovingMax<T>::update(const QVector<T> &input)//an input vector of size m
{
    if(m<1)RUNTIME_ERROR("input vector size needs to be positive", m);
    if(n<0)RUNTIME_ERROR("window size needs to be non-nedative", n);
    if(input.size()!=m)RUNTIME_ERROR("input vector size is not the expected size", m);
    if(n==0)
    {
        max=input;
        return max;
    }

    for(int k=0;k<m;k++)
    {
        if(input[k]>max[k])max[k]=input[k];
        double outgoing=mv[k][start];
        mv[k][start]=input[k];
        start++;start%=n;
        if(outgoing>=max[k])max[k]=*std::max_element(mv[k].begin(),mv[k].end());
    }

    return max;

}
template <class T>
void VectorMovingMax<T>::setSize(const QPair<int,int> mn)
{
    m=mn.first;
    n=mn.second;
    if(m<1)RUNTIME_ERROR("input vector size needs to be positive", m);
    if(n<0)RUNTIME_ERROR("window size needs to be non-nedative", n);
    mv.resize(m);
    max.resize(m);
    for(int k=0;k<mv.size();k++)mv[k].resize(n);
    //clear
    flush();
}
template <class T>
QPair<int,int> VectorMovingMax<T>::getSize()
{
    return QPair<int,int>(m,n);
}
template <class T>
void VectorMovingMax<T>::flush()
{
    max.fill(0);
    for(int k=0;k<mv.size();k++)mv[k].fill(0);
    start=1;
}

//----end of VectorMovingMax

//----start of VectorMovingMin

//define class for int, double.
template class VectorMovingMin<int>;
template class VectorMovingMin<double>;

template <class T>
VectorMovingMin<T>::VectorMovingMin(const QPair<int,int> mn)
{
    setSize(mn);
}
template <class T>
const QVector<T> &VectorMovingMin<T>::update(const QVector<T> &input)//an input vector of size m
{
    if(m<1)RUNTIME_ERROR("input vector size needs to be positive", m);
    if(n<0)RUNTIME_ERROR("window size needs to be non-nedative", n);
    if(input.size()!=m)RUNTIME_ERROR("input vector size is not the expected size", m);
    if(n==0)
    {
        min=input;
        return min;
    }

    for(int k=0;k<m;k++)
    {
        if(input[k]<min[k])min[k]=input[k];
        double outgoing=mv[k][start];
        mv[k][start]=input[k];
        start++;start%=n;
        if(outgoing<=min[k])min[k]=*std::min_element(mv[k].begin(),mv[k].end());
    }

    return min;

}
template <class T>
void VectorMovingMin<T>::setSize(const QPair<int,int> mn)
{
    m=mn.first;
    n=mn.second;
    if(m<1)RUNTIME_ERROR("input vector size needs to be positive", m);
    if(n<0)RUNTIME_ERROR("window size needs to be non-nedative", n);
    mv.resize(m);
    min.resize(m);
    for(int k=0;k<mv.size();k++)mv[k].resize(n);
    //clear
    flush();
}
template <class T>
QPair<int,int> VectorMovingMin<T>::getSize()
{
    return QPair<int,int>(m,n);
}
template <class T>
void VectorMovingMin<T>::flush()
{
    min.fill(0);
    for(int k=0;k<mv.size();k++)mv[k].fill(0);
    start=1;
}

//----end of VectorMovingMin

//----start of VectorMovingVariance

//define class for int, double and std::complex<double>
template class VectorMovingVariance<int>;
template class VectorMovingVariance<double>;
template class VectorMovingVariance<std::complex<double>>;

template <class T>
VectorMovingVariance<T>::VectorMovingVariance()
{

}
template <class T>
VectorMovingVariance<T>::VectorMovingVariance(const QPair<int,int> mn)
{
    setSize(mn);
}
template <class T>
QVector<T> &VectorMovingVariance<T>::update(const QVector<T> &input)//an input vector of size m
{
    QVector<T> inputSquared=input;
    for(int k=0;k<m;k++)inputSquared[k]*=inputSquared[k];
    E2<<inputSquared;
    E1<<input;
    for(int k=0;k<m;k++)
    {
        val[k]=scale*(E2[k]-(E1[k]*E1[k]));
    }
    return val;
}
template <class T>
void VectorMovingVariance<T>::setSize(const QPair<int,int> mn)
{
    E1.setSize(mn);
    E2.setSize(mn);
    m=mn.first;
    n=mn.second;
    val.resize(m);
    if(n<2)scale=0;
    else scale=((double)(n))/((double)(n-1));
    flush();
}
template <class T>
QPair<int,int> VectorMovingVariance<T>::getSize()
{
    return QPair<int,int>(m,n);
}
template <class T>
void VectorMovingVariance<T>::flush()
{
    E1.flush();
    E2.flush();
    val.fill(0);
}

//----start VectorMovingAverage

//define class for int, double and std::complex<double>
template class VectorMovingAverage<int>;
template class VectorMovingAverage<double>;
template class VectorMovingAverage<std::complex<double>>;

template <class T>
VectorMovingAverage<T>::VectorMovingAverage()
{
}
template <class T>
VectorMovingAverage<T>::VectorMovingAverage(const QPair<int,int> mn)
{
    setSize(mn);
}
template <class T>
QVector<T> &VectorMovingAverage<T>::update(const QVector<T> &input)//an input vector of size m
{
    if(m<1)RUNTIME_ERROR("input vector size needs to be positive", m);
    if(n<0)RUNTIME_ERROR("window size needs to be non-nedative", n);
    if(input.size()!=m)RUNTIME_ERROR("input vector size is not the expected size", m);
    if(n==0)
    {
        val=input;
        return val;
    }

    if(start==0)
    {
        for(int k=0;k<m;k++)
        {
            mv[k][start]=input[k];
            mvsum[k]=0;
            for(int i=0;i<n;i++)mvsum[k]+=mv[k][i];
            val[k]=mvsum[k]*scale;
        }
    }
    else
    {
        for(int k=0;k<m;k++)
        {
            mvsum[k]-=mv[k][start];
            mvsum[k]+=input[k];
            mv[k][start]=input[k];
            val[k]=mvsum[k]*scale;
        }
    }
    start++;start%=n;

    return val;
}
template <class T>
void VectorMovingAverage<T>::setSize(const QPair<int,int> mn)
{
    //resize
    m=mn.first;
    n=mn.second;
    if(m<1)RUNTIME_ERROR("input vector size needs to be positive", m);
    if(n<0)RUNTIME_ERROR("window size needs to be non-nedative", n);
    mv.resize(m);
    mvsum.resize(m);
    val.resize(m);
    for(int k=0;k<mv.size();k++)mv[k].resize(n);
    if(n>0)scale=1.0/((double)n);
    //clear
    flush();
}
template <class T>
QPair<int,int> VectorMovingAverage<T>::getSize()
{
    return QPair<int,int>(m,n);
}
template <class T>
void VectorMovingAverage<T>::flush()
{
    //fill with zeros
    mvsum.fill(0);
    val.fill(0);
    for(int k=0;k<mv.size();k++)mv[k].fill(0);
    start=1;
}

//----end VectorMovingAverage

//----start MovingAverage

//define class for int, double and std::complex<double>
template class MovingAverage<int>;
template class MovingAverage<double>;
template class MovingAverage<std::complex<double>>;

template <class T>
MovingAverage<T>::MovingAverage()
{
}

template <class T>
MovingAverage<T>::MovingAverage(int number)
{
    setSize(number);
}

template <class T>
MovingAverage<T>::MovingAverage(const MovingAverage<T> &rhs)
{
    setSize(rhs.MASz);
}

template <class T>
void MovingAverage<T>::setSize(int number)
{
    if(MASz>0)delete [] MABuffer;
    MASz=number;
    MASum=0;
    if(MASz<0)RUNTIME_ERROR("array size needs to be non negative", number);
    if(MASz)MABuffer=new T[MASz];
    for(int i=0;i<MASz;i++)MABuffer[i]=0;
    MAPtr=0;
    val=0;
    PrecisionDilutionCorrectCnt=MASz-1;
    scale=1.0/((double)MASz);
}

template <class T>
int MovingAverage<T>::getSize()
{
    return MASz;
}

template <class T>
T MovingAverage<T>::update(T sig)
{
    if(MASz<=0)
    {
        MASum=sig;
        val=MASum;
        return val;
    }

    MASum=MASum-MABuffer[MAPtr];
    MASum=MASum+sig;
    MABuffer[MAPtr]=sig;

    if(PrecisionDilutionCorrectCnt>0)PrecisionDilutionCorrectCnt--;
     else
     {
        MASum=0;
        for(int i=0;i<MASz;++i)
        {
            MASum+=MABuffer[i];
        }
        PrecisionDilutionCorrectCnt=MASz-1;
     }

    MAPtr++;MAPtr%=MASz;
    val=MASum*scale;
    return val;
}

template <class T>
MovingAverage<T>::~MovingAverage()
{
    if(MASz>0)delete [] MABuffer;
    MASz=0;
}

//----end MovingAverage

}
