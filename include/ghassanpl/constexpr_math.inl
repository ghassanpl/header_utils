/// The below code is based on Sun's libm library code, which is licensed under the following license:
/*====================================================
* Copyright(C) 2004 by Sun Microsystems, Inc.All rights reserved.
*
* Permission to use, copy, modify, and distribute this
* software is freely granted, provided that this notice
* is preserved.
* ====================================================
*/

namespace detail
{
	inline constexpr auto nan_mix(auto x, auto y) noexcept { return ((x + 0.0L) + (y + 0)); }

	template <typename A, typename B>
	constexpr void extract_words(A& high, B& low, double d) noexcept
	{
		low = std::bit_cast<uint64_t>(d) & 0xFFFFFFFF;
		high = (std::bit_cast<uint64_t>(d) >> 32LL) & 0xFFFFFFFF;
	}

	template <typename A>
	constexpr void get_high_word(A& high, double d) noexcept
	{
		high = (std::bit_cast<uint64_t>(d) >> 32LL) & 0xFFFFFFFF;
	}

	template <typename A>
	constexpr void get_low_word(A& low, double d) noexcept
	{
		low = std::bit_cast<uint64_t>(d) & 0xFFFFFFFF;
	}

	template <typename A, typename B>
	constexpr void insert_words(double& d, A high, B low) noexcept
	{
		const auto bits = static_cast<uint64_t>(std::bit_cast<uint32_t>(low)) | (static_cast<uint64_t>(std::bit_cast<uint32_t>(high)) << 32ULL);
		d = std::bit_cast<double>(bits);
	}

	template <typename A>
	constexpr void set_high_word(double& d, A i) noexcept
	{
		uint32_t low = 0, high = 0;
		::ghassanpl::cem::detail::extract_words(high, low, d);
		::ghassanpl::cem::detail::insert_words(d, i, low);
	}

	template <typename A>
	constexpr void set_low_word(double& d, A i) noexcept
	{
		uint32_t low = 0, high = 0;
		::ghassanpl::cem::detail::extract_words(high, low, d);
		::ghassanpl::cem::detail::insert_words(d, high, i);
	}

	static constexpr double bp[] = { 1.0, 1.5 };
	static constexpr double dp_h[] = { 0.0, 5.84962487220764160156e-01 }; /// 0x3FE2B803; 0x40000000 
	static constexpr double dp_l[] = { 0.0, 1.35003920212974897128e-08 }; /// 0x3E4CFDEB; 0x43CFD006 
	static constexpr double zero = 0.0;
	static constexpr double half = 0.5;
	static constexpr double qrtr = 0.25;
	static constexpr double thrd = 3.3333333333333331e-01; /// 0x3fd55555, 0x55555555
	static constexpr double one = 1.0;
	static constexpr double two = 2.0;
	static constexpr double two53 = 9007199254740992.0; /// 0x43400000; 0x00000000
	static constexpr double huge = 1.0e300;
	static constexpr double tiny = 1.0e-300;
	static constexpr double L1 = 5.99999999999994648725e-01;      /// 0x3FE33333; 0x33333303 
	static constexpr double L2 = 4.28571428578550184252e-01;      /// 0x3FDB6DB6; 0xDB6FABFF 
	static constexpr double L3 = 3.33333329818377432918e-01;      /// 0x3FD55555; 0x518F264D 
	static constexpr double L4 = 2.72728123808534006489e-01;      /// 0x3FD17460; 0xA91D4101 
	static constexpr double L5 = 2.30660745775561754067e-01;      /// 0x3FCD864A; 0x93C9DB65 
	static constexpr double L6 = 2.06975017800338417784e-01;      /// 0x3FCA7E28; 0x4A454EEF 
	static constexpr double P1 = 1.66666666666666019037e-01;      /// 0x3FC55555; 0x5555553E 
	static constexpr double P2 = -2.77777777770155933842e-03;     /// 0xBF66C16C; 0x16BEBD93 
	static constexpr double P3 = 6.61375632143793436117e-05;      /// 0x3F11566A; 0xAF25DE2C 
	static constexpr double P4 = -1.65339022054652515390e-06;     /// 0xBEBBBD41; 0xC5D26BF1 
	static constexpr double P5 = 4.13813679705723846039e-08;      /// 0x3E663769; 0x72BEA4D0 
	static constexpr double lg2 = 6.93147180559945286227e-01;     /// 0x3FE62E42; 0xFEFA39EF 
	static constexpr double lg2_h = 6.93147182464599609375e-01;   /// 0x3FE62E43; 0x00000000 
	static constexpr double lg2_l = -1.90465429995776804525e-09;  /// 0xBE205C61; 0x0CA86C39 
	static constexpr double ovt = 8.0085662595372944372e-0017;    /// -(1024-log2(ovfl+.5ulp)) 
	static constexpr double cp = 9.61796693925975554329e-01;      /// 0x3FEEC709; 0xDC3A03FD =2/(3ln2) 
	static constexpr double cp_h = 9.61796700954437255859e-01;    /// 0x3FEEC709; 0xE0000000 =(float)cp 
	static constexpr double cp_l = -7.02846165095275826516e-09;   /// 0xBE3E2FE0; 0x145B01F5 =tail of cp_h
	static constexpr double ivln2 = 1.44269504088896338700e+00;   /// 0x3FF71547; 0x652B82FE =1/ln2 
	static constexpr double ivln2_h = 1.44269502162933349609e+00; /// 0x3FF71547; 0x60000000 =24b 1/ln2
	static constexpr double ivln2_l = 1.92596299112661746887e-08; /// 0x3E54AE0B; 0xF85DDF44 =1/ln2 tail

