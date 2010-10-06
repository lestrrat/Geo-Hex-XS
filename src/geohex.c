/*
 * This file was generated automatically by ExtUtils::ParseXS version 2.21 from the
 * contents of geohex.xs. Do not edit this file, edit geohex.xs instead.
 *
 *	ANY CHANGES MADE HERE WILL BE LOST! 
 *
 */

#line 1 "src/geohex.xs"
#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

int X60POW[] = {
    1,
    60,
    3600,
    216000,
    12960000,
    77600000
};

#define H_KEY_COUNT 62
char H_KEY[] = {
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
    'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C',
    'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
    'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z'
};

#define H_BASE  (20037508.34)
#define H_DEG   (M_PI * ( 30.0 / 180.0 ))
#define H_K     (tan(H_DEG))
#define H_RANGE 21
#define GEOHEX_CODE_BUFSIZ 4096

typedef struct {
    double lat;
    double lon;
    double x;
    double y;
    char code[ GEOHEX_CODE_BUFSIZ ];
} PerlGeoHexZone;

static double
hex_size (double level) {
    return (H_BASE / pow(2.0, level)) / 3.0;
}

static int
loc_2xy ( double lon, double lat, double *x, double *y ) {
    *x = lon * H_BASE / 180;
    *y = H_BASE * log( tan( (90 + lat ) * M_PI / 360 ) ) / ( M_PI / 180 ) / 180;
    return 1;
}

static int
xy_2loc ( double x, double y, double *lon, double *lat ) {
    *lon = ( x / H_BASE ) * 180;
    *lat = ( y / H_BASE ) * 180;
    *lat = 180 / M_PI * ( 2 * atan( exp( *lat * M_PI / 180 ) ) - M_PI / 2 );
    return 1;
}

static int
get_code_by_xy( char **code, int x, int y, double max, int level ) {
    int i = 0;
    char buf[3];

    snprintf(*code, GEOHEX_CODE_BUFSIZ, "%c", H_KEY[ level % 60 ] );

    for ( i = 4; i > 0; i-- ) {
        double current = (double) X60POW[i];

        if (max >= current / 2) {
            int above = X60POW[i + 1];
            snprintf(buf, 3, "%c%c",
                H_KEY[ ((int) floor( ( x % above) / current )) ],
                H_KEY[ ((int) floor( ( y % above) / current )) ]
            );
            strncat(*code, buf, GEOHEX_CODE_BUFSIZ);
        }
    }

    snprintf(buf, 3, "%c%c",
        H_KEY[ ((int) floor( ( x % 3600 ) % 60 ) ) ],
        H_KEY[ ((int) floor( ( y % 3600 ) % 60 ) ) ]
    );
    strncat(*code, buf, GEOHEX_CODE_BUFSIZ);

    return 1;
}



static int
get_zone_by_location (PerlGeoHexZone *zone, double lat, double lon, int level) {
    double lon_grid, lat_grid;
    double unit_x, unit_y;
    double h_pos_x, h_pos_y;
    double h_x_0, h_y_0;
    double h_x_q, h_y_q;
    double h_x_p, h_y_p;
    double h_x_abs, h_y_abs;
    double h_x, h_y;
    double h_max;
    double h_lat, h_lon;
    double z_loc_x, z_loc_y;
    double h_size = hex_size(level);

    loc_2xy( lon, lat, &lon_grid, &lat_grid );

    unit_x = 6.0 * h_size;
    unit_y = 6.0 * h_size * H_K;
    h_pos_x = ( lon_grid + lat_grid / H_K ) / unit_x;
    h_pos_y = ( lat_grid - H_K * lon_grid ) / unit_y;
    h_x_0   = floor(h_pos_x);
    h_y_0   = floor(h_pos_y);
    h_x_q   = h_pos_x - h_x_0;
    h_y_q   = h_pos_y - h_y_0;
    h_x     = round(h_pos_x);
    h_y     = round(h_pos_y);

    h_max   = round( H_BASE / unit_x + H_BASE / unit_y );
    if ( h_y_q > -1 * h_x_q + 1 ) {
        if ( ( h_y_q < 2 * h_x_q ) && ( h_y_q > 0.5 * h_x_q ) ) {
            h_x = h_x_0 + 1;
            h_y = h_y_0 + 1;
        }
    } else if ( h_y_q < -1 * h_x_q + 1.0 ) {
        if ( ( h_y_q > ( 2 * h_x_q ) - 1.0 ) && ( h_y_q < ( 0.5 * h_x_q ) + 0.5 ) ) {
            h_x = h_x_0;
            h_y = h_y_0;
        }
    }

    h_lat = ( H_K * h_x * unit_x + h_y * unit_y ) / 2.0;
    h_lon = ( h_lat - h_y * unit_y ) / H_K;

    xy_2loc( h_lon, h_lat, &z_loc_x, &z_loc_y );

    if ( H_BASE - h_lon < h_size ) {
        int tmp;
        z_loc_x = 180;
        tmp = h_x;
        h_x = h_y;
        h_y = tmp;
    }
        
    h_x_p = 0;
    h_y_p = 0;
    if (h_x < 0 ) {
        h_x_p = 1;
    }
    if (h_y < 0) {
        h_y_p = 1;
    }

    h_x_abs = abs( h_x ) * 2 + h_x_p;
    h_y_abs = abs( h_y ) * 2 + h_y_p;

    zone->lat = z_loc_y;
    zone->lon = z_loc_x;
    zone->x   = h_x == 0 ? 0 : h_x;
    zone->y   = h_y == 0 ? 0 : h_y;

    {
        char *h_code;
        Newxz(h_code, GEOHEX_CODE_BUFSIZ, char);
        get_code_by_xy( &h_code, (int) h_x_abs, (int) h_y_abs, h_max, level );
        Copy(h_code, zone->code, GEOHEX_CODE_BUFSIZ, char);
        Safefree(h_code);
    }

    return 1;
}

