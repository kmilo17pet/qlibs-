#include <include/fp16.hpp>


namespace qlibs {
    const fp16 FP_E        = 2.718281828459045235360_fp;
    const fp16 FP_LOG2E    = 1.442695040888963407360_fp;
    const fp16 FP_LOG10E   = 0.434294481903251827651_fp;
    const fp16 FP_LN2      = 0.693147180559945309417_fp;
    const fp16 FP_LN10     = 2.302585092994045684020_fp;
    const fp16 FP_PI       = 3.141592653589793238460_fp;
    const fp16 FP_PI_2     = 1.570796326794896619230_fp;
    const fp16 FP_PI_4     = 0.785398163397448309616_fp;
    const fp16 FP_1_PI     = 0.318309886183790671538_fp;
    const fp16 FP_2_PI     = 0.636619772367581343076_fp;
    const fp16 FP_2_SQRTPI = 1.128379167095512573900_fp;
    const fp16 FP_SQRT2    = 1.414213562373095048800_fp;
    const fp16 FP_SQRT1_2  = 0.707106781186547524401_fp;
}

using namespace qlibs;


fp16Raw_t fp16::Min = -2147483647; // skipcq: CXX-W2009
fp16Raw_t fp16::Max = 2147483647; // skipcq: CXX-W2009
bool fp16::flag_rounding = true; // skipcq: CXX-W2009
bool fp16::saturation = false; // skipcq: CXX-W2009
const fp16Raw_t fp16::exp_max = 681391;
const fp16Raw_t fp16::f2 = 131072;
const uint32_t fp16::overflow_mask = 0x80000000U;
const uint32_t fp16::fraction_mask = 0x0000FFFFU;
const uint32_t fp16::integer_mask = 0xFFFF0000U;
const fp16Raw_t fp16::f_pi_2 = 102944;
const fp16Raw_t fp16::overflow = -2147483647 - 1;
const fp16Raw_t fp16::one = 65536;
const fp16Raw_t fp16::one_half = 32768;

