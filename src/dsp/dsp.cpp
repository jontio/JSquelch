#include "dsp.h"
#include <math.h>
#include <complex>
#include "../util/RuntimeError.h"

namespace JDsp
{

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
    if(MASz<0)RUNTIME_ERROR("array size needs to be positive", number);
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
T MovingAverage<T>::Update(T sig)
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



}
