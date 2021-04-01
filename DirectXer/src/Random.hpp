/* Copyright (c) 2018 Arvid Gerstmann. */
/* This code is licensed under MIT license. */

// @Note: This is taken from here https://gist.github.com/Leandros/6dc334c22db135b033b57e9ee0311553
#pragma once

#include <random>

static inline std::random_device g_RandomDevice;

class splitmix
{
  public:
    using result_type = uint32_t;
    static constexpr result_type (min)() { return 0; }
    static constexpr result_type (max)() { return UINT32_MAX; }
    friend bool operator==(splitmix const &, splitmix const &);
    friend bool operator!=(splitmix const &, splitmix const &);

    splitmix();
    explicit splitmix(std::random_device &rd);

    void seed(std::random_device &rd);

    result_type operator()();

    void discard(unsigned long long n);

  private:
    uint64_t m_seed;
};

bool operator==(splitmix const &lhs, splitmix const &rhs);
bool operator!=(splitmix const &lhs, splitmix const &rhs);

class xorshift
{
  public:
    using result_type = uint32_t;
    static constexpr result_type (min)() { return 0; }
    static constexpr result_type (max)() { return UINT32_MAX; }
    friend bool operator==(xorshift const &, xorshift const &);
    friend bool operator!=(xorshift const &, xorshift const &);

    xorshift();
    explicit xorshift(std::random_device &rd);

    void seed(std::random_device &rd);

    result_type operator()();

    void discard(unsigned long long n);

  private:
    uint64_t m_seed;
};

bool operator==(xorshift const &lhs, xorshift const &rhs);
bool operator!=(xorshift const &lhs, xorshift const &rhs);

class pcg
{
  public:
    using result_type = uint32_t;
    static constexpr result_type (min)() { return 0; }
    static constexpr result_type (max)() { return UINT32_MAX; }
    friend bool operator==(pcg const &, pcg const &);
    friend bool operator!=(pcg const &, pcg const &);

    pcg();
    explicit pcg(std::random_device &rd);

	void seed(std::random_device &rd);

    result_type operator()();

    void discard(unsigned long long n);
	
  private:
    uint64_t m_state;
    uint64_t m_inc;
};

bool operator==(pcg const &lhs, pcg const &rhs);
bool operator!=(pcg const &lhs, pcg const &rhs);


struct Random
{
	static inline pcg RandomGenerator{};

	static void Init()
	{

	};

	static float Uniform(float start = 0.0f, float end = 1.0f)
	{
		const std::uniform_real_distribution<float> uniformDistribution(start, end);
		return uniformDistribution(RandomGenerator);
	}

};