	constexpr double sqrt_impl(double x) noexcept
	{
		double z;
		int32_t sign = (int)0x80000000;
		int32_t ix0, s0, q, m, t, i;
		uint32_t r, t1, s1, ix1, q1;

		extract_words(ix0, ix1, x);

		/* take care of Inf and NaN */
		if ((ix0 & 0x7ff00000) == 0x7ff00000)
		{
			return x * x + x; /* sqrt(NaN)=NaN, sqrt(+inf)=+inf
						 sqrt(-inf)=sNaN */
		}
		/* take care of zero */
		if (ix0 <= 0)
		{
			if (((ix0 & (~sign)) | ix1) == 0)
				return x; /* sqrt(+-0) = +-0 */
			else if (ix0 < 0)
				return std::numeric_limits<double>::quiet_NaN(); /* sqrt(-ve) = sNaN */
		}
		/* normalize x */
		m = (ix0 >> 20);
		if (m == 0)
		{ /* subnormal x */
			while (ix0 == 0)
			{
				m -= 21;
				ix0 |= (ix1 >> 11);
				ix1 <<= 21;
			}
			for (i = 0; (ix0 & 0x00100000) == 0; i++)
				ix0 <<= 1;
			m -= i - 1;
			ix0 |= (ix1 >> (32 - i));
			ix1 <<= i;
		}
		m -= 1023; /* unbias exponent */
		ix0 = (ix0 & 0x000fffff) | 0x00100000;
		if (m & 1)
		{ /* odd m, double x to make it even */
			ix0 += ix0 + ((ix1 & sign) >> 31);
			ix1 += ix1;
		}
		m >>= 1; /* m = [m/2] */

		/* generate sqrt(x) bit by bit */
		ix0 += ix0 + ((ix1 & sign) >> 31);
		ix1 += ix1;
		q = q1 = s0 = s1 = 0; /* [q,q1] = sqrt(x) */
		r = 0x00200000;       /* r = moving bit from right to left */

		while (r != 0)
		{
			t = s0 + r;
			if (t <= ix0)
			{
				s0 = t + r;
				ix0 -= t;
				q += r;
			}
			ix0 += ix0 + ((ix1 & sign) >> 31);
			ix1 += ix1;
			r >>= 1;
		}

		r = sign;
		while (r != 0)
		{
			t1 = s1 + r;
			t = s0;
			if ((t < ix0) || ((t == ix0) && (t1 <= ix1)))
			{
				s1 = t1 + r;
				if (((t1 & sign) == sign) && (s1 & sign) == 0)
					s0 += 1;
				ix0 -= t;
				if (ix1 < t1)
					ix0 -= 1;
				ix1 -= t1;
				q1 += r;
			}
			ix0 += ix0 + ((ix1 & sign) >> 31);
			ix1 += ix1;
			r >>= 1;
		}

		/* use floating add to find out rounding direction */
		if ((ix0 | ix1) != 0)
		{
			z = one - tiny; /* trigger inexact flag */
			if (z >= one)
			{
				z = one + tiny;
				if (q1 == (uint32_t)0xffffffff)
				{
					q1 = 0;
					q += 1;
				}
				else if (z > one)
				{
					if (q1 == (uint32_t)0xfffffffe)
						q += 1;
					q1 += 2;
				}
				else
					q1 += (q1 & 1);
			}
		}
		ix0 = (q >> 1) + 0x3fe00000;
		ix1 = q1 >> 1;
		if ((q & 1) == 1)
			ix1 |= sign;
		ix0 += (m << 20);
		insert_words(z, ix0, ix1);
		return z;
	}

