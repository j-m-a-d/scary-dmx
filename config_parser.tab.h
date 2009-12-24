#define VALUE 257
#define CHANNEL 258
#define CHANNEL_LIST 259
#define FLOAT_VALUE 260
#define FILE_SPEC 261
#define CUE 262
#define CHAN 263
#define FLICKER 264
#define OSCILLATOR 265
#define ANALYZER 266
#define TIMER 267
#define SPEED 268
#define LOW 269
#define HIGH 270
#define FILENAME 271
#define TYPE 272
#define FREQ 273
#define THRESHOLD 274
#define BANDS 275
#define THRESHOLD_VALUE 276
#define DMX_VALUE 277
#define ONTIME 278
#define OFFTIME 279
#define LPAREN 280
#define RPAREN 281
#define LBRACE 282
#define RBRACE 283
#define SEMICOLON 284
#define DASH 285
#define UNKNOWN 286
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
