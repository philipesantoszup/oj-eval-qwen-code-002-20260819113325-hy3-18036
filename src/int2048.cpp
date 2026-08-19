#include "int2048.h"

#include <cstdio>

namespace {

using V = std::vector<unsigned int>;

const unsigned int BASE = 1000000000u; // 10^9
const int BW = 9;

void trim(V &a) {
    while (a.size() > 1 && a.back() == 0u)
        a.pop_back();
}

int ucmp(const V &a, const V &b) {
    if (a.size() != b.size())
        return a.size() < b.size() ? -1 : 1;
    for (int i = (int)a.size() - 1; i >= 0; --i) {
        if (a[i] != b[i])
            return a[i] < b[i] ? -1 : 1;
    }
    return 0;
}

// a + b (magnitudes)
V uadd(const V &a, const V &b) {
    V r;
    size_t n = a.size() > b.size() ? a.size() : b.size();
    r.reserve(n + 1);
    unsigned long long carry = 0;
    for (size_t i = 0; i < n; ++i) {
        unsigned long long s = carry;
        if (i < a.size())
            s += a[i];
        if (i < b.size())
            s += b[i];
        r.push_back((unsigned int)(s % BASE));
        carry = s / BASE;
    }
    if (carry)
        r.push_back((unsigned int)carry);
    return r;
}

// a - b assuming a >= b (magnitudes)
V usub(const V &a, const V &b) {
    V r;
    size_t n = a.size() > b.size() ? a.size() : b.size();
    r.reserve(n);
    long long borrow = 0;
    for (size_t i = 0; i < n; ++i) {
        long long s = (long long)(i < a.size() ? a[i] : 0) - borrow;
        if (i < b.size())
            s -= b[i];
        if (s < 0) {
            s += BASE;
            borrow = 1;
        } else {
            borrow = 0;
        }
        r.push_back((unsigned int)s);
    }
    trim(r);
    return r;
}

// schoolbook multiplication of magnitudes
V umul_school(const V &a, const V &b) {
    V r(a.size() + b.size(), 0u);
    for (size_t i = 0; i < a.size(); ++i) {
        unsigned long long carry = 0;
        unsigned long long ai = a[i];
        for (size_t j = 0; j < b.size(); ++j) {
            unsigned long long cur =
                (unsigned long long)r[i + j] + ai * b[j] + carry;
            r[i + j] = (unsigned int)(cur % BASE);
            carry = cur / BASE;
        }
        size_t k = i + b.size();
        while (carry) {
            unsigned long long cur = (unsigned long long)r[k] + carry;
            r[k] = (unsigned int)(cur % BASE);
            carry = cur / BASE;
            ++k;
        }
    }
    trim(r);
    return r;
}

// r += src * BASE^shift
void add_shifted(V &r, const V &src, size_t shift) {
    if (src.empty() || (src.size() == 1 && src[0] == 0u))
        return;
    if (r.size() < src.size() + shift)
        r.resize(src.size() + shift, 0u);
    unsigned long long carry = 0;
    for (size_t i = 0; i < src.size(); ++i) {
        unsigned long long cur =
            (unsigned long long)r[i + shift] + src[i] + carry;
        r[i + shift] = (unsigned int)(cur % BASE);
        carry = cur / BASE;
    }
    size_t i = src.size() + shift;
    while (carry) {
        unsigned long long cur = (unsigned long long)r[i] + carry;
        r[i] = (unsigned int)(cur % BASE);
        carry = cur / BASE;
        ++i;
    }
}

// Karatsuba multiplication of magnitudes
V umul(const V &a, const V &b) {
    if (a.empty() || b.empty())
        return V(1, 0u);
    if (a.size() * b.size() <= 4096u)
        return umul_school(a, b);

    size_t n = a.size() > b.size() ? a.size() : b.size();
    size_t m = n / 2;

    size_t am = a.size() < m ? a.size() : m;
    size_t bm = b.size() < m ? b.size() : m;
    V alo(a.begin(), a.begin() + am);
    V ahi(a.begin() + am, a.end());
    V blo(b.begin(), b.begin() + bm);
    V bhi(b.begin() + bm, b.end());
    trim(alo);
    trim(ahi);
    trim(blo);
    trim(bhi);

    V z0 = umul(alo, blo);
    V z2 = umul(ahi, bhi);
    V sa = uadd(alo, ahi);
    V sb = uadd(blo, bhi);
    V z1 = umul(sa, sb);
    V z0z2 = uadd(z0, z2);
    // z1 >= z0z2 always (it equals alo*bhi + ahi*blo)
    z1 = usub(z1, z0z2);

    V r = z0;
    add_shifted(r, z1, m);
    add_shifted(r, z2, 2 * m);
    trim(r);
    return r;
}

// unsigned long division / modulo: a = q*b + r, 0 <= r < b, b != 0
void udivmod(const V &a, const V &b, V &qout, V &rout) {
    if (ucmp(a, b) < 0) {
        qout = V(1, 0u);
        rout = a;
        trim(rout);
        return;
    }
    size_t n = b.size();
    size_t m = a.size() - n;

    if (n == 1) {
        V q(a.size(), 0u);
        unsigned long long rem = 0;
        for (int i = (int)a.size() - 1; i >= 0; --i) {
            unsigned long long cur = rem * BASE + a[i];
            q[i] = (unsigned int)(cur / b[0]);
            rem = cur % b[0];
        }
        trim(q);
        qout = q;
        rout = V(1, (unsigned int)rem);
        return;
    }

    // normalize so that the scaled top digit >= BASE/2 and never overflows
    unsigned int d = BASE / (b[n - 1] + 1);
    V u(a.size() + 1, 0u);
    unsigned long long carry = 0;
    for (size_t i = 0; i < a.size(); ++i) {
        unsigned long long cur = (unsigned long long)a[i] * d + carry;
        u[i] = (unsigned int)(cur % BASE);
        carry = cur / BASE;
    }
    u[a.size()] = (unsigned int)carry;

    V v(n, 0u);
    carry = 0;
    for (size_t i = 0; i < n; ++i) {
        unsigned long long cur = (unsigned long long)b[i] * d + carry;
        v[i] = (unsigned int)(cur % BASE);
        carry = cur / BASE;
    }

    V q(m + 1, 0u);
    for (int j = (int)m; j >= 0; --j) {
        unsigned long long num =
            (unsigned long long)u[j + n] * BASE + u[j + n - 1];
        unsigned int qhat = (unsigned int)(num / v[n - 1]);
        unsigned int rhat = (unsigned int)(num % v[n - 1]);
        while (qhat >= BASE ||
               (unsigned long long)qhat * v[n - 2] >
                   (unsigned long long)rhat * BASE + u[j + n - 2]) {
            qhat--;
            rhat += v[n - 1];
            if (rhat >= BASE)
                break;
        }

        long long borrow = 0;
        unsigned long long car = 0;
        for (size_t i = 0; i < n; ++i) {
            unsigned long long p = (unsigned long long)qhat * v[i] + car;
            car = p / BASE;
            unsigned int pmod = (unsigned int)(p % BASE);
            long long sub = (long long)u[j + i] - (long long)pmod - borrow;
            if (sub < 0) {
                sub += BASE;
                borrow = 1;
            } else {
                borrow = 0;
            }
            u[j + i] = (unsigned int)sub;
        }
        long long t = (long long)u[j + n] - (long long)car - borrow;
        if (t < 0) {
            qhat--;
            unsigned long long kk = 0;
            for (size_t i = 0; i < n; ++i) {
                unsigned long long sum =
                    (unsigned long long)u[j + i] + v[i] + kk;
                u[j + i] = (unsigned int)(sum % BASE);
                kk = sum / BASE;
            }
            u[j + n] = (unsigned int)(u[j + n] + kk);
        } else {
            u[j + n] = (unsigned int)t;
        }
        q[j] = qhat;
    }

    // unnormalize remainder
    V r(n, 0u);
    unsigned long long rem = 0;
    for (int i = (int)n - 1; i >= 0; --i) {
        unsigned long long cur = rem * BASE + u[i];
        r[i] = (unsigned int)(cur / d);
        rem = cur % d;
    }
    trim(r);
    trim(q);
    qout = q;
    rout = r;
}

void append_to_string(const V &digs, bool neg, std::string &s) {
    if (digs.size() == 1 && digs[0] == 0u) {
        s.push_back('0');
        return;
    }
    if (neg)
        s.push_back('-');
    int n = (int)digs.size();
    char buf[16];
    std::snprintf(buf, sizeof(buf), "%u", digs[n - 1]);
    s += buf;
    for (int i = n - 2; i >= 0; --i) {
        std::snprintf(buf, sizeof(buf), "%09u", digs[i]);
        s += buf;
    }
}

} // namespace

