/* petite lib pour le decodage des sentences NMEA du GPS NEO 8 (et autres)
*/

#include "JLNMEA.h"
// #include <stdio.h>

// initialisation des variables
void gps_var_init( nmea_ctx * ctx )
{
int iva;

ctx->data[0].type = '2';	// hour
ctx->data[1].type = '2';	// mn
ctx->data[2].type = 'f';	// sec
ctx->data[3].type = 'a';	// receiver status (A=ok?)
ctx->data[4].type = '2';	// latitude degres
ctx->data[5].type = 'f'; 	// latitude minutes
ctx->data[6].type = 'a'; 	// lat. sign
ctx->data[7].type = '3'; 	// longitude degres
ctx->data[8].type = 'f'; 	// longitude minutes
ctx->data[9].type = 'a'; 	// long. sign
ctx->data[10].type = 'f';	// knots
ctx->data[11].type = 'f';	// track made good degrees
ctx->data[12].type = '2';	// day
ctx->data[13].type = '2';	// month
ctx->data[14].type = 'f';	// year
ctx->data[15].type = 'd';	// GPS qualite (1=good)
ctx->data[16].type = 'd';	// number of satellites
ctx->data[17].type = 'f' ;	// HDOP
ctx->data[18].type = 'f' ;	// VDOP
ctx->data[19].type = 'f';	// Altitude (over sea level)

//ctx->data[].type = '';	//

ctx->qdata = 20;
for	( iva = 0; iva < ctx->qdata; ++iva )
	ctx->data[iva].stat = 1;	// 1 <==> undefined
}

// liste des fields de chaque sentence supportee : un index de variable pour chaque field,
// ou -1 s'il n'est pas destine a etre stocke dans une variable
const char sent_NGGA[] = {
0,	// hour
1,	// mn
2,	// sec
4,	// latitude degres
5,	// latitude minutes
6,	// lat. sign
7,	// longitude degres
8,	// longitude minutes
9,	// long. sign
15,	// GPS qualite (1=good)(2=?)
16,	// number of satellites
17,	// HDOP
19,	// Altitude (over sea level)
};

const char sent_NRMC[] = {
0,	// hour
1,	// mn
2,	// sec
3,	// status
4,	// latitude degres
5,	// latitude minutes
6,	// lat. sign
7,	// longitude degres
8,	// longitude minutes
9,	// long. sign
10,	// knots
11,	// track made good degrees
12,	// day
13,	// month
14,	// year
-1,	// magnetic var
-1,	// magnetic sign 
};


void new_sentence( nmea_ctx * ctx )
{
ctx->qfields = 0;
ctx->ifield = -1;	// le preambule style $GNRMC ne compte pas pour un field
ctx->ichar = 0;
ctx->status = 0;	// 0 = empty, 1 = in progress, 42 = checksum Ok
}

static void next_field( nmea_ctx * ctx )
{
++ctx->ifield;
if	( ctx->ifield < ctx->qfields )
	ctx->ivar = ctx->fields[ctx->ifield];
else	ctx->ivar = -1;				// an extra field
if	( ctx->ivar < ctx->qdata )
	ctx->tfield = ctx->data[ctx->ivar].type;
else	ctx->tfield = 0;	// not to be stored
if	( ctx->tfield == 'f' )
	ctx->data[ctx->ivar].frac = (char)-99;
ctx->ichar = 0;
}

void invalidate_sentence( nmea_ctx * ctx )
{
char ifi, iva;
for	( ifi = 0; ifi < ctx->qfields; ++ifi )
	{
	iva = ctx->fields[ifi];
	if	( iva >= 0 )
		ctx->data[iva].stat = 2;
	}
}

// traitement d'un char
void nmea_proc( nmea_ctx * ctx, char c )
{
if	( ctx->ifield == -1 )	// preambule ou attente preambule
	{
	if	( ctx->ichar == 0 )
		{ if ( c == '$' ) ++ctx->ichar; ctx->checksum = 0; ctx->status = 1; }
	else if ( ctx->ichar == 1 )
		{ if ( c == 'G' ) { ++ctx->ichar; ctx->fourcc = 0; } else ctx->ichar = 0; }
	else if ( ctx->ichar <= 5 )
		{ ctx->fourcc |= (long)c << ( 8 * ( ctx->ichar - 2 ) ); ++ctx->ichar; }
	else if ( ctx->ichar == 6 )
		{
		if	( c == ',' )
			{
			switch	( ctx->fourcc )
				{
				case NRMC :
				case PRMC :	ctx->fields = sent_NRMC;
						ctx->qfields = sizeof(sent_NRMC);
						next_field( ctx ); break;
				case NGGA :
				case PGGA :	ctx->fields = sent_NGGA;
						ctx->qfields = sizeof(sent_NGGA);
						next_field( ctx ); break;
				}
			}
		ctx->ichar = 0;
		}
	if	( c != '$' )
		ctx->checksum ^= c;
	}
else if	( ctx->ifield >= 0 )	// data field
	{
	if	( ( c == ',' ) || ( c == '*' ) )		// normal delimited end
		{
		if	( ctx->tfield )
			{
			if	( ctx->ichar == 0 )
				{ ctx->data[ctx->ivar].stat = 1; }
			else	{ ctx->data[ctx->ivar].stat = 0; ctx->data[ctx->ivar].val = ctx->tmp; } 
			}
		if	( c == '*' )
			{ ctx->ifield = -2; ctx->ichar = 0; }
		else	next_field( ctx );
		}
	else if	(						// normal counted end
			( ( ctx->tfield == '2' ) && ( ctx->ichar == 1 ) ) ||
			( ( ctx->tfield == '3' ) && ( ctx->ichar == 2 ) )
		)
		{
		ctx->tmp = ctx->tmp * 10 + ( c - '0');
		if	( ctx->tfield )
			{ ctx->data[ctx->ivar].stat = 0; ctx->data[ctx->ivar].val = ctx->tmp; } 
		next_field( ctx );
		}
	else if ( c < ' ' )
		{						// premature end of line
		invalidate_sentence( ctx );
		new_sentence( ctx );
		}
	else if	( c == '.' )					// decimal point
		{ 
		if	( ctx->tfield )
			ctx->data[ctx->ivar].frac = 0;
		++ctx->ichar;
		}
	else	{						// normal data digit
		if	( ctx->tfield == 'a' )
			ctx->tmp = c;
		else if	( ctx->ichar == 0 )
			ctx->tmp = c - '0';
		else	ctx->tmp = ctx->tmp * 10 + ( c - '0');
		++ctx->ichar;
		if	( ctx->tfield )
			++ctx->data[ctx->ivar].frac;
		}
	if	( c != '*' )
		ctx->checksum ^= c;
	}
else if	( ctx->ifield == -2 )
	{				// checksum
	c -= '0';
	if ( c > 9 ) c -= ('A' - '9' - 1 );
	if	( ctx->ichar == 0 )
		{ ctx->tmp = c; ++ctx->ichar; }
	else if	( ctx->ichar == 1 )
		{
		ctx->tmp = ( ctx->tmp << 4 ) + c;
		if	( ctx->tmp == ctx->checksum )
			ctx->status = 42;	// normal end of sentence
		}
	}
}

