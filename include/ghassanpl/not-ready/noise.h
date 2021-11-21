# The base bit-noise constants were crafted to have distinctive and interesting
# bits, and have so far produced excellent experimental test results.
NOISE1 = 0xb5297a4d  # 0b0110'1000'1110'0011'0001'1101'1010'0100
NOISE2 = 0x68e31da4  # 0b1011'0101'0010'1001'0111'1010'0100'1101
NOISE3 = 0x1b56c4e9  # 0b0001'1011'0101'0110'1100'0100'1110'1001

CAP = 1 << 32


def squirrel3(n, seed=0):
    """Returns an unsigned integer containing 32 reasonably-well-scrambled
    bits, based on a given (signed) integer input parameter `n` and optional
    `seed`.  Kind of like looking up a value in an infinitely large
    non-existent table of previously generated random numbers.
    """
    n *= NOISE1
    n += seed
    n ^= n >> 8
    n += NOISE2
    n ^= n << 8
    n *= NOISE3
    n ^= n >> 8
    # Cast into uint32 like the original `Squirrel3`.
    return n % CAP
    

void SetParameters(float hurst, float lacunarity, float octaves)
{
    m_lacunarity = lacunarity;
    m_hurst = hurst;
    m_octaves = octaves;

    _recompute();
}

void _recompute()
{
    float frequency = 1.0;
    m_sum = 0.f;
    m_exponent_array.clear();

    for (int i(0) ; i < static_cast<int>(m_octaves) ; ++i)
    {
        m_exponent_array.push_back(std::pow( frequency, -m_hurst ));
        frequency *= m_lacunarity;
        m_sum += m_exponent_array.at(i);
    }
}

float Get(std::initializer_list<float> coordinates, float scale) const
{
    float value = 0.0;

    for(int i(0); i < m_octaves; ++i)
    {
        value += m_source.Get(coordinates,scale) * m_exponent_array.at(i);
        scale *= m_lacunarity;
    }

    float remainder = m_octaves - static_cast<int>(m_octaves);

    if(std::fabs(remainder) > 0.01f)
      value += remainder * m_source.Get(coordinates,scale) * m_exponent_array.at(static_cast<int>(m_octaves-1));

    return value / m_sum;
}