static int
get_index_of_h_key( char k ) {
    int i;
    for ( i = 0; i < H_KEY_COUNT; i++ ) {
        if (H_KEY[i] == k) {
            return i;
        }
    }
    croak("Could not find appropriate H_KEY in given code");
}

static int
get_zone_by_code( PerlGeoHexZone *zone, char *code ) {
    int i;
    int level = get_index_of_h_key( *code );
    double h_size;
    double h_max;
    double unit_x, unit_y;
    double h_x = 0, h_y = 0;
    double h_lon, h_lat;
    double h_lon_x, h_lat_y;

    h_size = hex_size(level);
    unit_x = h_size * 6.0;
    unit_y = h_size * 6.0 * H_K;
    h_max  = round( H_BASE / unit_x + H_BASE / unit_y );

    for (i = 4; i >= 0; i--) {
        if ( h_max >= X60POW[i] / 2 ) {
            int j;
            for ( j = i; j >= 0; j-- ) {
                h_x += get_index_of_h_key( *(code + (i - j) * 2 + 1) ) * X60POW[j];
                h_y += get_index_of_h_key( *(code + (i - j + 1) * 2) ) * X60POW[j];
            }
            break;
        }
    }

    h_x = ((int) h_x % 2) ? -1 * (h_x - 1) / 2.0 : h_x / 2.0;
    h_y = ((int) h_y % 2) ? -1 * (h_y - 1) / 2.0 : h_y / 2.0;

    h_lat_y = ( H_K * h_x * unit_x + h_y * unit_y ) / 2.0;
    h_lon_x = ( h_lat_y - h_y * unit_y ) / H_K;

    xy_2loc( h_lon_x, h_lat_y, &h_lon, &h_lat );

    Copy( code, zone->code, strlen(code), char );
    zone->lat  = h_lat;
    zone->lon  = h_lon;
    zone->x    = h_x == 0 ? 0 : h_x;
    zone->y    = h_y == 0 ? 0 : h_y;

    return 1;
}

static int
get_steps( double start_x, double start_y, double end_x, double end_y ) {
    double x = end_x - start_x;
    double y = end_y - start_y;
    double x_abs = abs(x);
    double y_abs = abs(y);
    double m = 0;

    if (x_abs != 0 && x_abs != 0) {
        if ( x / x_abs > y / y_abs ) {
            m = x;
        } else {
            m = y;
        }
    }

    return x_abs + y_abs + abs(m) + 1;
}

