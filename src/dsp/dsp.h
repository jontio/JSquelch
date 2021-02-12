#ifndef DSP_H
#define DSP_H

#include <QVector>
#include <QDebug>
#include <complex>
#include "../util/RuntimeError.h"

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
    inline operator QVector<T>() const
    {
        return val;
    }
private:
    VectorMovingAverage<T> E1,E2;
    int m=0;
    int n=0;
    double scale=1;
};

template <class T>
class VectorMovingMax
{
public:
    VectorMovingMax(){}
    VectorMovingMax(const QPair<int,int> mn);
    const QVector<T> &update(const QVector<T> &input);//an input vector of size m
    void setSize(const QPair<int,int> mn);
    QPair<int,int> getSize();
    void flush();
    //do not change max after init else it wont work hence why const and no direct access
    inline const QVector<T> &getMax(){return max;}
    //insertion operator
    inline QVector<T> &operator<< (const QVector<T> &input)
    {
        update(input);
        return max;
    }
    //index operator
    inline const T &operator[](int i) const
    {
        return max[i];
    }
    inline operator QVector<T>() const
    {
        return max;
    }
protected:
    inline const QVector<QVector<T>> &getMv(){return mv;}
private:
    int m=0;
    int n=0;
    int start=0;
    QVector<T> max;
    QVector<QVector<T>> mv;
};

template <class T>
class VectorMovingMin
{
public:
    VectorMovingMin(){}
    VectorMovingMin(const QPair<int,int> mn);
    const QVector<T> &update(const QVector<T> &input);//an input vector of size m
    void setSize(const QPair<int,int> mn);
    QPair<int,int> getSize();
    void flush();
    //do not change min after init else it wont work hence why const and no direct access
    inline const QVector<T> &getMin(){return min;}
    //insertion operator
    inline QVector<T> &operator<< (const QVector<T> &input)
    {
        update(input);
        return min;
    }
    //index operator
    inline const T &operator[](int i) const
    {
        return min[i];
    }
    inline operator QVector<T>() const
    {
        return min;
    }
protected:
    inline const QVector<QVector<T>> &getMv(){return mv;}
private:
    int m=0;
    int n=0;
    int start=0;
    QVector<T> min;
    QVector<QVector<T>> mv;
};

//---not used
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
inline QDebug operator<<(QDebug debug, JDsp::VectorMovingVariance<double> &obj)
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

//qdebug outputs for min
inline QDebug operator<<(QDebug debug, JDsp::VectorMovingMin<double> &obj)
{
    return QtPrivate::printSequentialContainer(debug, "Min", obj.getMin());
}
inline QDebug operator<<(QDebug debug, JDsp::VectorMovingMin<int> &obj)
{
    return QtPrivate::printSequentialContainer(debug, "Min", obj.getMin());
}

//qdebug outputs for max
inline QDebug operator<<(QDebug debug, JDsp::VectorMovingMax<double> &obj)
{
    return QtPrivate::printSequentialContainer(debug, "Max", obj.getMax());
}
inline QDebug operator<<(QDebug debug, JDsp::VectorMovingMax<int> &obj)
{
    return QtPrivate::printSequentialContainer(debug, "Max", obj.getMax());
}


#endif // DSP_H
