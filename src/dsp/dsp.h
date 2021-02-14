#ifndef DSP_H
#define DSP_H

#include <QVector>
#include <QDebug>
#include <complex>
#include "../JFFT/jfft.h"
#include "../util/RuntimeError.h"

namespace JDsp
{

template <class T>
struct JVector : public QVector<T>
{
public:
    JVector(int n) : QVector<T>(n){}
    JVector(){}
    JVector(std::initializer_list<T> args) : QVector<T>(args){}
    JVector(int size, const T &t) : QVector<T>(size,t){}
    JVector(const QVector<T> &v) : QVector<T>(v){}

    //dot product is handy
    //if sizes arent the same then it does as much as it can
    JVector<T> &operator*=(const JVector<T> &other)
    {
        int m=qMin(this->size(),other.size());
        for(int k=0;k<m;k++)
        {
            this->operator[](k)*=other[k];
        }
        return *this;
    }

    //addition is handy too but changes QVector operator += witch is appending
    //if sizes arent the same then it does as much as it can
    JVector<T> &operator+=(const JVector<T> &other)
    {
        int m=qMin(this->size(),other.size());
        for(int k=0;k<m;k++)
        {
            this->operator[](k)+=other[k];
        }
        return *this;
    }

};

class Hann : public QVector<double>
{
public:
    Hann(int n)
    {
        fill(0,n);
        if(n==1)
        {
            operator[](0)=1;
            return;
        }
        for(int k=0;k<n;k++)
        {
            operator[](k)=0.5*(1.0-cos(2.0*M_PI*((double)(k))/((double)(n-1))));
        }
    }
};

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
    inline operator QVector<T>() const
    {
        return val;
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
    inline const QVector<int> &getMaxIndex(){return max_index;}
    //insertion operator
    inline const QVector<T> &operator<< (const QVector<T> &input)
    {
        return update(input);
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
    QVector<int> max_index;
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
    inline const QVector<int> &getMinIndex(){return min_index;}
    //insertion operator
    inline const QVector<T> &operator<< (const QVector<T> &input)
    {
        return update(input);
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
    inline const int &getStart(){return start;}
private:
    int m=0;
    int n=0;
    int start=0;
    QVector<T> min;
    QVector<int> min_index;
    QVector<QVector<T>> mv;
};

template <class T>
class VectorMovingMinWithAssociate : public VectorMovingMin<T>
{
public:
    VectorMovingMinWithAssociate(){}
    VectorMovingMinWithAssociate(const QPair<int,int> mn);
    const QVector<T> &update(const QVector<T> &input, const QVector<T> &associate_input);//an input vector of size m
    void setSize(const QPair<int,int> mn);
    inline const QVector<T> &getAssociate(){return associate;}
    //insertion operator
    inline const QVector<T> &operator<< (const QVector<T> &input)
    {
        update(input,QVector<T>(input.size()));
        return VectorMovingMin<T>::getMin();
    }
private:
    QVector<QVector<T>> mv_associate;
    QVector<T> associate;
    int m=0;
    int n=0;
};

class OverlappedRealFFT
{
public:
    OverlappedRealFFT();
    OverlappedRealFFT(int size);
    void setSize(int n);
    void update(const QVector<double> &input);
    QVector<JFFT::cpx_type> Xfull;
    QVector<double> Xabs;
private:
    QVector<double> buffer;
    QVector<double> x;
    QVector<double> window;
    JFFT fft;
    int n=0;
};

class MovingNoiseEstimator : public VectorMovingAverage<double>
{
public:
    MovingNoiseEstimator();
    MovingNoiseEstimator(int vector_width_m,int moving_stats_window_size,int moving_minimum_window_size,int output_moving_average_window_size);
    QVector<double> &update(const QVector<double> &input);
    void setSize(int vector_width_m,int moving_stats_window_size,int moving_minimum_window_size,int output_moving_average_window_size);
    //insertion operator
    inline QVector<double> &operator<< (const QVector<double> &input)
    {
        return update(input);
    }
protected:
    VectorMovingMinWithAssociate<double> mm;
    VectorMovingVariance<double> mv;
private:
    QVector<double> buffer;
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

//qdebug output for MovingNoiseEstimator
inline QDebug operator<<(QDebug debug, JDsp::MovingNoiseEstimator &obj)
{
    return QtPrivate::printSequentialContainer(debug, "Noise", obj.val);
}

//dot product for JVector
template<class T>
inline JDsp::JVector<T> operator*(JDsp::JVector<T> lhs, const JDsp::JVector<T> &rhs)
{
    return lhs *= rhs;
}
//addition for JVector. if you use QVector and JVector appending will be done
template<class T>
inline JDsp::JVector<T> operator+(JDsp::JVector<T> lhs, const JDsp::JVector<T> &rhs)
{
    return lhs += rhs;
}

//qdebug output for QVector<std::complex<double>>
inline QDebug operator<<(QDebug debug, const QVector<std::complex<double>> &obj)
{
    QDebugStateSaver saver(debug);
    debug.nospace()<<"QVector(";
    for(int k=0;k<obj.size();k++)
    {
        if(k==obj.size()-1)
        {
            if(obj[k].imag()>=0)debug.nospace()<<obj[k].real()<<"+"<<obj[k].imag()<<"i)";
            else debug.nospace()<<obj[k].real()<<obj[k].imag()<<"i)";
        }
        else
        {
            if(obj[k].imag()>=0)debug.nospace()<<obj[k].real()<<"+"<<obj[k].imag()<<"i, ";
            else debug.nospace()<<obj[k].real()<<obj[k].imag()<<"i, ";
        }
    }
    return debug;
}

#endif // DSP_H