static int
get_zone_by_xy( PerlGeoHexZone *zone, double x, double y, int level ) {
    double h_size = hex_size( level );
    double unit_x = 6 * h_size;
    double unit_y = 6 * h_size * H_K;
    double h_max = round( H_BASE / unit_x + H_BASE / unit_y );
    double h_lat_y = ( H_K * x * unit_x + y * unit_y ) / 2;
    double h_lon_x = ( h_lat_y - y * unit_y ) / H_K;
    double h_lat, h_lon;
    int x_p = x < 0 ? 1 : 0;
    int y_p = y < 0 ? 1 : 0;
    double h_x_abs = abs(x) * 2 + x_p;
    double h_y_abs = abs(y) * 2 + y_p;
    xy_2loc( h_lon_x, h_lat_y, &h_lon, &h_lat );

    {
        char *h_code;
        Newxz(h_code, GEOHEX_CODE_BUFSIZ, char);
        get_code_by_xy( &h_code, (int) h_x_abs, (int) h_y_abs, h_max, level );
        Copy(h_code, zone->code, GEOHEX_CODE_BUFSIZ, char);
        Safefree(h_code);
    }

    zone->lat = h_lat;
    zone->lon = h_lon;
    zone->x = x;
    zone->y = y;
    
    return 1;
}

#line 289 "src/geohex.c"
#ifndef PERL_UNUSED_VAR
#  define PERL_UNUSED_VAR(var) if (0) var = var
#endif

#ifndef PERL_ARGS_ASSERT_CROAK_XS_USAGE
#define PERL_ARGS_ASSERT_CROAK_XS_USAGE assert(cv); assert(params)

/* prototype to pass -Wmissing-prototypes */
STATIC void
S_croak_xs_usage(pTHX_ const CV *const cv, const char *const params);

STATIC void
S_croak_xs_usage(pTHX_ const CV *const cv, const char *const params)
{
    const GV *const gv = CvGV(cv);

    PERL_ARGS_ASSERT_CROAK_XS_USAGE;

    if (gv) {
        const char *const gvname = GvNAME(gv);
        const HV *const stash = GvSTASH(gv);
        const char *const hvname = stash ? HvNAME(stash) : NULL;

        if (hvname)
            Perl_croak(aTHX_ "Usage: %s::%s(%s)", hvname, gvname, params);
        else
            Perl_croak(aTHX_ "Usage: %s(%s)", gvname, params);
    } else {
        /* Pants. I don't think that it should be possible to get here. */
        Perl_croak(aTHX_ "Usage: CODE(0x%"UVxf")(%s)", PTR2UV(cv), params);
    }
}
#undef  PERL_ARGS_ASSERT_CROAK_XS_USAGE

#ifdef PERL_IMPLICIT_CONTEXT
#define croak_xs_usage(a,b)	S_croak_xs_usage(aTHX_ a,b)
#else
#define croak_xs_usage		S_croak_xs_usage
#endif

#endif

/* NOTE: the prototype of newXSproto() is different in versions of perls,
 * so we define a portable version of newXSproto()
 */
#ifdef newXS_flags
#define newXSproto_portable(name, c_impl, file, proto) newXS_flags(name, c_impl, file, proto, 0)
#else
#define newXSproto_portable(name, c_impl, file, proto) (PL_Sv=(SV*)newXS(name, c_impl, file), sv_setpv(PL_Sv, proto), (CV*)PL_Sv)
#endif /* !defined(newXS_flags) */

#line 341 "src/geohex.c"

XS(XS_Geo__Hex__XS_get_zone_by_code); /* prototype to pass -Wmissing-prototypes */
XS(XS_Geo__Hex__XS_get_zone_by_code)
{
#ifdef dVAR
    dVAR; dXSARGS;
#else
    dXSARGS;
#endif
    if (items != 1)
       croak_xs_usage(cv,  "code");
    {
	char *	code = (char *)SvPV_nolen(ST(0));
#line 287 "src/geohex.xs"
        PerlGeoHexZone zone;
#line 357 "src/geohex.c"
	SV *	RETVAL;
#line 289 "src/geohex.xs"
        get_zone_by_code( &zone, code);
        {
            dSP;
            int count = 0;
            SV *zone_sv;

            ENTER;
            SAVETMPS;
            PUSHMARK(SP);
            mXPUSHp( "Geo::Hex::XS::Zone", 18 );
            mXPUSHp( "lat",  3 );
            mXPUSHn( zone.lat );
            mXPUSHp( "lon",  3 );
            mXPUSHn( zone.lon );
            mXPUSHp( "x",  1 );
            mXPUSHn( zone.x );
            mXPUSHp( "y",  1 );
            mXPUSHn( zone.y );
            mXPUSHp( "code",  4 );
            mXPUSHp( zone.code, strlen(zone.code) );
            PUTBACK;

            count = call_method( "new", G_SCALAR );
            SPAGAIN;

            if (count < 1) {
                croak("Geo::Hex::XS::Zone::new did not return an object");
            }

            zone_sv = newSVsv(POPs);
            PUTBACK;
            FREETMPS;
            LEAVE;

            RETVAL = zone_sv;
        }
#line 396 "src/geohex.c"
	ST(0) = RETVAL;
	sv_2mortal(ST(0));
    }
    XSRETURN(1);
}


