#include <include/pid.hpp>
#include <include/ffmath.hpp>

using namespace qlibs;


const uint32_t pidAutoTuning::UNDEFINED = 0xFFFFFFFEUL;

/*============================================================================*/
bool pidController::setup( const real_t kc,
                           const real_t ki,
                           const real_t kd,
                           const real_t dT ) noexcept
{
    bool retValue = false;

    if ( dt > 0.0_re ) {
        dt = dT;
        (void)setDerivativeFilter( 0.98_re );
        (void)setEpsilon( REAL_MIN );
        (void)setGains( kc, ki, kd );
        (void)setSaturation( 0.0_re, 100.0_re );
        (void)setMode( pidMode::PID_AUTOMATIC );
        (void)setManualInput( 0.0_re );
        (void)setExtraGains( 1.0_re, 1.0_re );
        (void)setDirection( pidDirection::PID_FORWARD );
        (void)setReferenceWeighting( 1.0_re, 0.0_re );
        init = true;
        retValue = reset();
    }

    return retValue;
}
/*============================================================================*/
bool pidController::setDirection( const pidDirection d ) noexcept
{
    bool retValue = false;

    if ( init ) {
        dir = d;
        retValue = true;
    }

    return retValue;
}
/*============================================================================*/
bool pidController::setParams( const real_t kc,
                               const real_t ti,
                               const real_t td ) noexcept
{
    bool retValue = false;

    if ( init ) {
        Kc = kc;
        Ki = kc/ti;
        Kd = kc*td;
        retValue = true;
    }

    return retValue;
}
/*============================================================================*/
bool pidController::setGains( const real_t kc,
                              const real_t ki,
                              const real_t kd ) noexcept
{
    bool retValue = false;

    if ( init ) {
        Kc = kc;
        Ki = ki;
        Kd = kd;
        retValue = true;
    }

    return retValue;
}
/*============================================================================*/
bool pidController::setGains( const pidGains &g ) noexcept
{
    bool retValue = false;

    if ( init ) {
        Kc = g.Kc;
        Ki = g.Ki;
        Kd = g.Kd;
        retValue = true;
    }

    return retValue;
}
/*============================================================================*/
bool pidController::setExtraGains( const real_t Kw,
                                   const real_t Kt ) noexcept
{
    bool retValue = false;

    if ( init ) {
        kw = Kw;
        kt = Kt;
        retValue = true;
    }

    return retValue;
}
/*============================================================================*/
bool pidController::setSaturation( const real_t Min,
                                   const real_t Max ) noexcept
{
    bool retValue = false;

    if ( init && ( Max > Min ) ) {
        min = Min;
        max = Max;
        retValue = true;
    }

    return retValue;
}
/*============================================================================*/
bool pidController::setSeries( void ) noexcept
{
    bool retValue = false;

    if ( init ) {
        real_t ti, td, tmp;

        ti = Kc/Ki;
        td = Kd/Kc;
        tmp = 1.0_re + ( td/ti );
        Kc = Kc*tmp;
        Ki = Kc/( ti*tmp );
        Kd = Kc*( td/tmp );
        retValue = true;
    }

    return retValue;
}
/*============================================================================*/
bool pidController::setEpsilon( const real_t eps ) noexcept
{
    bool retValue = false;

    if ( init ) {
        epsilon = eps;
        retValue = true;
    }

    return retValue;
}
/*============================================================================*/
bool pidController::setDerivativeFilter( const real_t Beta ) noexcept
{
    bool retValue = false;

    if ( init ) {
        beta = Beta;
        retValue = true;
    }

    return retValue;
}
/*============================================================================*/
bool pidController::setMode( const pidMode Mode ) noexcept
{
    bool retValue = false;

    if ( init ) {
        mode = Mode;
        retValue = true;
    }

    return retValue;
}
/*============================================================================*/
bool pidController::setReferenceWeighting( const real_t gb,
                                           const real_t gc ) noexcept
{
    bool retValue = false;

    if ( init ) {
        b = saturate( gb, 0.0_re, 1.0_re );
        c = saturate( gc, 0.0_re, 1.0_re );
        retValue = true;
    }

    return retValue;
}
/*============================================================================*/
bool pidController::setManualInput( const real_t manualInput ) noexcept
{
    bool retValue = false;

    if ( init ) {
        mInput = manualInput;
        retValue = true;
    }

    return retValue;
}
/*============================================================================*/
bool pidController::reset( void ) noexcept
{
    bool retValue = false;

    if ( init ) {
        c_state.init();
        m_state.init();
        b_state.init();
        D = 0.0_re;
        u1 = 0.0_re;
        m = 0.0_re;
        retValue = true;
    }

    return retValue;
}
/*============================================================================*/
bool pidController::setModelReferenceControl( const real_t &modelRef,
                                              const real_t Gamma,
                                              const real_t Alpha ) noexcept
{
    bool retValue = false;

    if ( init && ( Gamma > 0.0_re ) && ( Alpha > 0.0_re ) ) {
        m_state.init();
        alpha = Alpha;
        gamma = Gamma;
        yr = &modelRef;
        retValue = true;
    }

    return retValue;
}
/*============================================================================*/
bool pidController::removeModelReferenceControl( void ) noexcept
{
    bool retValue = false;

    if ( init ) {
        yr = nullptr;
        retValue = true;
    }

    return retValue;
}
/*============================================================================*/
real_t pidController::control( const real_t w,
                               const real_t y ) noexcept
{
    real_t u = w;

    if ( init ) {
        real_t e, v, de, ie, bt, sw, kc, ki, kd;
        kc = Kc;
        ki = Ki;
        kd = Kd;
        if ( pidDirection::PID_BACKWARD == dir ) {
            kc = ( kc > 0.0_re ) ? -kc : kc;
            ki = ( ki > 0.0_re ) ? -ki : ki;
            kd = ( kd > 0.0_re ) ? -kd : kd;
        }
        e = w - y;
        if ( ffmath::absf( e ) <= epsilon ) {
            e = 0.0_re;
        }
        ie = c_state.integrate( e + u1, dt );
        de = c_state.derive( ( c*w ) - y , dt );
        D = de + beta*( D - de ); /*derivative filtering*/
        v  = ( kc*( ( b*w ) - y ) ) + ( ki*ie ) + ( kd*D ); /*compute PID action*/
        if ( nullptr != yr ) {
            /*MRAC additive controller using the modified MIT rule*/
            real_t theta = 0.0_re;
            if ( ffmath::absf( u1 ) <= epsilon ) { /*additive anti-windup*/
                const real_t em = y - yr[ 0 ];
                const real_t delta = -gamma*em*yr[ 0 ]/( alpha + ( yr[ 0 ]*yr[ 0 ] ) );
                theta = m_state.integrate( delta /*+ c->u1*/, dt );
            }
            v += w*theta;
        }
        /*bumpless-transfer*/
        bt = ( kt*mInput ) + ( kw*( u - m ) );
        m = b_state.integrate( bt, dt );
        sw = ( pidMode::PID_AUTOMATIC == mode ) ? v : m;
        u = saturate( sw, min, max );
        /*anti-windup feedback*/
        u1 = kw*( u - v );
        if ( nullptr != adapt ) {
            adaptGains( u, y );
        }
    }

    return u;
}
/*============================================================================*/
bool pidAutoTuning::step( const real_t u,
                          const real_t y,
                          const real_t dt ) noexcept
{
    real_t error , r, l0, l1;
    real_t lp00, lp01, lp10, lp11;
    real_t tmp1, tmp2;
    real_t gain, timeConstant;
    bool ready = false;

    tmp1 = p00*uk;
    tmp2 = p11*yk;
    r = l +( uk*( tmp1 - ( p10*yk ) ) ) - ( yk*( ( p01*uk ) - tmp2 ) );
    /*compute corrections*/
    l0 = ( tmp1 - ( p01*yk ) )/r;
    l1 = ( ( p10*uk ) - tmp2 )/r;
    error = y - ( ( b1*uk ) - ( a1*yk ) );
    /*fix estimations*/
    b1 += l0*error;
    a1 += l1*error;
    /*update covariances*/
    lp00 = il*p00;
    lp01 = il*p01;
    lp10 = il*p10;
    lp11 = il*p11;
    tmp1 = ( l0*uk ) - 1.0_re;
    tmp2 = ( l1*yk ) + 1.0_re;
    p00 = ( l0*lp10*yk ) - ( lp00*tmp1 ) + 1e-10_re;
    p01 = ( l0*lp11*yk ) - ( lp01*tmp1 );
    p10 = ( lp10*tmp2 ) - ( l1*lp00*uk );
    p11 = ( lp11*tmp2 ) - ( l1*lp01*uk ) + 1e-10_re;
    /*update I/O measurements*/
    yk = y;
    uk = u;
    gain = b1/( 1.0_re + a1 );
    timeConstant = -dt/ffmath::log( ffmath::absf( a1 ) );
    /*cstat -MISRAC++2008-5-14-1*/
    if ( isValidValue( timeConstant ) && isValidValue( gain ) && ( it > 0UL ) ) { /*no side effects here*/
    /*cstat +MISRAC++2008-5-14-1*/
        k = gain + ( mu*( k - gain ) );
        tao = timeConstant + ( mu*( tao - timeConstant ) );
        if ( ( 0UL == --it ) && ( pidAutoTuning::UNDEFINED != it ) ) {
            ready = true;
        }
    }

    return ready;
}
/*============================================================================*/
pidGains pidAutoTuning::getEstimates( const real_t dt ) const noexcept
{
    pidGains gains;
    real_t tmp1, tmp2;

    tmp1 = dt/tao;
    tmp2 = ( 1.35_re + ( 0.25_re*tmp1 ) );
    gains.Kc = ( speed*tmp2*tao )/( k*dt );
    gains.Ki = ( ( speed*gains.Kc )*( 0.54_re + ( 0.33_re*tmp1 ) ) )/( tmp2*dt );
    gains.Kd = ( 0.5_re*speed*gains.Kc*dt )/tmp2;

    return gains;
}
/*============================================================================*/
void pidController::adaptGains( const real_t u,
                                const real_t y ) noexcept
{
    if ( adapt->step( u, y, dt ) ) {
        pidGains newGains = adapt->getEstimates( dt );
        Kc = newGains.Kc;
        Ki = newGains.Ki;
        Kd = newGains.Kd;
    }
}
/*============================================================================*/
bool pidController::bindAutoTuning( pidAutoTuning &at ) noexcept
{
    bool retValue = false;

    if ( init ) {
        real_t k, T;

        adapt = &at;
        at.l = 0.9898_re;
        at.il = 1.0_re/at.l;
        at.p00 = 1000.0_re;
        at.p11 = 1000.0_re;
        at.p01 = 0.0_re;
        at.p10 = 0.0_re;
        at.uk  = 0.0_re;
        at.yk = 0.0_re;
        at.k = 0.0_re;
        at.tao = 0.0_re;
        at.it = 100UL;
        at.mu = 0.95_re;
        k = Kc/0.9_re;
        T = ( 0.27_re*k )/Ki;
        /*cstat -CERT-FLP32-C_b*/
        at.a1 = -ffmath::exp( -dt/T );
        /*cstat +CERT-FLP32-C_b*/
        at.b1 = k*( 1.0_re + at.a1 );
        at.speed = 0.25_re;
        at.it = pidAutoTuning::UNDEFINED;
        retValue = true;
    }

    return retValue;
}
/*============================================================================*/
bool pidController::enableAutoTuning( const uint32_t tEnable ) noexcept
{
    bool retValue = false;

    if ( nullptr != adapt ) {
        adapt->it = ( 0UL == tEnable ) ? pidAutoTuning::UNDEFINED : tEnable;
        retValue = true;
    }

    return retValue;
}
/*============================================================================*/
bool pidController::isAutoTuningComplete( void ) const noexcept
{
    bool retValue = false;

    if ( nullptr != adapt ) {
        retValue = ( ( 0UL == adapt->it ) && ( adapt->it != pidAutoTuning::UNDEFINED ) );
    }

    return retValue;
}
/*============================================================================*/
bool pidController::setAutoTuningParameters( const real_t Mu,
                                             const real_t Alpha,
                                             const real_t lambda ) noexcept
{
    bool retValue = false;

    if ( nullptr != adapt ) {
        if ( ( Mu > 0.0_re ) && ( Mu <= 1.0_re ) && ( Alpha > 0.0_re ) && ( Alpha <= 1.0_re ) && ( lambda >= 0.8_re ) && ( lambda <= 1.0_re ) ) {
            adapt->l = lambda;
            adapt->mu = Mu;
            adapt->speed = Alpha;
            retValue = true;
        }
    }

    return retValue;
}
/*============================================================================*/
bool pidAutoTuning::isValidValue( const real_t x ) noexcept
{
     /*cstat -MISRAC++2008-5-14-1*/
     return ( !ffmath::isNan( x ) ) && ( x > 0.0_re ) && ( !ffmath::isInf( x ) );
     /*cstat +MISRAC++2008-5-14-1*/
}
/*============================================================================*/
real_t pidController::saturate( real_t x,
                                const real_t vMin,
                                const real_t vMax ) noexcept
{
    if ( x > vMax ) {
        x = vMax;
    }
    else if ( x < vMin ) {
        x = vMin;
    }
    else {
        /*nothing to do*/
    }

    return x;
}
/*============================================================================*/
