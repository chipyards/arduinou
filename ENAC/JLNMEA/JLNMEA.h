/* petite lib pour le decodage des sentences NMEA du GPS NEO 8 (et autres)
   optimisee pour petit microcontroleur (8 bits)
   long type is used to force the 8-bit machine to use 32 bits,
   it should have no effect on 32 bits machine
*/

// format pour 1 variable
typedef struct {
long int val;	// raw int value
char	type;	//	'a'	ascii, delim ',' or '*'
		//	'd'	decimal, delim ',' or '*'
		//	'2'	2-byte, no delim
		//	'3'	3-byte, no delim
		//	'f'	floating point,  delim ',' or '*'
		//	0	not to be stored
char	frac;	// number of fractional digits (for 'f')
char	stat;	// 0=ok, 1=undefined, 2=err
char	fill;
} gps_var;

// contexte du parseur

// quantite de variables pour allocation statique
// attention pas plus de 128 variables, l'indice est char, et -1 est reserve
// ATTENTION la fonction gps_var_init() ne doit pas depasser cette limite
#define QVAR 20

typedef struct {
// stockage global
gps_var * data;
char qdata;		// number of data <= QVAR

// job en cours
const char * fields;	// definition de la sentence en cours
char qfields;		// number of fields defined for this sentence
char status;		// 42 = checksum ok
long int fourcc;	// sentence model
char ifield;		// index of the current/next field
char ichar;		// index of current char in field
char ivar;		// index of current variable in data[]
char tfield;		// type of current field
char checksum;		// checksum in progress
long int tmp;		// current element in progress
} nmea_ctx;

// codes FOURCC
#define NGGA ( ('N') | ((long)'G'<<8) | ((long)'G'<<16) | ((long)'A'<<24) )
#define PGGA ( ('P') | ((long)'G'<<8) | ((long)'G'<<16) | ((long)'A'<<24) )
#define NRMC ( ('N') | ((long)'R'<<8) | ((long)'M'<<16) | ((long)'C'<<24) )
#define PRMC ( ('P') | ((long)'R'<<8) | ((long)'M'<<16) | ((long)'C'<<24) )

// initialisation des variables
void gps_var_init( nmea_ctx * ctx );
void new_sentence( nmea_ctx * ctx );
void invalidate_sentence( nmea_ctx * ctx );

// traitement d'un char
void nmea_proc( nmea_ctx * ctx, char c );

/* conversions *
// ref locale : borne IGN de Bonneville 44.6727579, 0.8560689 selon OSM
#define REF_LAT_MIN_5 4036547
#define COEF_LAT_NUM_5 22225		// 111.125 km/deg --> 111125 / 6000000 = 22225 / 1200000
#define COEF_LAT_DEN_5 1200000
#define REF_LON_MIN_5 5136413
#define COEF_LON_NUM_5 15859		// 79.294 km/deg --> 79294 / 6000000 = 15859 / 1200000
#define COEF_LON_DEN_5 1200000

int min2microdeg( int fmin, int frac );
int min2local_lat( int fmin, int frac );
int min2local_lon( int fmin, int frac );
//*/