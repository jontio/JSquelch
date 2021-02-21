#ifndef DSP_H
#define DSP_H

#include <QVector>
#include <QDebug>
#include <complex>
#include "../JFFT/jfft.h"
#include "../util/RuntimeError.h"

namespace JDsp
{

//not used
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

    //dot division is handy
    //if sizes arent the same then it does as much as it can
    //will return nan or +-inf if scaled badly
    JVector<T> &operator/=(const JVector<T> &other)
    {
        int m=qMin(this->size(),other.size());
        for(int k=0;k<m;k++)
        {
            this->operator[](k)/=other[k];
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

//    T division_tolerance=0;

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

class StrangeSineCorrectionWindow : public QVector<double>
{
public:
    StrangeSineCorrectionWindow(int n)
    {
        fill(0,n);
        double dn=n;
        for(int k=0;k<n;k++)
        {
            operator[](k)=std::abs(std::sin(M_PI*(((double)k)+0.5)/dn)/(1.2729*dn-1.1621))+1.0;
        }
    }
};

//getting really piss of at this point not going to test this
template <class T>
class VectorDelayLine : public QVector<T>
{
public:
    VectorDelayLine()
    {
        setSize(QPair<int,int>(0,0));
    }
    VectorDelayLine(const QPair<int,int> &mn)
    {
        setSize(mn);
    }
    void setSize(const QPair<int,int> &mn)
    {
        m=mn.first;
        n=mn.second;
        buffer_index=0;
        buffer.fill(QVector<T>(m),n);
    }
    QPair<int,int> getSize()
    {
        QPair<int,int> mn(m,n);
        return mn;
    }
    QVector<T> &update(const QVector<T> &input)
    {
        if(input.size()!=m)RUNTIME_ERROR("input vector size is not the expected size", input.size());
        if(n<=0)
        {
            QVector<T>::operator=(input);
            return *this;
        }
        QVector<T>::operator=(buffer[buffer_index]);
        buffer[buffer_index]=input;
        buffer_index++;buffer_index%=n;
        return *this;
    }
    inline VectorDelayLine &operator<< (const QVector<T> &input)
    {
        update(input);
        return *this;
    }
private:
    QVector<QVector<T>> buffer;
    int m=0;
    int n=0;
    int buffer_index=0;
};

template <class T>
class ScalarDelayLine
{
public:
    ScalarDelayLine()
    {
        setSize(0);
    }
    ScalarDelayLine(const int &n)
    {
        setSize(n);
    }
    void setSize(const int &n)
    {
        this->n=n;
        buffer_index=0;
        retval=0;
        buffer.fill(0,n);
    }
    T update(const T &input)
    {
        if(n<=0)
        {
            retval=input;
            return retval;
        }
        retval=(buffer[buffer_index]);
        buffer[buffer_index]=input;
        buffer_index++;buffer_index%=n;
        return retval;
    }
    void delay(T &input)
    {
        input=update(input);
    }
    inline ScalarDelayLine &operator<< (const T &input)
    {
        update(input);
        return *this;
    }
    inline operator T() const
    {
        return retval;
    }
private:
    T retval=0;
    QVector<T> buffer;
    int n=0;
    int buffer_index=0;
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
class MovingMax
{
public:
    MovingMax(){}
    MovingMax(int n);
    const T &update(const QVector<T> &input);
    const T &update(const T &input);
    void setSize(int n);
    int getSize();
    void flush();
    //do not change max after init else it wont work hence why const and no direct access
    inline const T &getMax(){return max;}
    //insertion operator
    inline const T &operator<< (const QVector<T> &input)
    {
        return update(input);
    }
    inline const T &operator<< (const T &input)
    {
        return update(input);
    }
    inline operator T() const
    {
        return max;
    }
protected:
    inline const QVector<T> &getMv(){return mv;}
private:
    int n=0;
    int start=0;
    T max;
    QVector<T> mv;
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
    OverlappedRealFFT(int size);//size needs to be a power of two else an error will be thrown when updating
    void setInSize(int n);
    int getInSize(){return n;}
    int getOutSize(){return (2*n+1);}
    QVector<double> &update(const QVector<double> &input);
    QVector<JFFT::cpx_type> Xfull;
    QVector<double> Xabs;
    //insertion operator
    inline OverlappedRealFFT &operator<< (const QVector<double> &input)
    {
        update(input);
        return *this;
    }
private:
    QVector<double> buffer;
    QVector<double> x;
    QVector<double> window;
    JFFT fft;
    int n=0;
};

//getting really pissed of at this point not going to test this
class OverlappedRealFFTDelayLine
{
    public:
    OverlappedRealFFTDelayLine()
    {
        delay_Xabs.setSize(QPair<int,int>(257,51));
        delay_Xfull.setSize(QPair<int,int>(512,51));
    }
    OverlappedRealFFTDelayLine(const OverlappedRealFFT &fft,int n)
    {
        setSize(fft,n);
    }
    void setSize(const OverlappedRealFFT &fft,int n)
    {
        delay_Xabs.setSize(QPair<int,int>(fft.Xabs.size(),n));
        delay_Xfull.setSize(QPair<int,int>(fft.Xfull.size(),n));
    }
    void delay(OverlappedRealFFT &fft)
    {
        fft.Xabs=delay_Xabs.update(fft.Xabs);
        fft.Xfull=delay_Xfull.update(fft.Xfull);
    }
private:
    VectorDelayLine<double> delay_Xabs;
    VectorDelayLine<JFFT::cpx_type> delay_Xfull;
};

class InverseOverlappedRealFFT : public QVector<double>
{
public:
    InverseOverlappedRealFFT();
    InverseOverlappedRealFFT(int size);//size needs to be a power of two else an error will be thrown when updating
    void setOutSize(int n);
    int getInSize(){return (4*n);}
    int getOutSize(){return n;}
    QVector<double> &update(const QVector<JFFT::cpx_type> &input);
    //insertion operators
    inline InverseOverlappedRealFFT &operator<< (const QVector<JFFT::cpx_type> &input)
    {
        update(input);
        return *this;
    }
    inline InverseOverlappedRealFFT &operator<< (const OverlappedRealFFT &input)
    {
        update(input.Xfull);
        return *this;
    }
    inline InverseOverlappedRealFFT &operator/= (const double scalar)
    {
        for(int k=0;k<QVector<double>::size();k++)
        {
            QVector<double>::operator[](k)/=scalar;
        }
        return *this;
    }
    inline InverseOverlappedRealFFT &operator= (const QVector<double> x)
    {
        QVector<double>::operator=(x);
        return *this;
    }
private:
    QVector<double> window;
    QVector<double> x,x_part_last;
    JFFT fft;
    int n=0;
};

class InverseOverlappedRealFFTDelayLine
{
    public:
    InverseOverlappedRealFFTDelayLine()
    {
        setSize(8000);
    }
    InverseOverlappedRealFFTDelayLine(int n)
    {
        setSize(n);
    }
    void setSize(int n)
    {
        this->n=n;
        buffer_index=0;
        buffer.fill(0,n);
    }
    void delay(InverseOverlappedRealFFT &ifft)
    {
        if(n<=0)return;
        for(int k=0;k<ifft.size();k++)
        {
            double retval=(buffer[buffer_index]);
            buffer[buffer_index]=ifft[k];
            ifft[k]=retval;
            buffer_index++;buffer_index%=n;
        }
    }
private:
    QVector<double> buffer;
    int n=0;
    int buffer_index=0;
};

class MovingNoiseEstimator : public VectorMovingAverage<double>
{
public:
    MovingNoiseEstimator();
    MovingNoiseEstimator(int vector_width_m,int moving_stats_window_size,int moving_minimum_window_size,int output_moving_average_window_size);
    QVector<double> &update(const QVector<double> &input);
    void setSize(int vector_width_m,int moving_stats_window_size,int moving_minimum_window_size,int output_moving_average_window_size);
    //insertion operators
    inline MovingNoiseEstimator &operator<< (const QVector<double> &input)
    {
        update(input);
        return *this;
    }
    inline MovingNoiseEstimator &operator<< (OverlappedRealFFT &input)
    {
        update(input.Xabs);
        return *this;
    }
protected:
    VectorMovingMinWithAssociate<double> mm;
    VectorMovingVariance<double> mv;
private:
    QVector<double> buffer;
};

class Normalize
{
public:
    Normalize();
    Normalize(int size);
    QVector<double> &update(const QVector<double> &input);
    void setSize(int size);
    //insertion operator
protected:
    VectorMovingMinWithAssociate<double> mm;
    VectorMovingVariance<double> mv;
private:
    QVector<double> buffer;
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

class MovingSignalEstimator : public VectorMovingAverage<double>
{
public:
    MovingSignalEstimator();
    MovingSignalEstimator(int vector_width_m,int moving_stats_window_size,int min_voice_bin,int max_voice_bin,int output_moving_voice_snr_estimate_window_size,int output_moving_average_window_size);
    void setSize(int vector_width_m,int moving_stats_window_size,int min_voice_bin,int max_voice_bin,int output_moving_voice_snr_estimate_window_size,int output_moving_average_window_size);
    QVector<double> &update(const QVector<double> &input);//expects the input to be normalized first
    //insertion operators
    inline MovingSignalEstimator &operator<< (const QVector<double> &input)
    {
        update(input);
        return *this;
    }
    inline MovingSignalEstimator &operator<< (OverlappedRealFFT &input)
    {
        update(input.Xabs);
        return *this;
    }
    double voice_snr_estimate=0;
private:
    VectorMovingVariance<double> moving_stats;
    MovingAverage<double> ma;
    int start_index=0;
    int end_index=0;
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

//dot devision of QVector<T>
template<class T>
inline QVector<T> &operator/=(QVector<T> &a, const QVector<T> &b)
{
    int m=qMin(a.size(),b.size());
    for(int k=0;k<m;k++)
    {
        a[k]/=b[k];
    }
    return a;
}
//dot devision for normalization using the noise estimate
inline QVector<double> &operator/=(QVector<double> &a, const JDsp::MovingNoiseEstimator &b)
{
    if(a.size()!=b.val.size())RUNTIME_ERROR("dot devision by noise estimate need to have the same sizes", b.val.size());
    a/=b.val;
    return a;
}
//dot devision for normalization of everything in OverlappedRealFFT using the noise estimate
inline JDsp::OverlappedRealFFT &operator/=(JDsp::OverlappedRealFFT &a, const JDsp::MovingNoiseEstimator &b)
{
    //divide all the real abs vals
    //will check for size errors
    a.Xabs/=b;
    //divide the first part of complex numbers from dc to nyquist inclusive
    for(int k=0;k<b.val.size();k++)
    {
        a.Xfull[k]/=b.val[k];
    }
    //the rest is complex conjugate stuff so dont have to divide
    auto p=a.Xfull.end();
    for(int k=1;k<(b.val.size()-1);k++)
    {
        p--;
        *p=std::conj(a.Xfull[k]);
    }
    return a;
}

//dot product of QVector<T>
template<class T>
inline QVector<T> &operator*=(QVector<T> &a, const QVector<T> &b)
{
    int m=qMin(a.size(),b.size());
    for(int k=0;k<m;k++)
    {
        a[k]*=b[k];
    }
    return a;
}
//dot product for signal estimation scalling using the signal estimate
inline QVector<double> &operator*=(QVector<double> &a, const JDsp::MovingSignalEstimator &b)
{
    if(a.size()!=b.val.size())RUNTIME_ERROR("dot product by signal estimate need to have the same sizes", b.val.size());
    a*=b.val;
    return a;
}
//dot product for signal scaling of everything in OverlappedRealFFT using the signal estimate
inline JDsp::OverlappedRealFFT &operator*=(JDsp::OverlappedRealFFT &a, const JDsp::MovingSignalEstimator &b)
{
    //divide all the real abs vals
    //will check for size errors
    a.Xabs*=b;
    //multily the first part of complex numbers from dc to nyquist inclusive
    for(int k=0;k<b.val.size();k++)
    {
        a.Xfull[k]*=b.val[k];
    }
    //the rest is complex conjugate stuff so dont have to divide
    auto p=a.Xfull.end();
    for(int k=1;k<(b.val.size()-1);k++)
    {
        p--;
        *p=std::conj(a.Xfull[k]);
    }
    return a;
}

#endif // DSP_H