	constexpr double scalbn(double x, int n) noexcept
	{
		double y = x;

		if (n > 1023)
		{
			y *= 0x1p1023;
			n -= 1023;
			if (n > 1023)
			{
				y *= 0x1p1023;
				n -= 1023;
				if (n > 1023)
					n = 1023;
			}
		}
		else if (n < -1022)
		{
			/* make sure final n < -53 to avoid double
			rounding in the subnormal range */
			y *= 0x1p-1022 * 0x1p53;
			n += 1022 - 53;
			if (n < -1022)
			{
				y *= 0x1p-1022 * 0x1p53;
				n += 1022 - 53;
				if (n < -1022)
					n = -1022;
			}
		}
		x = y * std::bit_cast<double>((uint64_t)(0x3ff + n) << 52);
		return x;
	}

	constexpr double pow_impl(double x, double y) noexcept
	{
		double z, ax, z_h, z_l, p_h, p_l;
		double y1, t1, t2, r, s, t, u, v, w;
		int32_t i, j, k, yisint, n;
		int32_t hx, hy, ix, iy;
		uint32_t lx, ly;

		extract_words(hx, lx, x);
		extract_words(hy, ly, y);
		ix = hx & 0x7fffffff;
		iy = hy & 0x7fffffff;

		/* y==zero: x**0 = 1 */
		if ((iy | ly) == 0)
			return one;

		/* x==1: 1**y = 1, even if y is NaN */
		if (hx == 0x3ff00000 && lx == 0)
			return one;

		/* y!=zero: result is NaN if either arg is NaN */
		if (ix > 0x7ff00000 || ((ix == 0x7ff00000) && (lx != 0)) ||
			iy > 0x7ff00000 || ((iy == 0x7ff00000) && (ly != 0)))
			return nan_mix(x, y);

		/* determine if y is an odd int when x < 0
		 * yisint = 0	... y is not an integer
		 * yisint = 1	... y is an odd int
		 * yisint = 2	... y is an even int
		 */
		yisint = 0;
		if (hx < 0)
		{
			if (iy >= 0x43400000)
				yisint = 2; /* even integer y */
			else if (iy >= 0x3ff00000)
			{
				k = (iy >> 20) - 0x3ff; /* exponent */
				if (k > 20)
				{
					j = ly >> (52 - k);
					if (((uint32_t)j << (52 - k)) == ly)
						yisint = 2 - (j & 1);
				}
				else if (ly == 0)
				{
					j = iy >> (20 - k);
					if ((j << (20 - k)) == iy)
						yisint = 2 - (j & 1);
				}
			}
		}

		/* special value of y */
		if (ly == 0)
		{
			if (iy == 0x7ff00000)
			{ /* y is +-inf */
				if (((ix - 0x3ff00000) | lx) == 0)
					return one;            /* (-1)**+-inf is 1 */
				else if (ix >= 0x3ff00000) /* (|x|>1)**+-inf = inf,0 */
					return (hy >= 0) ? y : zero;
				else /* (|x|<1)**-,+inf = inf,0 */
					return (hy < 0) ? -y : zero;
			}
			if (iy == 0x3ff00000)
			{ /* y is  +-1 */
				if (hy < 0)
					return one / x;
				else
					return x;
			}
			if (hy == 0x40000000)
				return x * x; /* y is  2 */
			if (hy == 0x3fe00000)
			{                /* y is  0.5 */
				if (hx >= 0) /* x >= +0 */
					return sqrt_impl(x);
			}
		}

		ax = cem::abs(x);
		/* special value of x */
		if (lx == 0)
		{
			if (ix == 0x7ff00000 || ix == 0 || ix == 0x3ff00000)
			{
				z = ax; /*x is +-0,+-inf,+-1*/
				if (hy < 0)
					z = one / z; /* z = (1/|x|) */
				if (hx < 0)
				{
					if (((ix - 0x3ff00000) | yisint) == 0)
					{
						z = (z - z) / (z - z); /* (-1)**non-int is NaN */
					}
					else if (yisint == 1)
						z = -z; /* (x<0)**odd = -(|x|**odd) */
				}
				return z;
			}
		}

		/* CYGNUS LOCAL + fdlibm-5.3 fix: This used to be
		n = (hx>>31)+1;
		   but ANSI C says a right shift of a signed negative quantity is
		   implementation defined.  */
		n = ((uint32_t)hx >> 31) - 1;

		/* (x<0)**(non-int) is NaN */
		if ((n | yisint) == 0)
			return std::numeric_limits<double>::quiet_NaN();

		s = one; /* s (sign of result -ve**odd) = -1 else = 1 */
		if ((n | (yisint - 1)) == 0)
			s = -one; /* (-ve)**(odd int) */

		/* |y| is huge */
		if (iy > 0x41e00000)
		{ /* if |y| > 2**31 */
			if (iy > 0x43f00000)
			{ /* if |y| > 2**64, must o/uflow */
				if (ix <= 0x3fefffff)
					return (hy < 0) ? huge * huge : tiny * tiny;
				if (ix >= 0x3ff00000)
					return (hy > 0) ? huge * huge : tiny * tiny;
			}
			/* over/underflow if x is not close to one */
			if (ix < 0x3fefffff)
				return (hy < 0) ? s * huge * huge : s * tiny * tiny;
			if (ix > 0x3ff00000)
				return (hy > 0) ? s * huge * huge : s * tiny * tiny;
			/* now |1-x| is tiny <= 2**-20, suffice to compute
			   log(x) by x-x^2/2+x^3/3-x^4/4 */
			t = ax - one; /* t has 20 trailing zeros */
			w = (t * t) * (half - t * (thrd - t * qrtr));
			u = ivln2_h * t; /* ivln2_h has 21 sig. bits */
			v = t * ivln2_l - w * ivln2;
			t1 = u + v;
			set_low_word(t1, 0);
			t2 = v - (t1 - u);
		}
		else
		{
			double ss, s2, s_h, s_l, t_h, t_l;
			n = 0;
			/* take care subnormal number */
			if (ix < 0x00100000)
			{
				ax *= two53;
				n -= 53;
				get_high_word(ix, ax);
			}
			n += ((ix) >> 20) - 0x3ff;
			j = ix & 0x000fffff;
			/* determine interval */
			ix = j | 0x3ff00000; /* normalize ix */
			if (j <= 0x3988E)
				k = 0; /* |x|<sqrt(3/2) */
			else if (j < 0xBB67A)
				k = 1; /* |x|<sqrt(3)   */
			else
			{
				k = 0;
				n += 1;
				ix -= 0x00100000;
			}
			set_high_word(ax, ix);

			/* compute ss = s_h+s_l = (x-1)/(x+1) or (x-1.5)/(x+1.5) */
			u = ax - bp[k]; /* bp[0]=1.0, bp[1]=1.5 */
			v = one / (ax + bp[k]);
			ss = u * v;
			s_h = ss;
			set_low_word(s_h, 0);
			/* t_h=ax+bp[k] High */
			t_h = zero;
			set_high_word(t_h, ((ix >> 1) | 0x20000000) + 0x00080000 + (k << 18));
			t_l = ax - (t_h - bp[k]);
			s_l = v * ((u - s_h * t_h) - s_h * t_l);
			/* compute log(ax) */
			s2 = ss * ss;
			r = s2 * s2 * (L1 + s2 * (L2 + s2 * (L3 + s2 * (L4 + s2 * (L5 + s2 * L6)))));
			r += s_l * (s_h + ss);
			s2 = s_h * s_h;
			t_h = 3 + s2 + r;
			set_low_word(t_h, 0);
			t_l = r - ((t_h - 3) - s2);
			/* u+v = ss*(1+...) */
			u = s_h * t_h;
			v = s_l * t_h + t_l * ss;
			/* 2/(3log2)*(ss+...) */
			p_h = u + v;
			set_low_word(p_h, 0);
			p_l = v - (p_h - u);
			z_h = cp_h * p_h; /* cp_h+cp_l = 2/(3*log2) */
			z_l = cp_l * p_h + p_l * cp + dp_l[k];
			/* log2(ax) = (ss+..)*2/(3*log2) = n + dp_h + z_h + z_l */
			t = n;
			t1 = (((z_h + z_l) + dp_h[k]) + t);
			set_low_word(t1, 0);
			t2 = z_l - (((t1 - t) - dp_h[k]) - z_h);
		}

		/* split up y into y1+y2 and compute (y1+y2)*(t1+t2) */
		y1 = y;
		set_low_word(y1, 0);
		p_l = (y - y1) * t1 + y * t2;
		p_h = y1 * t1;
		z = p_l + p_h;
		extract_words(j, i, z);
		if (j >= 0x40900000)
		{                                    /* z >= 1024 */
			if (((j - 0x40900000) | i) != 0) /* if z > 1024 */
				return s * huge * huge;      /* overflow */
			else
			{
				if (p_l + ovt > z - p_h)
					return s * huge * huge; /* overflow */
			}
		}
		else if ((j & 0x7fffffff) >= 0x4090cc00)
		{                                    /* z <= -1075 */
			if (((j - 0xc090cc00) | i) != 0) /* z < -1075 */
				return s * tiny * tiny;      /* underflow */
			else
			{
				if (p_l <= z - p_h)
					return s * tiny * tiny; /* underflow */
			}
		}
		/*
		 * compute 2**(p_h+p_l)
		 */
		i = j & 0x7fffffff;
		k = (i >> 20) - 0x3ff;
		n = 0;
		if (i > 0x3fe00000)
		{ /* if |z| > 0.5, set n = [z+0.5] */
			n = j + (0x00100000 >> (k + 1));
			k = ((n & 0x7fffffff) >> 20) - 0x3ff; /* new k for n */
			t = zero;
			set_high_word(t, n & ~(0x000fffff >> k));
			n = ((n & 0x000fffff) | 0x00100000) >> (20 - k);
			if (j < 0)
				n = -n;
			p_h -= t;
		}
		t = p_l + p_h;
		set_low_word(t, 0);
		u = t * lg2_h;
		v = (p_l - (t - p_h)) * lg2 + t * lg2_l;
		z = u + v;
		w = v - (z - u);
		t = z * z;
		t1 = z - t * (P1 + t * (P2 + t * (P3 + t * (P4 + t * P5))));
		r = (z * t1) / (t1 - two) - (w + z * w);
		z = one - (r - z);
		get_high_word(j, z);
		j += (n << 20);

		if ((j >> 20) <= 0)
			z = scalbn(z, n); /* subnormal output */
		else
			set_high_word(z, j);
		return s * z;
	}
}
