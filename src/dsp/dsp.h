#ifndef DSP_H
#define DSP_H

#include <QVector>
#include <QDebug>
#include <complex>

namespace JDsp
{

template <class T>
class VectorMovingAverage
{
public:
    VectorMovingAverage();
    VectorMovingAverage(const QPair<int,int> mn);
    QVector<T> &update(const QVector<T> &input);//an input vector of size m
    void setSize(const QPair<int,int> mn);
    QPair<int,int> getSize();
    void flush();
    QVector<T> val;
    //insertion operator
    inline QVector<T> &operator<< (const QVector<T> &input)
    {
        update(input);
        return val;
    }
    //index operator
    inline const T &operator[](int i) const
    {
        return val[i];
    }
    inline T &operator[](int i)
    {
        return val[i];
    }
private:
    QVector<QVector<T>> mv;
    QVector<T> mvsum;
    int m=0;
    int n=0;
    int start=0;
    double scale=1;
};

//VectorMovingVariance calculates variance but also calulates mean in the process so you dont have to use VectorMovingAverage if you are already using VectorMovingVariance
template <class T>
class VectorMovingVariance
{
public:
    VectorMovingVariance();
    VectorMovingVariance(const QPair<int,int> mn);
    QVector<T> &update(const QVector<T> &input);//an input vector of size m
    void setSize(const QPair<int,int> mn);
    QPair<int,int> getSize();
    void flush();
    QVector<T> val;
    //access to mean object to save having to recalculate this when needed both var and mean
    inline const VectorMovingAverage<T> &mean()
    {
        return E1;
    }
    //insertion operator
    inline QVector<T> &operator<< (const QVector<T> &input)
    {
        update(input);
        return val;
    }
    //index operator
    inline const T &operator[](int i) const
    {
        return val[i];
    }
    inline T &operator[](int i)
    {
        return val[i];
    }
private:
    VectorMovingAverage<T> E1,E2;
    int m=0;
    int n=0;
    double scale=1;
};



template <class T>
class MovingAverage
{
public:

    ~MovingAverage();
    T update(T sig);
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

//qdebug outputs for variance
inline QDebug operator<<(QDebug debug, const JDsp::VectorMovingVariance<double> &obj)
{
    return QtPrivate::printSequentialContainer(debug, "Variance", obj.val);
}
inline QDebug operator<<(QDebug debug, const JDsp::VectorMovingVariance<int> &obj)
{
    return QtPrivate::printSequentialContainer(debug, "Variance", obj.val);
}
inline QDebug operator<<(QDebug debug, const JDsp::VectorMovingVariance<std::complex<double>> &obj)
{
    Q_UNUSED(obj);
    QDebugStateSaver saver(debug);
    debug.nospace()<<"Variance qDebug for complex not implimented yet";
    return debug;
}

//qdebug outputs for mean
inline QDebug operator<<(QDebug debug, const JDsp::VectorMovingAverage<double> &obj)
{
    return QtPrivate::printSequentialContainer(debug, "Mean", obj.val);
}
inline QDebug operator<<(QDebug debug, const JDsp::VectorMovingAverage<int> &obj)
{
    return QtPrivate::printSequentialContainer(debug, "Mean", obj.val);
}
inline QDebug operator<<(QDebug debug, const JDsp::VectorMovingAverage<std::complex<double>> &obj)
{
    Q_UNUSED(obj);
    QDebugStateSaver saver(debug);
    debug.nospace()<<"Mean qDebug for complex not implimented yet";
    return debug;
}


#endif // DSP_H
