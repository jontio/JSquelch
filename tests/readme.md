For creating the cpp/h files for testing from Matlab the following Matlab code was used for creating the files

```Matlab
function printcpp_complex_file(filename,name,x)
%write cpp file
fileID = fopen(filename+".cpp",'w');
fprintf(fileID,'#include \"%s\"\n',filename+".h");
fprintf(fileID,'typedef  JFFT::cpx_type ct;\n');
for k=1:numel(x)
    if(k==1)
        fprintf(fileID,"const QVector<JFFT::cpx_type> %s={",name);
    end
    fprintf(fileID,"ct(%f,%f)",real(x(k)),imag(x(k)));
    if(k<numel(x))
        fprintf(fileID,",");
        if(mod(k,100)==0)
            fprintf(fileID,"\\\n");
        end
    else
        fprintf(fileID,"};\n");
    end
end
fclose(fileID);
%write header
fileID = fopen(filename+".h",'w');
fprintf(fileID,'#include "../src/dsp/dsp.h\"\n');
fprintf(fileID,"extern const QVector<JFFT::cpx_type> %s;\n",name);
fclose(fileID);
end

function printcpp_real_file(filename,name,x)
%write cpp file
fileID = fopen(filename+".cpp",'w');
fprintf(fileID,'#include \"%s\"\n',filename+".h");
for k=1:numel(x)
    if(k==1)
        fprintf(fileID,"const QVector<double> %s={",name);
    end
    fprintf(fileID,"%.9f",x(k));
    if(k<numel(x))
        fprintf(fileID,",");
        if(mod(k,100)==0)
            fprintf(fileID,"\\\n");
        end
    else
        fprintf(fileID,"};\n");
    end
end
fclose(fileID);
%write header
fileID = fopen(filename+".h",'w');
fprintf(fileID,'#include "../src/dsp/dsp.h\"\n');
fprintf(fileID,"extern const QVector<double> %s;\n",name);
fclose(fileID);
end 
```
