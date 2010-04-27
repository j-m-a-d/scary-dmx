#define VALUE 257
#define CHANNEL 258
#define CHANNEL_LIST 259
#define FLOAT_VALUE 260
#define FILE_SPEC 261
#define ERROR 262
#define CUE 263
#define CHAN 264
#define FLICKER 265
#define OSCILLATOR 266
#define ANALYZER 267
#define TIMER 268
#define SPEED 269
#define LOW 270
#define HIGH 271
#define FILENAME 272
#define TYPE 273
#define FREQ 274
#define THRESHOLD 275
#define BANDS 276
#define THRESHOLD_VALUE 277
#define DMX_VALUE 278
#define ONTIME 279
#define OFFTIME 280
#define LPAREN 281
#define RPAREN 282
#define LBRACE 283
#define RBRACE 284
#define SEMICOLON 285
#define DASH 286
#define UNKNOWN 287
typedef union {
    int     val;
    double  dval;
    char    *text;
    struct {
        int count;
        int channels[512];
    } chan_list;
} YYSTYPE;
extern YYSTYPE yylval;
