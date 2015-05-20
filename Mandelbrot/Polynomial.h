/*
 * Polynomial template and helper functions
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

#ifndef Mandelbrot_Polynomial_h
#define Mandelbrot_Polynomial_h

#include <vector>
#include <algorithm>
#include <assert.h>

template<typename T> bool isZero(T x) { return x == 0; }
template<> bool isZero<float>(float x) { return fabs(x)<1e-6;}
template<> bool isZero<double>(double x) { return fabs(x)<1e-10;}
template<> bool isZero<std::complex<float> >(std::complex<float> x) { return std::norm(x)<1e-6;}
template<> bool isZero<std::complex<double> >(std::complex<double> x) { return std::norm(x)<1e-12;}


template<typename T> bool isNegative(const T &x) { return x < 0; }
template<> bool isNegative<std::complex<float> >(const std::complex<float> &x) { return false; }
template<> bool isNegative<std::complex<double> >(const std::complex<double> &x) { return false; }


template<typename T> class Polynomial {
public:
    Polynomial(T c=T(0.)):coefficients(1,c) {}
    Polynomial(const std::initializer_list<T> l): coefficients(l) {}
    Polynomial(const std::vector<T> l): coefficients(l) {}

    template<typename T1> bool isRoot(const T1 x) const { return isZero(operator()(x));}

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
        if (degree()==1) return Polynomial(coefficients[1]);
        auto rc = deflateWithResidue(r);
        assert (isZero(rc.second));
        return rc.first;
    }

    Polynomial deflate(T u, T v) const {
       auto rc = deflateWithResidue(u, v);
       return rc.first;
    }

    std::pair<Polynomial<T>, T> deflateWithResidue(T r) const {
        assert (degree()>0);
        /*Implement Ruffini's rule*/
        std::vector<T> nc(degree());
        auto cit = coefficients.rbegin();
        T prev = 0;
        for(auto it = nc.rbegin(); it != nc.rend(); ++it)
            prev = *it = *(cit++)+prev*r;
        auto s = *cit+prev*r;
        return std::pair<Polynomial<T>,T>(Polynomial(nc), s);
    }

    std::pair<Polynomial<T>, Polynomial<T> > deflateWithResidue(T u, T v) const {
        /* If there is nothing to deflate, return immediately */
        if (degree()<=1)
          return std::pair<Polynomial<T>, Polynomial<T> >(Polynomial(), Polynomial(coefficients));
        assert (degree()>1);
        std::vector<T> nc(degree()-1);
        auto cit = coefficients.rbegin();
        T prev(0), pprev(0);

        for(auto it = nc.rbegin(); it != nc.rend(); ++it) {
           *it = *(cit++)-u*prev-v*pprev;
           pprev = prev; prev = *it;
        }

        auto c = *(cit++)-u*prev-v*pprev;
        auto d = *cit-v*prev;
        return std::pair<Polynomial<T>, Polynomial<T> >(Polynomial(nc), Polynomial({d,c}));
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

/* Power function*/
template<typename T> Polynomial<T> operator^(const Polynomial<T> &a, unsigned b) {
    Polynomial<T> rc(T(1));
    for(auto i=0;i<b;++i)
        rc =rc*a;
    return rc;
}

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

template<typename T> class DoNotConvergeException {
public:
    DoNotConvergeException(T val):lastValue(val) {}
    const T & getValue() const { return lastValue; }
private:
    T lastValue;
};

template<typename T> T findRootLaguerre(const Polynomial<T> &p, T x0=0, unsigned maxSteps = 500) {
    unsigned step = 0;
    T x = x0;
    auto n = T(p.degree());
    auto nminus1 = n - T(1);
    auto derP = p.derivative();
    auto der2P = derP.derivative();

    //std::cout<<"findRootLaguerre(p="<<p<<" ,x0="<<x0<<"): derP="<<derP<<" der2P="<<der2P<<std::endl;
    while (!p.isRoot(x)) {
        auto px = p(x);
        auto G = derP(x)/px;
        auto H = G*G-der2P(x)/px;
        auto s = sqrt(nminus1*(n*H-G*G));
        auto a = abs(G+s)>abs(G-s)? n/(G+s) : n/(G-s);
        x-=a;
        if (step++ > maxSteps) throw DoNotConvergeException<T>(x);
    }
    //std::cout<<"Found root "<<x<<std::endl;

    return x;
}

template<typename T> T findRootNewton(const Polynomial<T> &p, T x0=0, unsigned maxSteps = 500) {
    unsigned step = 0;
    T x = x0;
    auto derP = p.derivative();

    while (!p.isRoot(x)) {
        auto a = p(x)/derP(x);
        x =- a;
        if (step++ > maxSteps) throw DoNotConvergeException<T>(x);
    }
    return x;
}


template<typename T> using conjugatePair = std::pair <std::complex<T>, std::complex<T> >;

template<typename T> conjugatePair<T> solveQuadratic(T u, T v) {
    auto D = u*u-4*v;
    if ( D >= 0)
        return conjugatePair<T> (std::complex<T>(.5*(-u-sqrt(D))), std::complex<T>(.5*(-u+sqrt(D))));

    return conjugatePair<T> (std::complex<T>(-.5*u,.5*sqrt(-D)), std::complex<T>(-.5*-u,.5*sqrt(-D)));
}

template<typename T> conjugatePair<T> findRootsBairstow(const Polynomial<T> &p, unsigned maxSteps = 500) {
   auto rc = findQuadraticFactorBairstow (p, maxSteps);
   return solveQuadratic (rc.first, rc.second);
}

template<typename T> std::pair<T,T> findQuadraticFactorBairstow(const Polynomial<T> &p, unsigned maxSteps = 500) {
    assert (p.degree() > 1);

    /* Initial quadratic polynomial coefficients */
    auto u = p[p.degree()-1];
    auto v = p[p.degree()-2];
    if (p[p.degree()]!=0) {
        u /= p[p.degree()];
        v /= p[p.degree()];
    }
    unsigned steps = 0;
    auto roots = solveQuadratic(u,v);
    while (!p.isRoot(roots.first)) {
        if (steps++ > maxSteps) throw DoNotConvergeException<decltype(roots.first)>(roots.first);
        /* Compute residue */
        auto Pq = p.deflateWithResidue(u,v);
        auto c = Pq.second[1];
        auto d = Pq.second[0];
        /* Compute du,dv-derivatives of the residue */
        auto Qq = Pq.first.deflateWithResidue(u,v);
        auto g = Qq.second[1];
        auto h = Qq.second[0];
        /* J is determinant of residue partial derivatives matrix */
        auto J = 1/(v*g*g+h*(h-u*g));
        auto nu = u-J*(g*d-h*c);
        auto nv = v-J*((g*u-h)*d-g*v*c);
        roots = solveQuadratic(u = nu, v = nv);
    }
    return std::pair<T, T> (u, v);
}

#endif
