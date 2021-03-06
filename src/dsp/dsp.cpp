#include "dsp.h"
#include <math.h>
#include "../util/RuntimeError.h"

namespace JDsp
{

//----start of AGC

AGC::AGC()
{
    setSettings(settings);
}

void AGC::setSettings(const Settings &settings)
{
    this->settings=settings;
    Dx.setSize(settings.moving_max_window_size);
    delayline.setSize(settings.delayline_size);
    max_g=std::log(settings.max_gain);
    A=2.0*std::log(settings.agc_level);//use this for moving max
//    A=2.0*std::log(settings.agc_level/std::sqrt(2.0));//use this for movinf mean
    g=0;
}

void AGC::update(QVector<double> &input,bool adjust)
{
    for(int n=0;n<input.size();n++)
    {
        double val=input[n];
        Dx.update(val*val);
        delayline.delay(val);
        double y=val*std::exp(g);
        double z=Dx*std::exp(2.0*g);
        double e=A-std::log(z);
        if(e>100)e=100;
        if(e<-100)e=-100;
        if(std::isnan(e))e=0;
        if(adjust)g=g+settings.K*e;
        if(g>max_g)g=max_g;
        input[n]=y;
    }
}

//----end of AGC

//----start of MovingMax

//define class for int, double.
template class MovingMax<int>;
template class MovingMax<double>;

template <class T>
MovingMax<T>::MovingMax(int n)
{
    setSize(n);
}
template <class T>
const T &MovingMax<T>::update(const QVector<T> &input)
{
    //find max non nan value input
    auto it= std::max_element(input.begin(),input.end(),[](T x, T y)
    {
            return ((x < y) ? true : std::isnan(x));
    });
    T max_in_vector  = *it;
//        int max_index = std::distance(input.begin(), it);
//        qDebug()<<"max_index="<<max_index;
//        qDebug()<<"max_in_vector="<<max_in_vector;
    //update window
    if(std::isnan(max_in_vector)) return max;
    if(n==0)
    {
        max=max_in_vector;
        return max;
    }
    if(max_in_vector>max)
    {
        max=max_in_vector;
    }
    double outgoing=mv[start];
    mv[start]=max_in_vector;
    if(outgoing>=max)
    {
        max=*std::max_element(mv.begin(),mv.end());
    }
    start++;start%=n;
    return max;
}
template <class T>
const T &MovingMax<T>::update(const T &input)
{
    if(std::isnan(input))return max;
    if(n==0)
    {
        max=input;
        return max;
    }
    if(input>max)
    {
        max=input;
    }
    double outgoing=mv[start];
    mv[start]=input;
    if(outgoing>=max)
    {
        max=*std::max_element(mv.begin(),mv.end());
    }
    start++;start%=n;
    return max;
}
template <class T>
void MovingMax<T>::setSize(int n)
{
    this->n=n;
    if(n<0)RUNTIME_ERROR("window size needs to be non-negative", n);
    flush();
}
template <class T>
int MovingMax<T>::getSize()
{
    return n;
}
template <class T>
void MovingMax<T>::flush()
{
    mv.fill(0,n);
    start=0;
    max=0;
}

//----end of MovingMax

//----start of InverseOverlappedRealFFT

InverseOverlappedRealFFT::InverseOverlappedRealFFT()
{
    setOutSize(128);
}
InverseOverlappedRealFFT::InverseOverlappedRealFFT(int size)//size needs to be a power of two else an error will be thrown when updating
{
    setOutSize(size);
}
void InverseOverlappedRealFFT::setOutSize(int n)
{
    this->n=n;
    //eg n=128 -> NFFT is 512 (input size), Xabs from fft is 257, window from fft is 256, outsize is 128
    x.fill(0,4*n);
    QVector<double>::fill(0,n);
    x_part_last.fill(0,n);
    window=JDsp::StrangeSineCorrectionWindow(n);
}
QVector<double> &InverseOverlappedRealFFT::update(const QVector<JFFT::cpx_type> &input)
{
    //do ifft
    if((n==0)||((n&(n-1))!=0))RUNTIME_ERROR("ifft size needs to be a power of 2", n);
    if(input.size()!=4*n)RUNTIME_ERROR("input vector size is not the expected size", n);
    fft.ifft_real(const_cast<QVector<JFFT::cpx_type>&>(input),x);//um yup not so good
    //this should not happen if sizes are ok before calling fft
    if(input.size()!=4*n)RUNTIME_ERROR("ifft changed input size", input.size());
    if(x.size()!=4*n)RUNTIME_ERROR("ifft changed output size", x.size());

    //overlap and add
    QVector<double>::operator=(x.mid(0,n));//oldest part of this block
    if(x_part_last.size()!=n)RUNTIME_ERROR("x_part_last wrong size", x_part_last.size());
    for(int k=0;k<n;k++)QVector<double>::operator[](k)+=x_part_last[k];//overlap last with this
    x_part_last=x.mid(n,n);//newest part of this block for next time

    //this inverse isn't exact and has a bit of a bump every n samples.
    //it's about 0.6% at n=128 samples. it gets worse for small n and better for large n
    //the bump is about abs(sin(pi*(x-0.5)/128)/161.8)+1.0 in matlab indexing for 128
    //generally it's about abs(sin(pi*(x-0.5)/n)/(1.2729*n-1.1621))+1.0 but this approximation
    //gets worse for small n and it's a bit flat between bumps.
    //allthough not large for n=128 this window operation below will reduce these bumps
    for(int k=0;k<n;k++)QVector<double>::operator[](k)*=window[k];
    //for n=128 the error is improved by about 2 orders of manitude
    //when using the windowing reducing the error to 0.004%

    return *this;
}

//----end of InverseOverlappedRealFFT

//----start of MovingSignalEstimator

MovingSignalEstimator::MovingSignalEstimator()
{
    setSize(257,8,3,96,62,8);
}
MovingSignalEstimator::MovingSignalEstimator(int vector_width_m,int moving_stats_window_size,int min_voice_bin,int max_voice_bin,int output_moving_voice_snr_estimate_window_size,int output_moving_average_window_size)
{
    setSize(vector_width_m,moving_stats_window_size,min_voice_bin,max_voice_bin,output_moving_voice_snr_estimate_window_size,output_moving_average_window_size);
}
void MovingSignalEstimator::setSize(int vector_width_m,int moving_stats_window_size,int min_voice_bin,int max_voice_bin,int output_moving_voice_snr_estimate_window_size,int output_moving_average_window_size)
{
    moving_stats.setSize(QPair<int,int>(vector_width_m,moving_stats_window_size));
    ma.setSize(output_moving_voice_snr_estimate_window_size);
    VectorMovingAverage<double>::setSize(QPair<int,int>(vector_width_m,output_moving_average_window_size));
    voice_snr_estimate=0;
    start_index=min_voice_bin;
    end_index=max_voice_bin;
    if((start_index>=vector_width_m)||(start_index<0))RUNTIME_ERROR("start_index out of bounds", start_index);
    if((end_index>=vector_width_m)||(end_index<0))RUNTIME_ERROR("end_index out of bounds", end_index);
    if(end_index<start_index)RUNTIME_ERROR("end_index is less than start_index", end_index);
}
QVector<double> &MovingSignalEstimator::update(const QVector<double> &input)//expects the input to be normalized first
{
    moving_stats.update(input);
    const VectorMovingAverage<double> &mean=moving_stats.mean();
    for(int k=0;k<moving_stats.val.size();k++)
    {
        moving_stats.val[k]=mean[k]*mean[k]+moving_stats.val[k]-1.0;
    }
    voice_snr_estimate=*std::max_element(moving_stats.val.begin()+start_index,moving_stats.val.begin()+end_index);
    voice_snr_estimate=ma.update(voice_snr_estimate);
    return VectorMovingAverage<double>::update(moving_stats.val);
}

//----end of MovingSignalEstimator

//----start of MovingNoiseEstimator

MovingNoiseEstimator::MovingNoiseEstimator()
{
    setSize(257,16,16,62);
}
MovingNoiseEstimator::MovingNoiseEstimator(int vector_width_m,int moving_stats_window_size,int moving_minimum_window_size,int output_moving_average_window_size)
{
    setSize(vector_width_m,moving_stats_window_size,moving_minimum_window_size,output_moving_average_window_size);
}
QVector<double> &MovingNoiseEstimator::update(const QVector<double> &input)
{
    if(input.size()!=val.size())RUNTIME_ERROR("input vector size is not the expected size", input.size());
    mv<<input;
    mm.update(mv.mean(),mv);
    //%calculate normalizing factor over a window
    //%the bigger this window is the less agile the noise estimateion will be
    //moving_sigma_estimate=1.062*(sqrt((Zmin_std).^2+(Zmin_mean).^2));
    //moving_sigma_estimate=movmean(moving_sigma_estimate,62);
    for(int k=0;k<buffer.size();k++)buffer[k]=sqrt(mm[k]*mm[k]+mm.getAssociate()[k]);
    VectorMovingAverage<double>::update(buffer);
    return VectorMovingAverage<double>::val;
}
void MovingNoiseEstimator::setSize(int vector_width_m,int moving_stats_window_size,int moving_minimum_window_size,int output_moving_average_window_size)
{
    mv.setSize(QPair<int,int>(vector_width_m,moving_stats_window_size));
    mm.setSize(QPair<int,int>(vector_width_m,moving_minimum_window_size));
    buffer.fill(0,vector_width_m);
    VectorMovingAverage<double>::setSize(QPair<int,int>(vector_width_m,output_moving_average_window_size));
}

//----end of MovingNoiseEstimator

//----start of VectorMovingMinWithAssociate

//define class for int, double.
template class VectorMovingMinWithAssociate<int>;
template class VectorMovingMinWithAssociate<double>;

template <class T>
VectorMovingMinWithAssociate<T>::VectorMovingMinWithAssociate(const QPair<int,int> mn)
{
    setSize(mn);
}
template <class T>
const QVector<T> &VectorMovingMinWithAssociate<T>::update(const QVector<T> &input, const QVector<T> &associate_input)//an input vector of size m
{
    int org_start=VectorMovingMin<T>::getStart();
    VectorMovingMin<T>::update(input);
    for(int k=0;k<m;k++)
    {
        mv_associate[k][org_start]=associate_input[k];
        associate[k]=mv_associate[k][VectorMovingMin<T>::getMinIndex()[k]];
    }
    return VectorMovingMin<T>::getMin();
}
template <class T>
void VectorMovingMinWithAssociate<T>::setSize(const QPair<int,int> mn)
{
    m=mn.first;
    n=mn.second;
    mv_associate.resize(m);
    for(int k=0;k<mv_associate.size();k++)mv_associate[k].resize(n);
    associate.fill(0,m);
    VectorMovingMin<T>::setSize(mn);
}

//----end of VectorMovingMinWithAssociate


//----start of OverlappedRealFFT

OverlappedRealFFT::OverlappedRealFFT()
{
    setInSize(128);
}
OverlappedRealFFT::OverlappedRealFFT(int size)
{
    setInSize(size);
}
void OverlappedRealFFT::setInSize(int n)
{

    this->n=n;
    //eg n=128 -> so window is 256, NFFT is 512 and Xabs is 257
    window=JDsp::Hann(2*n);
    x.fill(0,4*n);
    Xfull.fill(0,4*n);
    Xabs.fill(0,2*n+1);
    buffer.fill(0,2*n);
}
QVector<double> &OverlappedRealFFT::update(const QVector<double> &input)
{
    if((n==0)||((n&(n-1))!=0))RUNTIME_ERROR("fft size needs to be a power of 2", n);
    if(input.size()!=n)RUNTIME_ERROR("input vector size is not the expected size", n);
    //shift down and add new buffer
    for(int k=n;k<(2*n);k++)
    {
        buffer[k-n]=buffer[k];
        buffer[k]=input[k-n];
    }
    //hann and pad
    //xb=sig_test(k:k+256-1).*hann(256)
    //xb=[xb;zeros(numel(xb),1)]
    for(int k=0;k<(2*n);k++)x[k]=buffer[k]*window[k];//fill first half and window
    for(int k=(2*n);k<(4*n);k++)x[k]=0;//zero the rest
    //fft dont worry about scaling
    //XB=fft(xb)./scale;
    fft.fft_real(x,Xfull);
    //this should not happen if sizes are ok before calling fft
    if(x.size()!=4*n)RUNTIME_ERROR("fft changed input size", x.size());
    if(Xfull.size()!=4*n)RUNTIME_ERROR("fft changed output size", Xfull.size());
    //only 257 abs points are needed for reconstitution
    for(int k=0;k<(2*n+1);k++)Xabs[k]=std::abs(Xfull[k]);

    return Xabs;
}

//----end of OverlappedRealFFT

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
        if(isnan(input[k]))continue;
        if(input[k]>max[k])
        {
            max[k]=input[k];
            max_index[k]=start;
        }
        double outgoing=mv[k][start];
        mv[k][start]=input[k];
        if(outgoing>=max[k])
        {
            max_index[k]=std::max_element(mv[k].begin(),mv[k].end())-mv[k].begin();
            max[k]=mv[k][max_index[k]];
        }
    }
    start++;start%=n;

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
    max_index.resize(m);
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
    max_index.fill(0);
    for(int k=0;k<mv.size();k++)mv[k].fill(0);
    start=0;
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
        if(isnan(input[k]))continue;
        if(input[k]<min[k])
        {
            min[k]=input[k];
            min_index[k]=start;
        }
        double outgoing=mv[k][start];
        mv[k][start]=input[k];
        if(outgoing<=min[k])
        {
            min_index[k]=std::min_element(mv[k].begin(),mv[k].end())-mv[k].begin();
            min[k]=mv[k][min_index[k]];
        }
    }
    start++;start%=n;

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
    min_index.resize(m);
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
    min_index.fill(0);
    for(int k=0;k<mv.size();k++)mv[k].fill(0);
    start=0;
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
