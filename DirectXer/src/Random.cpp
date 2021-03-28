/* Copyright (c) 2018 Arvid Gerstmann. */
/* This code is licensed under MIT license. */

// @Note: This is taken from here https://gist.github.com/Leandros/6dc334c22db135b033b57e9ee0311553
#include <Random.hpp>

splitmix::splitmix() : m_seed(1) {}
splitmix::splitmix(std::random_device &rd)
{
	seed(rd);
}

void splitmix::seed(std::random_device &rd)
{
	m_seed = uint64_t(rd()) << 31 | uint64_t(rd());
}

splitmix::result_type splitmix::operator()()
{
	uint64_t z = (m_seed += UINT64_C(0x9E3779B97F4A7C15));
	z = (z ^ (z >> 30)) * UINT64_C(0xBF58476D1CE4E5B9);
	z = (z ^ (z >> 27)) * UINT64_C(0x94D049BB133111EB);
	return result_type((z ^ (z >> 31)) >> 31);
}

void splitmix::discard(unsigned long long n)
{
	for (unsigned long long i = 0; i < n; ++i)
		operator()();
}

bool operator==(splitmix const &lhs, splitmix const &rhs)
{
    return lhs.m_seed == rhs.m_seed;
}
bool operator!=(splitmix const &lhs, splitmix const &rhs)
{
    return lhs.m_seed != rhs.m_seed;
}


xorshift::xorshift() : m_seed(0xc1f651c67c62c6e0ull) {}
xorshift::xorshift(std::random_device &rd)
{
	seed(rd);
}

void xorshift::seed(std::random_device &rd)
{
	m_seed = uint64_t(rd()) << 31 | uint64_t(rd());
}

xorshift::result_type xorshift::operator()()
{
	uint64_t result = m_seed * 0xd989bcacc137dcd5ull;
	m_seed ^= m_seed >> 11;
	m_seed ^= m_seed << 31;
	m_seed ^= m_seed >> 18;
	return uint32_t(result >> 32ull);
}

void xorshift::discard(unsigned long long n)
{
	for (unsigned long long i = 0; i < n; ++i)
		operator()();
}

bool operator==(xorshift const &lhs, xorshift const &rhs)
{
    return lhs.m_seed == rhs.m_seed;
}
bool operator!=(xorshift const &lhs, xorshift const &rhs)
{
    return lhs.m_seed != rhs.m_seed;
}


pcg::pcg()
	: m_state(0x853c49e6748fea9bULL)
	, m_inc(0xda3e39cb94b95bdbULL)
{}
pcg::pcg(std::random_device &rd)
{
	seed(rd);
}

void pcg::seed(std::random_device &rd)
{
	uint64_t s0 = uint64_t(rd()) << 31 | uint64_t(rd());
	uint64_t s1 = uint64_t(rd()) << 31 | uint64_t(rd());

	m_state = 0;
	m_inc = (s1 << 1) | 1;
	(void)operator()();
	m_state += s0;
	(void)operator()();
}

pcg::result_type pcg::operator()()
{
	uint64_t oldstate = m_state;
	m_state = oldstate * 6364136223846793005ULL + m_inc;
	uint32_t xorshifted = uint32_t(((oldstate >> 18u) ^ oldstate) >> 27u);
	int rot = oldstate >> 59u;
	return (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
}

void pcg::discard(unsigned long long n)
{
	for (unsigned long long i = 0; i < n; ++i)
		operator()();
}


bool operator==(pcg const &lhs, pcg const &rhs)
{
    return lhs.m_state == rhs.m_state
        && lhs.m_inc == rhs.m_inc;
}
bool operator!=(pcg const &lhs, pcg const &rhs)
{
    return lhs.m_state != rhs.m_state
        || lhs.m_inc != rhs.m_inc;
}
