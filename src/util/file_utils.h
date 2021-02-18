#ifndef FILE_UTILS_H
#define FILE_UTILS_H
#include <QVector>
#include "../dsp/dsp.h"


namespace File_Utils
{

namespace Matlab
{
void print(const char *filename,const char *name,const QVector<JFFT::cpx_type> &result);
void print(const char *filename,const char *name,const QVector<double> &result);
}

namespace CPP
{
void print(const char *filename,const char *name,const QVector<JFFT::cpx_type> &result);
void print(const char *filename,const char *name,const QVector<double> &result);
}

}

#endif // FILE_UTILS_H
 