XS(XS_Geo__Hex__XS_get_zone_by_location); /* prototype to pass -Wmissing-prototypes */
XS(XS_Geo__Hex__XS_get_zone_by_location)
{
#ifdef dVAR
    dVAR; dXSARGS;
#else
    dXSARGS;
#endif
    if (items < 2 || items > 3)
       croak_xs_usage(cv,  "lat, lon, level = 16");
    {
	NV	lat = (NV)SvNV(ST(0));
	NV	lon = (NV)SvNV(ST(1));
	IV	level;
#line 334 "src/geohex.xs"
        PerlGeoHexZone zone;
#line 420 "src/geohex.c"
	SV *	RETVAL;

	if (items < 3)
	    level = 16;
	else {
	    level = (IV)SvIV(ST(2));
	}
#line 336 "src/geohex.xs"
        get_zone_by_location( &zone, lat, lon, level );
        {
            dSP;
            int count = 0;
            SV *zone_sv;

            ENTER;
            SAVETMPS;
            PUSHMARK(SP);
            mXPUSHp( "Geo::Hex::XS::Zone", 18 );
            mXPUSHp( "lat",  3 );
            mXPUSHn( zone.lat );
            mXPUSHp( "lon",  3 );
            mXPUSHn( zone.lon );
            mXPUSHp( "x",  1 );
            mXPUSHn( zone.x );
            mXPUSHp( "y",  1 );
            mXPUSHn( zone.y );
            mXPUSHp( "code",  4 );
            mXPUSHp( zone.code, strlen(zone.code) );
            PUTBACK;

            count = call_method( "new", G_SCALAR );
            SPAGAIN;

            if (count < 1) {
                croak("Geo::Hex::XS::Zone::new did not return an object");
            }

            zone_sv = newSVsv(POPs);
            PUTBACK;
            FREETMPS;
            LEAVE;

            RETVAL = zone_sv;
        }

#line 466 "src/geohex.c"
	ST(0) = RETVAL;
	sv_2mortal(ST(0));
    }
    XSRETURN(1);
}


XS(XS_Geo__Hex__XS_get_zone_by_xy); /* prototype to pass -Wmissing-prototypes */
XS(XS_Geo__Hex__XS_get_zone_by_xy)
{
#ifdef dVAR
    dVAR; dXSARGS;
#else
    dXSARGS;
#endif
    if (items < 2 || items > 3)
       croak_xs_usage(cv,  "x, y, level = 16");
    {
	NV	x = (NV)SvNV(ST(0));
	NV	y = (NV)SvNV(ST(1));
	IV	level;
#line 382 "src/geohex.xs"
        PerlGeoHexZone zone;
#line 490 "src/geohex.c"
	SV *	RETVAL;

	if (items < 3)
	    level = 16;
	else {
	    level = (IV)SvIV(ST(2));
	}
#line 384 "src/geohex.xs"
        get_zone_by_xy( &zone, x, y, level );
        {
            dSP;
            int count = 0;
            SV *zone_sv;

            ENTER;
            SAVETMPS;
            PUSHMARK(SP);
            mXPUSHp( "Geo::Hex::XS::Zone", 18 );
            mXPUSHp( "lat",  3 );
            mXPUSHn( zone.lat );
            mXPUSHp( "lon",  3 );
            mXPUSHn( zone.lon );
            mXPUSHp( "x",  1 );
            mXPUSHn( zone.x );
            mXPUSHp( "y",  1 );
            mXPUSHn( zone.y );
            mXPUSHp( "code",  4 );
            mXPUSHp( zone.code, strlen(zone.code) );
            PUTBACK;

            count = call_method( "new", G_SCALAR );
            SPAGAIN;

            if (count < 1) {
                croak("Geo::Hex::XS::Zone::new did not return an object");
            }

            zone_sv = newSVsv(POPs);
            PUTBACK;
            FREETMPS;
            LEAVE;

            RETVAL = zone_sv;
        }

#line 536 "src/geohex.c"
	ST(0) = RETVAL;
	sv_2mortal(ST(0));
    }
    XSRETURN(1);
}


