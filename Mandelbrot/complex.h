/*
 * Pre C++11 complex number template
 * 
 * Copyright (c) 2015 Nikita Shulga
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
 * IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER
 * OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef Mandelbrot_complex_h
#define Mandelbrot_complex_h

#include <math.h>
#include <iostream>

template<typename T> class Complex {
public:
    Complex(T r=0, T i=0):re(r), im(i) {}
    inline T mod2() const { return re*re+im*im; }
    inline T mod() const { return sqrt(mod2()); }
    inline T fi() const { return atan2(im,re); }
    
    const Complex operator+(const Complex &arg) const {
        return Complex(re+arg.re,im+arg.im);
    }
    const Complex operator-(const Complex &arg) const {
        return Complex(re-arg.re,im-arg.im);
    }

    const Complex operator*(const Complex &arg) const {
        return Complex(re*arg.re-im*arg.im,
                       im*arg.re+arg.im*re);
    }
    T re,im;
    static Complex<T> i;
};

template<typename T> std::ostream& operator<<(std::ostream &out, const Complex<T> &c) {
    return out<<"Complex("<<c.re<<","<<c.im<<")";
    
}

template<typename Ta,typename Tb> Complex<Tb> operator*(Ta a,const Complex<Tb> &b) {
    return Complex<Tb>(a*b.re,a*b.im);
}

template<typename Ta,typename Tb> Complex<Tb> operator*(const Complex<Tb> &b,Ta a) {
    return Complex<Tb>(a*b.re,a*b.im);
}


template<typename T> Complex<T> exp(const Complex<T> &a) {
    return exp(a.re)*Complex<T>(cos(a.im),sin(a.im));
}

template<typename T> Complex<T> log(const Complex<T> &a) {
    return Complex<T>(log(a.mod()),a.fi());
}

template<typename Ta, typename Tb> Complex<Ta> pow(const Complex<Ta> a, Tb b) {
    return a.mod2()!=0 ? exp(log(a)*b) : Complex<Ta>();
}

#endif
