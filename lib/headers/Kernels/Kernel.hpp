/*
BSD 3-Clause License

Copyright (c) 2018, Jack Miles Hunt
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

* Neither the name of the copyright holder nor the names of its
  contributors may be used to endorse or promote products derived from
  this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef GPLIB_KERNEL_HEADER
#define GPLIB_KERNEL_HEADER

#include <Aliases.hpp>

#include <vector>
#include <algorithm>
#include <iostream>
#include <cmath>

#include <Eigen/Dense>

namespace GPLib::Kernels {
    //Available kernel types enumerated here.
    enum class KernelType : short {
        SQUARED_EXPONENTIAL
    };

    template<typename T>
    class Kernel {
    private:
        void verifyParams() const;

    protected:
        ParameterSet<T> params;
        std::vector< std::string > validParams;

	protected:
		void verifyParam(const std::string &var) const;

    public:
        Kernel(const std::vector< std::string > &validParams, const ParameterSet<T> &params);

        virtual ~Kernel();

        virtual T f(const Vector<T> &a, const Vector<T> &b) const = 0;

        virtual T df(const Vector<T> &a, const Vector<T> &b, const std::string &gradVar) const = 0;

        virtual Vector<T> dfda(const Vector<T> &a, const Vector<T> &b) const = 0;
        
        virtual Vector<T> dfdb(const Vector<T> &a, const Vector<T> &b) const = 0;
    };
}

#endif