#ifdef STANDALONE
/* ---------- standalone demo : give a sentence as CLI 'argument' ----------- */
// gcc -Wall -o nmea.exe -Wno-char-subscripts -DSTANDALONE nmea.c

#include <string.h>
#include <stdio.h>

int vardump( nmea_ctx * ctx, int ivar )
{
double d;
printf("var %d: ", ivar );
gps_var * v = &ctx->data[ivar];
if	( v->stat == 1 )
	{ printf("undef\n"); return 1; }
else if ( v->stat != 0 )
	{ printf("invalid\n"); return 2; }
printf("'%c' ", v->type );
switch	( v->type )
	{
	case 'a' : printf("%c", (char)v->val ); break;
	case 'd' :
	case '2' :
	case '3' : printf("%d", (int)v->val ); break;
	case 'f' :	d = (double)v->val;
			for	( int p = 0; p < v->frac; p++ )
				d /= 10.0;
			printf("%g", d ); break;
	}
printf("\n");
return 0;
}

int main( int argc, char ** argv )
{
nmea_ctx lectx;		// we allocate the context here, it has a pointer to the variables
#define CTX (&lectx)	// and only scalar variables
gps_var lesvars[QVAR];	// we allocate the variables here
CTX->data = lesvars; 	// then we make the context aware of it
gps_var_init( CTX );	// we initialize the variables we are interested in

if	( argc < 2 )
	return 1;

char * lasentence = argv[1];

// traiter la sentence caractere par caractere
// c'est au programme appelant de detecter le terminateur t.q. CRLF
// et alors de tester le status et appeler new_sentence(CTX);
// Mais ici dans cette demo on utilise strlen
new_sentence(CTX);
for	( unsigned int i = 0; i < strlen(lasentence); i++ )
	{
	// printf("about to process char %d (%c)\n", i, lasentence[i] ); fflush(stdout);
	nmea_proc( CTX, lasentence[i] );
	}
if	( CTX->status == 42 )
	printf("<<checksum ok>>\n");
else	{
	invalidate_sentence( CTX );
 	printf("<<bad or no checksum>>\n");
	}
new_sentence(CTX);	// remet a zero le parseur mais les variables sont persistantes

// a ce point les variables repertoriees pour cette sentence sont a jour
// Si le checksum de la sentence est bon, toutes ont le status 0, sinon elles ont le status 2
// les variables non concernees sont intactes
for	( int i = 0; i < CTX->qdata; i++ )
	{
	vardump( CTX, i ); fflush(stdout);
	}
printf("number of sats : %ld, day %ld\n", CTX->data[16].val, CTX->data[12].val );

return 0;
}
#endif

/*
// conversion des minutes decimales en microdegres
// fmin : representation entiere, frac : nombre de digits apres la virgule
int min2microdeg( int fmin, int frac )
{
if	( frac == 5 )
	return( fmin / 6 );
else if	( frac == 4 )
	return( ( fmin * 10 ) / 6 );
else	return 0;			// erreur
}

// conversion minutes decimales en metres locaux
int min2local_lat( int fmin, int frac )
{
if	( frac == 4 )
	fmin *= 10;	// on se ramene au cas frac = 5
else if	( frac != 5 )
	return 99999;	// erreur
fmin -= REF_LAT_MIN_5;
// on doit multiplier par une fraction de l'ordre de 1852/100000
fmin *= COEF_LAT_NUM_5;
fmin += (COEF_LAT_DEN_5/2);	// arrondi optimal
fmin /= COEF_LAT_DEN_5;
return fmin;
}

int min2local_lon( int fmin, int frac )
{
if	( frac == 4 )
	fmin *= 10;	// on se ramene au cas frac = 5
else if	( frac != 5 )
	return 99999;	// erreur
fmin -= REF_LON_MIN_5;
// on doit multiplier par une fraction de l'ordre de 1852/100000
fmin *= COEF_LON_NUM_5;
fmin += (COEF_LON_DEN_5/2);	// arrondi optimal
fmin /= COEF_LON_DEN_5;
return fmin;
}
*/