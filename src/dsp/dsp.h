#ifndef DSP_H
#define DSP_H
#include <stdio.h>
#include <utility>

namespace JDsp
{

template <class T>
class MovingAverage
{
public:

    ~MovingAverage();
    T Update(T sig);
    T val;

    MovingAverage();
    MovingAverage(int number);
    MovingAverage(const MovingAverage<T> &rhs);
    void setSize(int number);
    int getSize();

    //copy assignment
    MovingAverage<T> &operator=(const MovingAverage<T> &rhs )
    {
        if(this==&rhs)return *this;
        this->setSize(rhs.MASz);
        return *this;
    }

protected:
    int MASz=0;
    T MASum;
    T *MABuffer;
    int MAPtr;
    int PrecisionDilutionCorrectCnt=0;
    double scale=1;
};

}

#endif // DSP_H