/*cstat -MISRAC++2008-5-0-21 -MISRAC++2008-5-0-9 -ATH-shift-neg -CERT-INT34-C_c -MISRAC++2008-5-0-10*/
/*============================================================================*/
int fp16::toInt( const fp16 &x ) noexcept
{
    int retValue;

    if ( flag_rounding ) {
        if ( x.value >= 0 ) {
            retValue = ( x.value + ( one >> 1 ) ) / one;
        }
        else {
            retValue = ( x.value - ( one >> 1 ) ) / one;
        }
    }
    else {
        retValue = static_cast<int>( static_cast<uint32_t>( x.value ) >> 16 );
    }

    return retValue;
}
/*============================================================================*/
float fp16::toFloat( const fp16 &x ) noexcept
{
    const float one_fp16_f = 0.0000152587890625F;
    /*cstat -CERT-FLP36-C*/
    return static_cast<float>( x.value )*one_fp16_f;
    /*cstat +CERT-FLP36-C*/
}
/*============================================================================*/
double fp16::toDouble( const fp16 &x ) noexcept
{
    const double one_fp16_d = 0.0000152587890625;
    return static_cast<double>( x.value )*one_fp16_d;
}
/*============================================================================*/
fp16Raw_t fp16::fromInt( const int x ) noexcept
{
    return static_cast<fp16Raw_t>( static_cast<uint32_t>( x ) << 16U );
}
/*============================================================================*/
fp16Raw_t fp16::fromFloat( const float x ) noexcept
{
    float f_value;
    /*cstat -CERT-FLP36-C -CERT-FLP34-C*/
    f_value = x * static_cast<float>( one );
    if ( flag_rounding ) {
        f_value += ( f_value >= 0.0F ) ? 0.5F : -0.5F;
    }
    return static_cast<fp16Raw_t>( f_value );
    /*cstat +CERT-FLP36-C +CERT-FLP34-C*/
}
/*============================================================================*/
fp16Raw_t fp16::fromDouble( const double x ) noexcept
{
    double d_value;
    /*cstat -CERT-FLP36-C -CERT-FLP34-C*/
    d_value = x * static_cast<double>( one );
    if ( flag_rounding ) {
        d_value += ( d_value >= 0.0 ) ? 0.5 : -0.5;
    }
    return static_cast<fp16Raw_t>( d_value );
    /*cstat +CERT-FLP36-C +CERT-FLP34-C*/
}
/*============================================================================*/
fp16Raw_t fp16::saturate( const fp16Raw_t nsInput,
                          const fp16Raw_t x,
                          const fp16Raw_t y ) noexcept
{
    fp16Raw_t retValue = nsInput;

    if ( saturation ) {
        if ( overflow == nsInput ) {
            retValue = ( ( x >= 0 ) == ( y >= 0 ) ) ? Max : Min;
        }
    }

    return retValue;
}
/*============================================================================*/
fp16Raw_t fp16::add( const fp16Raw_t X,
                     const fp16Raw_t Y ) noexcept
{
    const uint32_t x = static_cast<uint32_t>( X );
    const uint32_t y = static_cast<uint32_t>( Y );
    uint32_t retValue;

    retValue =  x + y;
    if ( ( 0U == ( ( x ^ y ) & overflow_mask ) ) && ( 0U != ( ( x ^ retValue ) & overflow_mask ) ) ) {
        retValue = static_cast<uint32_t>( overflow );
    }

    return saturate( static_cast<fp16Raw_t>( retValue ), X, X );
}
/*============================================================================*/
fp16Raw_t fp16::sub( const fp16Raw_t X,
                     const fp16Raw_t Y ) noexcept
{
    const uint32_t x = static_cast<uint32_t>( X );
    const uint32_t y = static_cast<uint32_t>( Y );
    uint32_t retValue;

    retValue =  x - y;
    if ( ( 0U != ( ( x ^ y ) & overflow_mask ) ) && ( 0U != ( ( x ^ retValue ) & overflow_mask ) ) ) {
        retValue = static_cast<uint32_t>( overflow );
    }

    return saturate( static_cast<fp16Raw_t>( retValue ), X, X );
}
/*============================================================================*/
fp16Raw_t fp16::mul( const fp16Raw_t x,
                     const fp16Raw_t y ) noexcept
{
    fp16Raw_t retValue = overflow;
    fp16Raw_t a, c, ac, ad_cb, mulH;
    uint32_t b, d, bd, tmp, mulL;

    a = ( x >> 16 );
    c = ( y >> 16 );
    b = static_cast<uint32_t>( x & 0xFFFF );
    d = static_cast<uint32_t>( y & 0xFFFF );
    ac = a*c;
    ad_cb = static_cast<fp16Raw_t>( ( static_cast<uint32_t>( a )*d ) + ( static_cast<uint32_t>( c )*b ) );
    bd = b*d;
    mulH = ac + ( ad_cb >> 16 );
    tmp = static_cast<uint32_t>( ad_cb ) << 16;
    mulL = bd + tmp;
    if ( mulL < bd ) {
        ++mulH;
    }
    /*cstat -MISRAC++2008-5-0-3*/
    a = ( mulH < 0 ) ? -1 : 0;
    /*cstat +MISRAC++2008-5-0-3*/
    if ( a == ( mulH >> 15 ) ) {
        if ( flag_rounding ) {
            uint32_t tmp2;

            tmp2 = mulL;
            mulL -= static_cast<uint32_t>( one_half );
            mulL -= static_cast<uint32_t>( mulH ) >> 31;
            if ( mulL > tmp2 ) {
                --mulH;
            }
            retValue = static_cast<fp16Raw_t>( mulH << 16 ) | static_cast<fp16Raw_t>( mulL >> 16 );
            retValue += 1;
        }
        else {
            retValue = static_cast<fp16Raw_t>( mulH << 16 ) | static_cast<fp16Raw_t>( mulL >> 16 );
        }
    }

    return saturate( retValue, x, y );
}
/*============================================================================*/
fp16Raw_t fp16::div( const fp16Raw_t x,
                     const fp16Raw_t y ) noexcept
{
    fp16Raw_t retValue = Min;

    if ( 0 != y ) {
        uint32_t xRem, xDiv, bit = 0x10000U;

        xRem = static_cast<uint32_t>( ( x >= 0 ) ? x : -x );
        xDiv = static_cast<uint32_t>( ( y >= 0 ) ? y : -y );

        while ( xDiv < xRem ) {
            xDiv <<= 1;
            bit <<= 1;
        }
        retValue = overflow;
        /*cstat -MISRAC++2008-0-1-2_a*/
        if ( 0U != bit ) {
        /*cstat +MISRAC++2008-0-1-2_a*/
            uint32_t quotient = 0U;

            if ( 0U != ( xDiv & 0x80000000U ) ) {
                if ( xRem >= xDiv ) {
                    quotient |= bit;
                    xRem -= xDiv;
                }
                xDiv >>= 1;
                bit >>= 1;
            }

            while ( ( 0U != bit ) && ( 0U != xRem ) ) {
                if ( xRem >= xDiv ) {
                    quotient |= bit;
                    xRem -= xDiv;
                }
                xRem <<= 1;
                bit >>= 1;
            }
            if ( flag_rounding ) {
                if ( xRem >= xDiv ) {
                    ++quotient;
                }
            }

            retValue = static_cast<fp16Raw_t>( quotient );
            if ( 0U != ( static_cast<uint32_t>( x ^ y ) & overflow_mask ) ) {
                if ( quotient == static_cast<uint32_t>( Min ) ) {
                    retValue = overflow;
                }
                else {
                    retValue = -retValue;
                }
            }
        }
    }

    return saturate( retValue, x, y );
}
/*============================================================================*/
fp16Raw_t fp16::absolute( fp16Raw_t x ) noexcept
{
    fp16Raw_t retValue;

    if ( x == Min ) {
        retValue = overflow;
    }
    else {
        retValue = ( x >= 0 ) ? x : -x;
    }

    return retValue;
}
/*============================================================================*/
fp16Raw_t fp16::ceil( fp16Raw_t x ) noexcept
{
    const fp16Raw_t i = x & static_cast<fp16Raw_t>( integer_mask );
    const fp16Raw_t f = x & static_cast<fp16Raw_t>( fraction_mask );

    return ( 0 != f ) ? i + one : i;
}
/*============================================================================*/
fp16Raw_t fp16::sqrt( fp16Raw_t x ) noexcept
{
    fp16Raw_t retValue = overflow;

    if ( x > 0 ) {
        uint32_t bit;
        uint8_t n;

        retValue = 0;
        /*cstat -MISRAC++2008-5-0-3*/
        bit = ( 0 != ( x & static_cast<fp16Raw_t>( 4293918720 ) ) ) ? 1073741824U : 262144U;
        /*cstat +MISRAC++2008-5-0-3*/
        while ( bit > static_cast<uint32_t>( x ) ) {
            bit >>= 2U;
        }

        for ( n = 0U ; n < 2U ; ++n ) {
            while ( 0U != bit ) {
                if ( x >= static_cast<fp16Raw_t>( static_cast<uint32_t>( retValue ) + bit ) ) {
                    x -= static_cast<fp16Raw_t>( static_cast<uint32_t>( retValue ) + bit );
                    retValue = static_cast<fp16Raw_t>( ( static_cast<uint32_t>( retValue ) >> 1U ) + bit );
                }
                else {
                    retValue = ( retValue >> 1 );
                }
                bit >>= 2U;
            }

            if ( 0U == n ) {
                if ( x > 65535 ) {
                    x -= retValue;
                    x = ( x << 16 ) - one_half;
                    retValue = ( retValue << 16 ) + one_half;
                }
                else {
                    x <<= 16;
                    retValue <<= 16;
                }
                /*cstat -ATH-shift-bounds -MISRAC++2008-5-8-1 -CERT-INT34-C_b*/
                bit = 1U << 14U;
                /*cstat +ATH-shift-bounds +MISRAC++2008-5-8-1 +CERT-INT34-C_b*/
            }
        }
    }
    if ( ( flag_rounding ) && ( x > retValue ) ) {
        ++retValue;
    }

    return static_cast<fp16Raw_t>( retValue );
}
/*============================================================================*/
fp16Raw_t fp16::exp( fp16Raw_t x ) noexcept
{
    fp16Raw_t retValue, term;
    bool isNegative;
    int i;

    if ( 0 == x ) {
        retValue = one;
    }
    else if ( x == one ) {
        const fp16Raw_t f_e = 178145;
        retValue = f_e;
    }
    else if ( x >= exp_max ) {
        retValue = Max;
    }
    else if ( x <= -exp_max ) {
        retValue = 0;
    }
    else {
        isNegative = ( x < 0 );
        if ( isNegative ) {
            x = -x;
        }

        retValue = x + one;
        term = x;

        for ( i = 2 ; i < 30 ; ++i ) {
            term = mul( term, div( x, fromInt( i ) ) );
            retValue += term;

            if ( ( term < 500 ) && ( ( i > 15 ) || ( term < 20 ) ) ) {
                break;
            }
        }

        if ( isNegative ) {
            retValue = div( one, retValue );
        }
    }

    return retValue;
}
/*============================================================================*/
fp16Raw_t fp16::log( fp16Raw_t x ) noexcept
{
    fp16Raw_t retValue = overflow;
    static const fp16Raw_t e4 = 3578144; /*e^4*/

    if ( x > 0 ) {
        fp16Raw_t guess = f2, delta;
        const fp16Raw_t f100 = 6553600;
        const fp16Raw_t f3 = 196608;
        int scaling = 0, count = 0;

        while ( x > f100 ) {
            x = div( x, e4 );
            scaling += 4;
        }

        while ( x < one ) {
            x = mul( x, e4 );
            scaling -= 4;
        }

        do {
            const fp16Raw_t e = exp( guess );

            delta = div( x - e , e );

            if ( delta > f3 ) {
                delta = f3;
            }
            guess += delta;
        } while ( ( count++ < 10 ) && ( ( delta > 1 ) || ( delta < -1 ) ) );

        retValue = guess + fromInt( scaling );
    }

    return retValue;
}
/*============================================================================*/
fp16Raw_t fp16::rs( fp16Raw_t x ) noexcept
{
    fp16Raw_t retValue;

    if ( flag_rounding ) {
        retValue = ( x >> 1U ) + ( x & 1 );
    }
    else {
        retValue = x >> 1;
    }

    return retValue;
}
/*============================================================================*/
fp16Raw_t fp16::log2i( fp16Raw_t x ) noexcept
{
    fp16Raw_t retValue = 0;

    while ( x >= f2 ) {
        ++retValue;
        x = rs( x );
    }

    if ( 0 == x ) {
        retValue = retValue << 16;
    }
    else {
        int i;
        for ( i = 16 ; i > 0 ; --i ) {
            x = mul( x, x );
            retValue <<= 1;
            if ( x >= f2 ) {
                retValue |= 1;
                x = rs( x );
            }
        }
        if ( flag_rounding ) {
            x = mul( x, x );
            if ( x >= f2 ) {
                ++retValue;
            }
        }
    }

    return retValue;
}
/*============================================================================*/
fp16Raw_t fp16::log2( fp16Raw_t x ) noexcept
{
    fp16Raw_t retValue = overflow;

    if ( x > 0 ) {
        if ( x < one ) {
            if ( 1 == x ) {
                const fp16Raw_t f16 = 1048576;
                retValue = -f16;
            }
            else {
                fp16Raw_t inv;
                inv = div( one, x );
                retValue = -log2i( inv );
            }
        }
        else {
            retValue = log2i( x );
        }
    }
    if ( saturation ) {
        if ( overflow == retValue ) {
            retValue = Min;
        }
    }

    return retValue;
}
/*============================================================================*/
fp16Raw_t fp16::radToDeg( const fp16Raw_t x )
{
    const fp16Raw_t f_180_pi = 3754936;
    return mul( wrapToPi( x ), f_180_pi );
}
/*============================================================================*/
fp16Raw_t fp16::degToRad( const fp16Raw_t x )
{
    const fp16Raw_t f_pi_180 = 1144;
    return mul( wrapTo180( x ), f_pi_180 );
}
/*============================================================================*/
fp16Raw_t fp16::wrapToPi( fp16Raw_t x ) noexcept
{
    const fp16Raw_t f_pi = 205887;

    if ( ( x < -f_pi ) || ( x > f_pi ) ) {
        const fp16Raw_t f_2pi = 411775;
        while ( x > f_pi ) {
            x -= f_2pi;
        }
        while ( x <= -f_pi ) {
            x += f_2pi;
        }
    }

    return x;
}
/*============================================================================*/
fp16Raw_t fp16::wrapTo180( fp16Raw_t x ) noexcept
{
    const fp16Raw_t F_180 = 11796480;

    if ( ( x < -F_180 ) || ( x > F_180 ) ) {
        const fp16Raw_t F_360 = 23592960;

        while ( x > F_180 ) {
            x -= F_360;
        }
        while ( x <= -F_180 ) {
            x += F_360;
        }
    }

    return x;
}
/*============================================================================*/
fp16Raw_t fp16::sin( fp16Raw_t x ) noexcept
{
    fp16Raw_t retValue, x2;

    x = wrapToPi( x );
    x2 = mul( x ,x );
    retValue = x;
    x = mul( x, x2 );
    retValue -= ( x / 6 ); /*x^3/3!*/
    x = mul( x, x2 );
    retValue += ( x / 120 ); /*x^5/5!*/
    x = mul( x, x2 );
    retValue -= ( x / 5040 ); /*x^7/7!*/
    x = mul( x, x2 );
    retValue += ( x / 362880 ); /*x^9/9!*/
    x = mul( x, x2);
    retValue -= ( x / 39916800 ); /*x^11/11!*/

    return retValue;
}
/*============================================================================*/
fp16Raw_t fp16::cos( fp16Raw_t x ) noexcept
{
    return sin( x + f_pi_2 );
}
/*============================================================================*/
fp16Raw_t fp16::tan( fp16Raw_t x ) noexcept
{
    fp16Raw_t a ,b;

    a = sin( x );
    b = cos( x );

    return div( a, b );
}
/*============================================================================*/
fp16Raw_t fp16::atan2( fp16Raw_t y,
                       fp16Raw_t x ) noexcept
{
    fp16Raw_t absY, mask, angle, r, r_3;
    const fp16Raw_t QFP16_0_981689 = 0x0000FB50;
    const fp16Raw_t QFP16_0_196289 = 0x00003240;
    static const fp16Raw_t f_3pi_div_4 = 154415; /*3*pi/4*/

    mask = ( y >> ( sizeof(fp16Raw_t)*7U ) );
    absY = ( y + mask ) ^ mask;
    if ( x >= 0 ) {
        r = div( ( x - absY ), ( x + absY ) );
        const fp16Raw_t f_pi_4 = 51471;
        angle = f_pi_4;
    }
    else {
        r = div( ( x + absY ), ( absY - x ) );
        angle = f_3pi_div_4;
    }
    r_3 = mul( mul( r, r ), r );
    angle += mul( QFP16_0_196289, r_3 ) - mul( QFP16_0_981689, r );
    /*cstat -ATH-neg-check-nonneg*/
    if ( y < 0 ) {
        angle = -angle;
    }
    /*cstat +ATH-neg-check-nonneg*/
    return angle;
}
/*============================================================================*/
fp16Raw_t fp16::atan( fp16Raw_t x ) noexcept
{
    return atan2( x, one );
}
/*============================================================================*/
fp16Raw_t fp16::asin( fp16Raw_t x ) noexcept
{
    fp16Raw_t retValue = 0;

    if ( ( x <= one ) && ( x >= -one ) ) {
        retValue = one - mul( x, x );
        retValue = div( x, sqrt( retValue ) );
        retValue = atan( retValue );
    }

    return retValue;
}
/*============================================================================*/
fp16Raw_t fp16::acos( fp16Raw_t x ) noexcept
{
    return ( f_pi_2 - asin( x ) );
}
/*============================================================================*/
fp16Raw_t fp16::cosh( fp16Raw_t x ) noexcept
{
    fp16Raw_t retValue = overflow;
    fp16Raw_t epx, enx;

    if ( 0 == x ) {
        retValue = one;
    }
    else if ( ( x >= exp_max ) || ( x <= -exp_max ) ) {
        retValue = Max;
    }
    else {
        epx = exp( x );
        enx = exp( -x );
        if ( ( overflow != epx ) && ( overflow != enx ) ) {
            retValue = epx + enx;
            retValue = ( retValue >> 1 );
        }
    }

    return retValue;
}
/*============================================================================*/
fp16Raw_t fp16::sinh( fp16Raw_t x ) noexcept
{
    fp16Raw_t retValue = overflow;
    fp16Raw_t epx, enx;

    if ( 0 == x ) {
        retValue = one;
    }
    else if ( x >= exp_max ) {
        retValue = Max;
    }
    else if ( x <= -exp_max ) {
        retValue = -Max;
    }
    else {
        epx = exp( x );
        enx = exp( -x );
        if ( ( overflow != epx ) && ( overflow != enx ) ) {
            retValue = epx - enx;
            retValue = ( retValue >> 1 );
        }
    }

    return retValue;
}
/*============================================================================*/
fp16Raw_t fp16::tanh( fp16Raw_t x ) noexcept
{
    fp16Raw_t retValue, epx, enx;
    const fp16Raw_t f6_5 = 425984;

    if ( 0 == x ) {
        retValue = 0;
    }
    else if ( x >  f6_5 ) { /* tanh for any x>6.5 ~= 1*/
        retValue = one;
    }
    else if ( x < -f6_5 ) { /* tanh for any x<6.5 ~= -1*/
        retValue = -one;
    }
    else {
        retValue = absolute( x );
        epx = exp( retValue );
        enx = exp( -retValue );
        retValue = div( epx - enx, epx + enx );
        retValue = ( x > 0 ) ? retValue : -retValue;
    }

    return retValue;
}
/*============================================================================*/
fp16Raw_t fp16::powi( fp16Raw_t x,
                      fp16Raw_t y ) noexcept
{
    fp16Raw_t retValue;
    fp16Raw_t n;
    int32_t i;

    retValue = one;
    n = y >> 16;
    if ( 0 == n ) {
        retValue = one;
    }
    else if ( one == n ) {
        retValue = x;
    }
    else {
        for ( i = 0 ; i < n ; ++i ) {
            retValue = mul( x, retValue );
            if ( overflow == retValue ) {
                break;
            }
        }
    }

    return retValue;
}
/*============================================================================*/
fp16Raw_t fp16::pow( fp16Raw_t x,
                     fp16Raw_t y ) noexcept
{
    fp16Raw_t retValue = overflow;

    if ( ( 0U == ( static_cast<uint32_t>( y ) & fraction_mask ) ) && ( y > 0 ) ) {
        retValue = powi( x, y );
    }
    else {
        fp16Raw_t tmp;
        tmp = mul( y, log( absolute( x ) ) );
        if ( overflow != tmp ) {
            retValue = exp( tmp );
            if ( x < 0 ) {
                retValue = -retValue;
            }
        }
    }

    return retValue;
}
/*============================================================================*/
char* fp16::itoa( char *buf,
                  uint32_t scale,
                  uint32_t val,
                  uint8_t skip ) noexcept
{
    while ( 0U != scale ) {
        const uint32_t digit = ( val / scale );
        if ( ( 0U == skip ) || ( 0U != digit ) || ( 1U == scale ) ) {
            skip = 0U;
            /*cstat -MISRAC++2008-5-0-3*/
            *buf++ = static_cast<char>( '0' ) + static_cast<char>( digit );
            /*cstat +MISRAC++2008-5-0-3*/
            val %= scale;
        }
        scale /= 10U;
    }

    return buf;
}
/*============================================================================*/
char* fp16::toASCII( const fp16Raw_t num,
                     char *str,
                     int decimals ) noexcept
{
    char * const retValue = str;

    if ( overflow == num ) {
        str[ 0 ] = 'o';
        str[ 1 ] = 'v';
        str[ 2 ] = 'e';
        str[ 3 ] = 'r';
        str[ 4 ] = 'f';
        str[ 5 ] = 'l';
        str[ 6 ] = 'o';
        str[ 7 ] = 'w';
        str[ 8 ] = '\0';
    }
    else {
        const uint32_t iScales[ 6 ] = { 1U, 10U, 100U, 1000U, 10000U, 100000U };
        uint32_t uValue, fPart, scale;
        fp16Raw_t iPart;

        uValue = static_cast<uint32_t>( ( num >= 0 ) ? num : -num );
        if ( num < 0 ) {
            *str++ = '-';
        }

        iPart = static_cast<fp16Raw_t>( uValue >> 16 );
        fPart = uValue & fraction_mask;
        if ( decimals > 5 ) {
            decimals = 5;
        }
        if ( decimals < 0 ) {
            decimals = 0;
        }
        scale = iScales[ decimals ];
        fPart = static_cast<uint32_t>( mul( static_cast<fp16Raw_t>( fPart ), static_cast<fp16Raw_t>( scale ) ) );

        if ( fPart >= scale ) {
            iPart++;
            fPart -= scale;
        }
        str = itoa( str, 10000U, static_cast<uint32_t>( iPart ), 1U );

        if ( 1U != scale ) {
            *str++ = '.';
            str = itoa( str, scale/10U, fPart, 0U );
        }
        *str = '\0';
    }

    return retValue;
}
/*============================================================================*/

/*cstat +MISRAC++2008-5-0-21 +MISRAC++2008-5-0-9 +ATH-shift-neg +CERT-INT34-C_c +MISRAC++2008-5-0-10*/