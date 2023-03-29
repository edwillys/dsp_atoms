#pragma once

#include <cmath>
#include <limits>

#ifdef USE_OPT_LOG
#define LOG std::log
#else
#define LOG std::log // TODO
#endif

#ifdef USE_OPT_EXP
#define EXP std::exp
#else
#define EXP std::exp // TODO
#endif

namespace NAtomHelper
{
    /**
     * @brief Evaluation of the Omega function for real values.
     *        Based on the implementation in
     *        https://github.com/scipy/scipy/blob/main/scipy/special/wright.cc
     *        Modified, adapted and optimized by Edgar Lubicz
     *
     *        License:
     *        Also published as ACM TOMS 917; relicensed as BSD by the author.
     *        Copyright (C) Piers Lawrence.
     *        All rights reserved.
     *        Redistribution and use in source and binary forms, with or without
     *        modification, are permitted provided that the following conditions are met:
     *        1. Redistributions of source code must retain the above copyright notice, this
     *        list of conditions and the following disclaimer.
     *        2. Redistributions in binary form must reproduce the above copyright notice,
     *        this list of conditions and the following disclaimer in the documentation
     *        and/or other materials provided with the distribution.
     *        3. Neither the name of the copyright holder nor the names of its contributors
     *        may be used to endorse or promote products derived from this software without
     *        specific prior written permission.
     *        THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
     *        AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
     *        IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
     *        DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
     *        FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
     *        DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
     *        SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
     *        CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
     *        OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
     *        OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
     *
     * @tparam T
     * @param x
     * @return T
     */
    template <class T>
    inline T WrightOmegaReal(T x)
    {
        T w, wp1, e, r, temp;

        // Skipping tests for NaN and inf

        if (x < -50.0)
        {
            /*
             * Skip the iterative scheme because it exp(x) is already
             * accurate to double precision.
             */
            w = EXP(x);
        }
        else if (x > 1e20)
        {
            /*
             * Skip the iterative scheme because the result is just x to
             * double precision
             */
            w = x;
        }
        else
        {
            /* Split int three distinct intervals (-inf,-2), [-2,1), [1,inf) */
            if (x < -2.0)
            {
                /* exponential is approx < 1.3e-1 accurate */
                w = EXP(x);
            }
            else if (x < 1)
            {
                /* on [-2,1) approx < 1.5e-1 accurate */
                w = EXP(static_cast<T>(2.0) * (x - static_cast<T>(1.0)) /
                        static_cast<T>(3.0));
            }
            else
            {
                /* infinite series with 2 terms approx <1.7e-1 accurate */
                w = LOG(x);
                w = x - w + w / x;
            }

            /* Iteration one of Fritsch, Shafer, and Crowley (FSC) iteration */
            r = x - w - LOG(w);
            wp1 = w + static_cast<T>(1.0);
            temp = static_cast<T>(2.0) * wp1 * (wp1 + static_cast<T>(2.0) / static_cast<T>(3.0) * r);
            e = r / wp1 * (temp - r) / (temp - static_cast<T>(2.0) * r);
            w = w * (static_cast<T>(1.0) + e);

            /* Iteration two (if needed based on the condition number) */
            // TODO: should we simply to the 2nd step regardless?
            const T eps = std::numeric_limits<T>::epsilon();
            if (fabs((2.0 * w * w - 8.0 * w - 1.0) * pow(fabs(r), 4.0)) >= eps * 72.0 * pow(fabs(wp1), 6.0))
            {
                r = x - w - LOG(w);
                wp1 = w + static_cast<T>(1.0);
                temp = static_cast<T>(2.0) * wp1 * (wp1 + static_cast<T>(2.0) / static_cast<T>(3.0) * r);
                e = r / wp1 * (temp - r) / (temp - static_cast<T>(2.0) * r);
                w = w * (static_cast<T>(1.0) + e);
            }
        }

        return w;
    }
}