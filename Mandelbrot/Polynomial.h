//
//  Polynomial.h
//  Mandelbrot
//
//  Created by Shulga Nikita on 2015/04/25.
//  Copyright (c) 2015年 Home. All rights reserved.
//

#ifndef Mandelbrot_Polynomial_h
#define Mandelbrot_Polynomial_h

#include <vector>
#include <algorithm>
#include <assert.h>

template<typename T> bool isZero(T x) { return x == 0; }
template<> bool isZero<float>(float x) { return fabs(x)<1e-6;}
template<> bool isZero<double>(double x) { return fabs(x)<1e-10;}
template<> bool isZero<std::complex<float> >(std::complex<float> x) { return std::abs(x)<1e-3;}
template<> bool isZero<std::complex<double> >(std::complex<double> x) { return std::abs(x)<1e-6;}


template<typename T> bool isNegative(const T &x) { return x < 0; }
template<> bool isNegative<std::complex<float> >(const std::complex<float> &x) { return std::abs(x) < 0; }
template<> bool isNegative<std::complex<double> >(const std::complex<double> &x) { return std::abs(x) < 0; }


template<typename T> class Polynomial {
public:
    Polynomial(T c=T(0.)):coefficients(1,c) {}
    Polynomial(const std::initializer_list<T> l): coefficients(l) {}
    Polynomial(const std::vector<T> l): coefficients(l) {}
    
    bool isRoot(const T x) const { return isZero(operator()(x));}
 
    template<typename T1> T1 operator()(const T1 x) const {
        T1 rc = 0;
        T1 y(1);
        for(auto c:coefficients) {
            rc += c*y;
            y *= x;
        }
        return rc;
    }
    size_t degree() const { return coefficients.size()-1;}
    typename std::vector<T>::const_iterator begin() const { return coefficients.begin(); }
    typename std::vector<T>::const_iterator end() const { return coefficients.end(); }
    
    const Polynomial &operator/=(const T &b) {
        std::transform(coefficients.begin(), coefficients.end(), coefficients.begin(), [b]( T x) { return x/b;});
        return *this;
    }
    /* Deflate polynomial by its root */
    Polynomial deflate(T r) const {
        assert (isRoot(r));
        assert (degree()>0);
        if (degree()==1) return Polynomial(coefficients[1]);
        /*Implement Ruffini's rule*/
        std::vector<T> nc(degree());
        auto cit = coefficients.rbegin();
        auto it = nc.rbegin();
        T prev = 0;
        while (it != nc.rend())
            prev = *(it++) = *(cit++)+prev*r;
        auto s = *cit+prev*r;
        assert (isZero(s));
        return Polynomial(nc);
    }
    
    Polynomial derivative() const {
        if (degree()==0) return Polynomial();
        std::vector<T> nc(degree());
        for(unsigned i=0;i<degree();++i)
            nc[i]=T(i+1)*coefficients[i+1];
        return Polynomial(nc);
        
    }

    T operator[](size_t i) const { return i < coefficients.size() ? coefficients[i] : 0; }
    static Polynomial<T> x;
private:
    std::vector<T> coefficients;
};

template<typename T> Polynomial<T> Polynomial<T>::x({0,1});

template<typename T1, typename T2> Polynomial<T1> operator*(const T2 a, const Polynomial<T1> &b) {
    std::vector<T1> nc(b.degree()+1);
    for(unsigned i=0; i<=b.degree(); ++i)
        nc[i]=a*b[i];
    return Polynomial<T1>(nc);
}

template<typename T> Polynomial<T> operator+(const Polynomial<T> &a, const Polynomial<T> &b) {
    auto newDegree = std::max(a.degree(), b.degree());
    std::vector<T> nc(newDegree+1);
    for(unsigned i=0; i <= newDegree; ++i)
        nc[i] = a[i]+b[i];
    return Polynomial<T>(nc);
}

template<typename T1, typename T2> Polynomial<T1> operator+(const Polynomial<T1> &a, const T2 &b) {
    std::vector<T1> nc(a.begin(), a.end());
    nc[0] += b;
    return Polynomial<T1>(nc);
}

template<typename T1, typename T2> Polynomial<T1> operator-(const Polynomial<T1> &a, const T2 &b) {
    std::vector<T1> nc(a.begin(), a.end());
    nc[0] -= b;
    return Polynomial<T1>(nc);
}

template<typename T> Polynomial<T> operator-(const Polynomial<T> &a, const Polynomial<T> &b) {
    auto newDegree = std::max(a.degree(), b.degree());
    std::vector<T> nc(newDegree+1);
    for(unsigned i=0; i <= newDegree; ++i)
        nc[i] = a[i]-b[i];
    return Polynomial<T>(nc);
}

template<typename T> Polynomial<T> operator*(const Polynomial<T> &a, const Polynomial<T> &b) {
    auto newDegree = a.degree()+b.degree();
    std::vector<T> nc(newDegree+1,0);
    for(unsigned i=0; i<= a.degree(); ++i)
        for (unsigned j=0; j <= b.degree(); ++j)
            nc[i+j] += a[i]*b[j];
    return Polynomial<T>(nc);
}

template<typename T> Polynomial<T> operator/(const Polynomial<T> &a, const T &b) {
    std::vector<T> nc(a.degree()+1);
    std::transform(a.begin(), a.end(), nc.begin(), [b](T x) { return x/b;});
    return Polynomial<T>(nc);
}


template<typename T> std::ostream &operator<<(std::ostream &os, const Polynomial<T> &p) {
    bool printed = false;
    
    for(auto i(p.degree());i>1; i--)
        if (p[i]!=T(0)) {
            if (printed && !isNegative(p[i])) os<<"+";
            if (p[i] != T(1.0))
                os<<p[i];
            os<<"x^"<<i;
            printed = true;
        }
    
    if (p.degree()>0 && p[1] != T(0)) {
        if (printed && !isNegative(p[1])) os<<"+";
        if (p[1] != T(1.0))
            os<<p[1];
        os<<"x";
        printed = true;
    }
    
    if (p[0]!=T(0) || !printed) {
        if (printed && !isNegative(p[0])) os<<"+";
        os<<p[0];
    }
    return os;
}

template<typename T> T findRootLaguerre(const Polynomial<T> &p, T x0=0) {
    T x = x0;
    auto n = T(p.degree());
    auto nminus1 = n - T(1);
    auto derP = p.derivative();
    auto der2P = derP.derivative();

    std::cout<<"findRoot(p="<<p<<",x0="<<x0<<"): derP"<<derP<<" der2P="<<der2P<<std::endl;
    while (!p.isRoot(x)) {
        auto px = p(x);
        auto G = derP(x)/px;
        auto H = G*G-der2P(x)/px;
        auto s = sqrt(nminus1*(n*H-G*G));
        auto a = abs(G+s)>abs(G-s)? n/(G+s) : n/(G-s);
        //std::cout<<"p("<<x<<")="<<px<<" G="<<G<<" H="<<H<<" a="<<a<<std::endl;
        x-=a;
    }
    std::cout<<"Found root "<<x<<std::endl;

    return x;
}

#endif