XS(XS_Geo__Hex__XS_get_steps); /* prototype to pass -Wmissing-prototypes */
XS(XS_Geo__Hex__XS_get_steps)
{
#ifdef dVAR
    dVAR; dXSARGS;
#else
    dXSARGS;
#endif
    if (items != 4)
       croak_xs_usage(cv,  "start_x, start_y, end_x, end_y");
    {
	NV	start_x = (NV)SvNV(ST(0));
	NV	start_y = (NV)SvNV(ST(1));
	NV	end_x = (NV)SvNV(ST(2));
	NV	end_y = (NV)SvNV(ST(3));
	NV	RETVAL;
	dXSTARG;

	RETVAL = get_steps(start_x, start_y, end_x, end_y);
	XSprePUSH; PUSHn((NV)RETVAL);
    }
    XSRETURN(1);
}


XS(XS_Geo__Hex__XS_encode_geohex); /* prototype to pass -Wmissing-prototypes */
XS(XS_Geo__Hex__XS_encode_geohex)
{
#ifdef dVAR
    dVAR; dXSARGS;
#else
    dXSARGS;
#endif
    if (items < 2 || items > 3)
       croak_xs_usage(cv,  "lat, lon, level = 16");
    {
	NV	lat = (NV)SvNV(ST(0));
	NV	lon = (NV)SvNV(ST(1));
	IV	level;
#line 437 "src/geohex.xs"
        PerlGeoHexZone zone;
#line 585 "src/geohex.c"
	SV *	RETVAL;

	if (items < 3)
	    level = 16;
	else {
	    level = (IV)SvIV(ST(2));
	}
#line 439 "src/geohex.xs"
        get_zone_by_location( &zone, lat, lon, level );
        RETVAL = newSV(0);
        sv_setpv( RETVAL, zone.code );
#line 597 "src/geohex.c"
	ST(0) = RETVAL;
	sv_2mortal(ST(0));
    }
    XSRETURN(1);
}


XS(XS_Geo__Hex__XS_decode_geohex); /* prototype to pass -Wmissing-prototypes */
XS(XS_Geo__Hex__XS_decode_geohex)
{
#ifdef dVAR
    dVAR; dXSARGS;
#else
    dXSARGS;
#endif
    if (items != 1)
       croak_xs_usage(cv,  "code");
    PERL_UNUSED_VAR(ax); /* -Wall */
    SP -= items;
    {
	char *	code = (char *)SvPV_nolen(ST(0));
#line 449 "src/geohex.xs"
        PerlGeoHexZone zone;
        int level;
#line 622 "src/geohex.c"
#line 452 "src/geohex.xs"
        get_zone_by_code( &zone, code );
        level = get_index_of_h_key( *code );

        mXPUSHn( zone.lat );
        mXPUSHn( zone.lon );
        mXPUSHi( level );
        XSRETURN(3);
#line 631 "src/geohex.c"
	PUTBACK;
	return;
    }
}

#ifdef __cplusplus
extern "C"
#endif
XS(boot_Geo__Hex__XS); /* prototype to pass -Wmissing-prototypes */
XS(boot_Geo__Hex__XS)
{
#ifdef dVAR
    dVAR; dXSARGS;
#else
    dXSARGS;
#endif
#if (PERL_REVISION == 5 && PERL_VERSION < 9)
    char* file = __FILE__;
#else
    const char* file = __FILE__;
#endif

    PERL_UNUSED_VAR(cv); /* -W */
    PERL_UNUSED_VAR(items); /* -W */
    XS_VERSION_BOOTCHECK ;

        (void)newXS("Geo::Hex::XS::get_zone_by_code", XS_Geo__Hex__XS_get_zone_by_code, file);
        (void)newXS("Geo::Hex::XS::get_zone_by_location", XS_Geo__Hex__XS_get_zone_by_location, file);
        (void)newXS("Geo::Hex::XS::get_zone_by_xy", XS_Geo__Hex__XS_get_zone_by_xy, file);
        (void)newXS("Geo::Hex::XS::get_steps", XS_Geo__Hex__XS_get_steps, file);
        (void)newXS("Geo::Hex::XS::encode_geohex", XS_Geo__Hex__XS_encode_geohex, file);
        (void)newXS("Geo::Hex::XS::decode_geohex", XS_Geo__Hex__XS_decode_geohex, file);
#if (PERL_REVISION == 5 && PERL_VERSION >= 9)
  if (PL_unitcheckav)
       call_list(PL_scopestack_ix, PL_unitcheckav);
#endif
    XSRETURN_YES;
}

