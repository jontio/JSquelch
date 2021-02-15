# JSquelch

![CI](https://github.com/jontio/JSquelch/workflows/CI/badge.svg)

Next step of the voice detection code.

This commit I have added VectorMovingMinWithAssociate OverlappedRealFFT and MovingNoiseEstimator.

OverlappedRealFFT takes time domain input and outputs frequency domain. It uses overlapping and padding and takes the abs value as described in the web page.

VectorMovingMinWithAssociate finds the min over a window and returns that value as well as an associated value, again as described in the web page.

MovingNoiseEstimator is the main class and takes input from the output of the OverlappedRealFFT class and estimates the noise floor. It's used like so...

```C++
//normally distributed numbers for the signal
std::default_random_engine generator(1);
std::normal_distribution<double> distribution(0.0,1.0);

JDsp::OverlappedRealFFT fft(3);//for an fft that takes 3 as in will output 2*3+1=7
QVector<double> x;//this would be the input signal split up into blocks
JDsp::MovingNoiseEstimator mne(fft.getOutSize(),16,16,32);
for(int k=0;k<640;k++)
{
    x={distribution(generator),distribution(generator),distribution(generator)};
    mne<<(fft<<x);
    if((k+1)%64)continue;
    qDebug()<<mne;
}
```

The output of this is...

```
Noise(1.00975, 1.01956, 1.04914, 1.10431, 1.12261, 1.13722, 1.16988)
Noise(1.22996, 1.22627, 1.23693, 1.23886, 1.25076, 1.29969, 1.30799)
Noise(1.00903, 1.07507, 1.19302, 1.30689, 1.363, 1.37509, 1.35586)
Noise(1.2335, 1.23451, 1.22991, 1.18543, 1.15753, 1.14204, 1.13511)
Noise(1.0106, 1.03077, 1.07481, 1.11599, 1.14903, 1.1484, 1.10748)
Noise(0.875575, 0.938062, 1.05466, 1.12679, 1.24008, 1.3088, 1.30957)
Noise(1.19341, 1.18137, 1.14924, 1.13763, 1.17851, 1.21613, 1.31142)
Noise(1.0965, 1.13382, 1.14679, 1.1303, 1.13517, 1.18505, 1.22808)
Noise(0.929295, 0.951046, 1.03854, 1.18444, 1.19993, 1.18092, 1.13159)
Noise(0.996635, 1.01634, 1.06267, 1.11054, 1.12922, 1.10954, 1.05887)
```

As you can see each frequency bin has a noise of about 1 which the same as the standard deviation of the signal in the time domain.


