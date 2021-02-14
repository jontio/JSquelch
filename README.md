# JSquelch

![CI](https://github.com/jontio/JSquelch/workflows/CI/badge.svg)

Next step of the voice detection code.

This commit I have added VectorMovingMinWithAssociate OverlappedRealFFT and MovingNoiseEstimator.

OverlappedRealFFT takes time domain input and outputs frequency domain. It uses overlapping and padding and takes the abs value as described in the web page.

VectorMovingMinWithAssociate finds the min over a window and return that value as well as an associated value, again as described in the web page.

MovingNoiseEstimator is the main class and takes input from the output of the OverlappedRealFFT class and estimates the noise floor. It's used like so...

```C++
QVector<double> x={2,3,1,4};
JDsp::MovingNoiseEstimator mne(4,3,3,0);
for(int k=0;k<6;k++)
{
    mne<<x;//usually x would come from the OverlappedRealFFT output
    qDebug()<<mne;//the output of MovingNoiseEstimator is the noise estimate
}
```

The output of this is...

```
Noise(0, 0, 0, 0)
Noise(0, 0, 0, 0)
Noise(1.33333, 2, 0.666667, 2.66667)
Noise(1.76383, 2.64575, 0.881917, 3.52767)
Noise(2, 3, 1, 4)
Noise(2, 3, 1, 4)
```