namespace sjtu {

int2048::int2048() {
    digs.assign(1, 0u);
    neg = false;
}

int2048::int2048(long long v) {
    if (v == 0) {
        digs.assign(1, 0u);
        neg = false;
        return;
    }
    unsigned long long m;
    if (v < 0) {
        neg = true;
        m = 0ULL - (unsigned long long)v;
    } else {
        neg = false;
        m = (unsigned long long)v;
    }
    digs.clear();
    while (m > 0) {
        digs.push_back((unsigned int)(m % BASE));
        m /= BASE;
    }
}

int2048::int2048(const std::string &s) { set_from_string(s); }

int2048::int2048(const int2048 &o) : digs(o.digs), neg(o.neg) {}

void int2048::normalize() {
    trim(digs);
    if (digs.empty())
        digs.push_back(0u);
    if (digs.size() == 1 && digs[0] == 0u)
        neg = false;
}

void int2048::set_from_string(const std::string &s) {
    size_t i = 0;
    bool sign = false;
    if (i < s.size() && (s[i] == '+' || s[i] == '-')) {
        sign = (s[i] == '-');
        ++i;
    }
    while (i < s.size() && s[i] == '0')
        ++i;
    if (i >= s.size()) {
        digs.assign(1, 0u);
        neg = false;
        return;
    }
    V d;
    int total = (int)(s.size());
    int start = (int)i;
    for (int e = total; e > start; e -= BW) {
        int from = (e - BW < start) ? start : e - BW;
        unsigned int part = 0u;
        for (int k = from; k < e; ++k)
            part = part * 10u + (unsigned int)(s[k] - '0');
        d.push_back(part);
    }
    digs.swap(d);
    neg = sign;
}

void int2048::read(const std::string &s) { set_from_string(s); }

void int2048::print() {
    std::string s;
    append_to_string(digs, neg, s);
    std::cout << s;
}

int2048 &int2048::add(const int2048 &o) {
    if (neg == o.neg) {
        digs = uadd(digs, o.digs);
    } else {
        int c = ucmp(digs, o.digs);
        if (c >= 0) {
            digs = usub(digs, o.digs);
        } else {
            digs = usub(o.digs, digs);
            neg = o.neg;
        }
    }
    normalize();
    return *this;
}

int2048 &int2048::minus(const int2048 &o) {
    int2048 t = o;
    if (!(t.digs.size() == 1 && t.digs[0] == 0u))
        t.neg = !t.neg;
    return add(t);
}

int2048 int2048::operator+() const { return *this; }

int2048 int2048::operator-() const {
    int2048 r = *this;
    if (!(r.digs.size() == 1 && r.digs[0] == 0u))
        r.neg = !r.neg;
    return r;
}

int2048 &int2048::operator=(const int2048 &o) {
    if (this != &o) {
        digs = o.digs;
        neg = o.neg;
    }
    return *this;
}

int2048 &int2048::operator+=(const int2048 &o) { return add(o); }

int2048 &int2048::operator-=(const int2048 &o) { return minus(o); }

int2048 &int2048::operator*=(const int2048 &o) {
    digs = umul(digs, o.digs);
    neg = neg ^ o.neg;
    normalize();
    return *this;
}

int2048 &int2048::operator/=(const int2048 &o) {
    bool an = neg, bn = o.neg;
    bool sameSign = (an == bn);
    V qm, rm;
    udivmod(digs, o.digs, qm, rm);
    if (sameSign) {
        digs = qm;
        neg = false;
    } else {
        if (rm.size() == 1 && rm[0] == 0u) {
            digs = qm;
            neg = true;
        } else {
            digs = uadd(qm, V(1, 1u));
            neg = true;
        }
    }
    normalize();
    return *this;
}

int2048 &int2048::operator%=(const int2048 &o) {
    bool an = neg, bn = o.neg;
    bool sameSign = (an == bn);
    V qm, rm;
    udivmod(digs, o.digs, qm, rm);
    if (sameSign) {
        digs = rm;
        neg = an;
    } else {
        if (rm.size() == 1 && rm[0] == 0u) {
            digs.assign(1, 0u);
            neg = false;
        } else {
            digs = usub(o.digs, rm);
            neg = bn;
        }
    }
    normalize();
    return *this;
}

int2048 add(int2048 a, const int2048 &b) { return a.add(b); }

int2048 minus(int2048 a, const int2048 &b) { return a.minus(b); }

int2048 operator+(int2048 a, const int2048 &b) { return a += b; }

int2048 operator-(int2048 a, const int2048 &b) { return a -= b; }

int2048 operator*(int2048 a, const int2048 &b) { return a *= b; }

int2048 operator/(int2048 a, const int2048 &b) { return a /= b; }

int2048 operator%(int2048 a, const int2048 &b) { return a %= b; }

std::istream &operator>>(std::istream &is, int2048 &x) {
    std::string s;
    is >> s;
    x.set_from_string(s);
    return is;
}

std::ostream &operator<<(std::ostream &os, const int2048 &x) {
    std::string s;
    append_to_string(x.digs, x.neg, s);
    os << s;
    return os;
}

bool operator==(const int2048 &a, const int2048 &b) {
    if (a.neg != b.neg)
        return false;
    return ucmp(a.digs, b.digs) == 0;
}

bool operator!=(const int2048 &a, const int2048 &b) { return !(a == b); }

bool operator<(const int2048 &a, const int2048 &b) {
    if (a.neg && !b.neg)
        return true;
    if (!a.neg && b.neg)
        return false;
    int c = ucmp(a.digs, b.digs);
    if (a.neg)
        return c > 0;
    return c < 0;
}

bool operator>(const int2048 &a, const int2048 &b) { return b < a; }

bool operator<=(const int2048 &a, const int2048 &b) { return !(b < a); }

bool operator>=(const int2048 &a, const int2048 &b) { return !(a < b); }

} // namespace sjtu
