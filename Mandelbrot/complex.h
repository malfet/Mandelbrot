/*
 * Complex number template
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
    return out<<"Complex("<<c.re<<","<<c.im<<")"<<std::endl;
    
}

template<typename Ta,typename Tb> Complex<Tb> operator*(Ta a,const Complex<Tb> &b) {
    return Complex<Tb>(a*b.re,a*b.im);
}

template<typename T> Complex<T> exp(const Complex<T> &a) {
    return exp(a.re)*Complex<T>(cos(a.im),sin(a.im));
}

template<typename T> Complex<T> log(const Complex<T> &a) {
    return Complex<T>(log(a.mod()),a.fi());
}

template<typename Ta, typename Tb> Complex<Ta> pow(const Complex<Ta> a, Tb b) {
    return exp(log(a)*b);
}

#endif
