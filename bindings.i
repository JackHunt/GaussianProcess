%module pyGP
%{
#include "lib/typedefs.h"
#include "lib/Kernels.h"
#include "lib/GPRegressor.h"
%}

%include "swig/numpy.i"

%include "lib/typedefs.h"
%include "lib/Kernels.h"
%include "lib/GPRegressor.h"
