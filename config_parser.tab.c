#include <sys/cdefs.h>
#ifndef lint
#if 0
static char yysccsid[] = "@(#)yaccpar	1.9 (Berkeley) 02/21/93";
#else
__IDSTRING(yyrcsid, "$NetBSD: skeleton.c,v 1.14 1997/10/20 03:41:16 lukem Exp $");
#endif
#endif
#include <stdlib.h>
#define YYBYACC 1
#define YYMAJOR 1
#define YYMINOR 9
#define YYLEX yylex()
#define YYEMPTY -1
#define yyclearin (yychar=(YYEMPTY))
#define yyerrok (yyerrflag=0)
#define YYRECOVERING (yyerrflag!=0)
#define YYPREFIX "yy"
#line 2 "config_parser.y"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "show_handler.h"
 
dmx_show_t *resultShow;

void yyerror(const char *str)
{
    fprintf(stderr,"error: %s\n",str);
}

int yywrap()
{
    return 1;
} 

int parse_show_file(const char *filename, dmx_show_t **show)
{
    FILE *showFile = fopen(filename, "r");
    if(!showFile){
        return -1;
    }
    /**/
    extern FILE *yyin;
    yyin = showFile;
    /**/
    int i = init_show(&resultShow); 
    if(i) return i;
    /**/
    yyrestart(showFile);
    i = yyparse();
    if(i) return i;
    /**/
    _rewind_show(resultShow);   
    fclose(showFile);
    /**/
    *show = resultShow;
    /**/
    return 0; 
}

#ifdef _EXT_PARSER
int main(int argc, char **argv)
{
    extern FILE *yyin;
    yyin = fopen(argv[1], "r");
    /**/
    int i = init_show(&resultShow);
    if(i){
        fprintf(stderr, "Failed to initialize show.");
    }
    i = yyparse();
    if(i){
        fprintf(stderr, "Parse error.");
        return i;
    }
    _rewind_show(resultShow);    
    /**/
    FILE *outFile = fopen("./outshow.shw", "w+");
    if(!outFile){
        printf("Could not open out file for show output.");
    }
    printShow(resultShow, outFile);
    fclose(outFile);
    /**/
    free_show(resultShow);
    return i;
}
#endif

#ifdef _CLI_MAIN
#include <signal.h>
#include <unistd.h>
#include <QuickTime/QuickTime.h>

void sig_all(int signal)
{
    stop_show();
    free_all_show();
    destroy_dmx();
    ExitMovies();
    fprintf(stderr, "Shutdown finished.\n");
    exit(0);
}
int main(int argc, char **argv)
{
    (void)signal(SIGINT, &sig_all);

    EnterMovies();
        
    dmx_show_t *newShow;
    char *showFile;
    /*showFile = argv[1];*/
    showFile = "/Volumes/HD2/jason/Projects/Scary DMX/halloween.shw";
    if(NULL == showFile){
        fprintf(stderr, "Usage: %s showfile\n", argv[0]);
        return 1;
    }
    if(load_show_from_file(showFile, &newShow )){
        fprintf(stderr, "Show not found or invalid show.\n");
        return 2;
    }

    if(DMX_INIT_OK != init_dmx()){
        fprintf(stderr, "Failed to open DMX device.\n");
        return 3;
    }
    
    start_dmx();
    fprintf(stdout, "Starting show.\n");
    start_show();
    fprintf(stdout, "Running....");
    while(1){
        usleep(100000);
    }
    
    return 0;
}
#endif

#line 125 "config_parser.y"
typedef union {
    int     val;
    double  dval;
    char    *text;
    struct {
        int count;
        int channels[512];
    } chan_list;
} YYSTYPE;
#line 152 "config_parser.tab.c"
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
#define YYERRCODE 256
short yylhs[] = {                                        -1,
    0,   17,   17,   18,   18,   18,   19,   19,   20,   20,
   20,   20,   20,   21,   22,   23,   23,   24,   25,    5,
    9,   10,    6,    7,    8,   14,   11,   12,   13,   15,
   16,    1,    2,    4,    3,   26,
};
short yylen[] = {                                         2,
    1,    1,    2,    3,    4,    7,    1,    2,    1,    1,
    1,    1,    1,    2,    5,   10,   10,    7,    7,    2,
    2,    2,    2,    2,    2,    2,    2,    2,    2,    2,
    2,    2,    2,    3,    3,    1,
};
short yydefred[] = {                                      0,
    0,    0,    0,    2,    0,    0,    3,    0,    0,    0,
    0,    0,    0,    4,    0,    7,    9,   10,   11,   12,
   13,    0,    0,   14,    0,    0,    0,    0,    5,    8,
    0,   32,    0,    0,    0,    0,    0,    0,    0,    0,
   22,    0,    0,    0,    0,    0,    0,    0,    0,    6,
   15,   27,    0,    0,   34,    0,    0,    0,    0,   26,
    0,    0,   28,    0,    0,   35,    0,   23,    0,    0,
    0,   30,    0,    0,   29,   18,   33,   24,    0,    0,
    0,   31,   19,   25,    0,    0,    0,   21,    0,    0,
    0,   20,   16,   17,
};
short yydgoto[] = {                                       2,
   41,   68,   46,   37,   90,   58,   70,   80,   86,   35,
   43,   54,   65,   49,   62,   74,    3,    4,   15,   16,
   17,   18,   19,   20,   21,    0,
};
short yysindex[] = {                                   -260,
 -277,    0, -260,    0, -239, -257,    0, -262, -236, -261,
 -258, -254, -251,    0, -253,    0,    0,    0,    0,    0,
    0, -249, -255,    0, -241, -223, -229, -223,    0,    0,
 -218,    0, -236, -236, -228, -216, -213, -232, -230, -231,
    0, -236, -215, -227, -240, -214, -214, -236, -222,    0,
    0,    0, -236, -210,    0, -220, -198, -211, -211,    0,
 -236, -217,    0, -236, -212,    0, -208,    0, -236, -209,
 -209,    0, -236, -206,    0,    0,    0,    0, -236, -205,
 -205,    0,    0,    0, -236, -202, -202,    0, -236, -203,
 -201,    0,    0,    0,
};
short yyrindex[] = {                                      0,
    0,    0,   68,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,
};
short yygindex[] = {                                      0,
   -9,    0,    0,    0,  -14,   27,   16,    8,    1,  -21,
    0,    0,    0,    0,    0,    0,    0,   81,   54,  -13,
    0,    0,    0,    0,    0,    0,
};
#define YYTABLESIZE 85
short yytable[] = {                                      24,
    9,   30,    1,    5,    9,    6,   38,   10,   11,   12,
   13,   10,   11,   12,   13,   47,   23,    8,   56,   22,
   23,   25,   33,   40,   26,   30,   14,    9,   27,   32,
   29,   28,   52,   31,   10,   11,   12,   13,   60,    9,
   34,   42,   36,   63,   44,   48,   10,   11,   12,   13,
   45,   72,   51,   50,   75,   53,   61,   55,   64,   78,
   57,   67,   73,   82,   66,   69,   79,    1,   85,   84,
   89,   76,   91,   59,   71,   88,   77,   83,   81,   92,
   93,   87,   94,    7,   39,
};
short yycheck[] = {                                       9,
  258,   15,  263,  281,  258,  283,   28,  265,  266,  267,
  268,  265,  266,  267,  268,   37,  257,  257,  259,  282,
  257,  283,  264,   33,  283,   39,  284,  258,  283,  285,
  284,  283,   42,  283,  265,  266,  267,  268,   48,  258,
  264,  270,  272,   53,  261,  278,  265,  266,  267,  268,
  264,   61,  284,  284,   64,  271,  279,  285,  269,   69,
  275,  260,  280,   73,  285,  277,  276,    0,  274,   79,
  273,  284,   87,   47,   59,   85,  285,  284,   71,   89,
  284,   81,  284,    3,   31,
};
#define YYFINAL 2
#ifndef YYDEBUG
#define YYDEBUG 0
#endif
#define YYMAXTOKEN 287
#if YYDEBUG
char *yyname[] = {
"end-of-file",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,"VALUE","CHANNEL","CHANNEL_LIST",
"FLOAT_VALUE","FILE_SPEC","ERROR","CUE","CHAN","FLICKER","OSCILLATOR",
"ANALYZER","TIMER","SPEED","LOW","HIGH","FILENAME","TYPE","FREQ","THRESHOLD",
"BANDS","THRESHOLD_VALUE","DMX_VALUE","ONTIME","OFFTIME","LPAREN","RPAREN",
"LBRACE","RBRACE","SEMICOLON","DASH","UNKNOWN",
};
char *yyrule[] = {
"$accept : show",
"show : cues",
"cues : cue",
"cues : cues cue",
"cue : CUE LBRACE RBRACE",
"cue : CUE LBRACE settings RBRACE",
"cue : CUE LPAREN VALUE RPAREN LBRACE settings RBRACE",
"settings : setting",
"settings : settings setting",
"setting : channel_setting",
"setting : flicker_setting",
"setting : analyzer_setting",
"setting : oscillator_setting",
"setting : timer_setting",
"channel_setting : CHANNEL value",
"flicker_setting : FLICKER LBRACE CHAN value RBRACE",
"analyzer_setting : ANALYZER LBRACE file_spec channel_list threshold threshold_value bands freq analyzer_type RBRACE",
"analyzer_setting : ANALYZER LBRACE file_spec chan threshold threshold_value bands freq analyzer_type RBRACE",
"oscillator_setting : OSCILLATOR LBRACE chan low_value high_value speed_value RBRACE",
"timer_setting : TIMER LBRACE chan dmx_value ontime_value offtime_value RBRACE",
"analyzer_type : TYPE value",
"freq : FREQ value",
"chan : CHAN value",
"threshold : THRESHOLD float_value",
"threshold_value : THRESHOLD_VALUE value",
"bands : BANDS value",
"dmx_value : DMX_VALUE value",
"low_value : LOW value",
"high_value : HIGH value",
"speed_value : SPEED value",
"ontime_value : ONTIME value",
"offtime_value : OFFTIME value",
"value : VALUE SEMICOLON",
"float_value : FLOAT_VALUE SEMICOLON",
"file_spec : FILENAME FILE_SPEC SEMICOLON",
"channel_list : CHAN CHANNEL_LIST SEMICOLON",
"err : ERROR",
};
#endif
#ifdef YYSTACKSIZE
#undef YYMAXDEPTH
#define YYMAXDEPTH YYSTACKSIZE
#else
#ifdef YYMAXDEPTH
#define YYSTACKSIZE YYMAXDEPTH
#else
#define YYSTACKSIZE 10000
#define YYMAXDEPTH 10000
#endif
#endif
#define YYINITSTACKSIZE 200
int yydebug;
int yynerrs;
int yyerrflag;
int yychar;
short *yyssp;
YYSTYPE *yyvsp;
YYSTYPE yyval;
YYSTYPE yylval;
short *yyss;
short *yysslim;
YYSTYPE *yyvs;
int yystacksize;
#line 443 "config_parser.y"

#line 351 "config_parser.tab.c"
/* allocate initial stack or double stack size, up to YYMAXDEPTH */
int yyparse __P((void));
static int yygrowstack __P((void));
static int yygrowstack()
{
    int newsize, i;
    short *newss;
    YYSTYPE *newvs;

    if ((newsize = yystacksize) == 0)
        newsize = YYINITSTACKSIZE;
    else if (newsize >= YYMAXDEPTH)
        return -1;
    else if ((newsize *= 2) > YYMAXDEPTH)
        newsize = YYMAXDEPTH;
    i = yyssp - yyss;
    if ((newss = (short *)realloc(yyss, newsize * sizeof *newss)) == NULL)
        return -1;
    yyss = newss;
    yyssp = newss + i;
    if ((newvs = (YYSTYPE *)realloc(yyvs, newsize * sizeof *newvs)) == NULL)
        return -1;
    yyvs = newvs;
    yyvsp = newvs + i;
    yystacksize = newsize;
    yysslim = yyss + newsize - 1;
    return 0;
}

#define YYABORT goto yyabort
#define YYREJECT goto yyabort
#define YYACCEPT goto yyaccept
#define YYERROR goto yyerrlab
int
yyparse()
{
    int yym, yyn, yystate;
#if YYDEBUG
    char *yys;

    if ((yys = getenv("YYDEBUG")) != NULL)
    {
        yyn = *yys;
        if (yyn >= '0' && yyn <= '9')
            yydebug = yyn - '0';
    }
#endif

    yynerrs = 0;
    yyerrflag = 0;
    yychar = (-1);

    if (yyss == NULL && yygrowstack()) goto yyoverflow;
    yyssp = yyss;
    yyvsp = yyvs;
    *yyssp = yystate = 0;

yyloop:
    if ((yyn = yydefred[yystate]) != 0) goto yyreduce;
    if (yychar < 0)
    {
        if ((yychar = yylex()) < 0) yychar = 0;
#if YYDEBUG
        if (yydebug)
        {
            yys = 0;
            if (yychar <= YYMAXTOKEN) yys = yyname[yychar];
            if (!yys) yys = "illegal-symbol";
            printf("%sdebug: state %d, reading %d (%s)\n",
                    YYPREFIX, yystate, yychar, yys);
        }
#endif
    }
    if ((yyn = yysindex[yystate]) && (yyn += yychar) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yychar)
    {
#if YYDEBUG
        if (yydebug)
            printf("%sdebug: state %d, shifting to state %d\n",
                    YYPREFIX, yystate, yytable[yyn]);
#endif
        if (yyssp >= yysslim && yygrowstack())
        {
            goto yyoverflow;
        }
        *++yyssp = yystate = yytable[yyn];
        *++yyvsp = yylval;
        yychar = (-1);
        if (yyerrflag > 0)  --yyerrflag;
        goto yyloop;
    }
    if ((yyn = yyrindex[yystate]) && (yyn += yychar) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yychar)
    {
        yyn = yytable[yyn];
        goto yyreduce;
    }
    if (yyerrflag) goto yyinrecovery;
    goto yynewerror;
yynewerror:
    yyerror("syntax error");
    goto yyerrlab;
yyerrlab:
    ++yynerrs;
yyinrecovery:
    if (yyerrflag < 3)
    {
        yyerrflag = 3;
        for (;;)
        {
            if ((yyn = yysindex[*yyssp]) && (yyn += YYERRCODE) >= 0 &&
                    yyn <= YYTABLESIZE && yycheck[yyn] == YYERRCODE)
            {
#if YYDEBUG
                if (yydebug)
                    printf("%sdebug: state %d, error recovery shifting\
 to state %d\n", YYPREFIX, *yyssp, yytable[yyn]);
#endif
                if (yyssp >= yysslim && yygrowstack())
                {
                    goto yyoverflow;
                }
                *++yyssp = yystate = yytable[yyn];
                *++yyvsp = yylval;
                goto yyloop;
            }
            else
            {
#if YYDEBUG
                if (yydebug)
                    printf("%sdebug: error recovery discarding state %d\n",
                            YYPREFIX, *yyssp);
#endif
                if (yyssp <= yyss) goto yyabort;
                --yyssp;
                --yyvsp;
            }
        }
    }
    else
    {
        if (yychar == 0) goto yyabort;
#if YYDEBUG
        if (yydebug)
        {
            yys = 0;
            if (yychar <= YYMAXTOKEN) yys = yyname[yychar];
            if (!yys) yys = "illegal-symbol";
            printf("%sdebug: state %d, error recovery discards token %d (%s)\n",
                    YYPREFIX, yystate, yychar, yys);
        }
#endif
        yychar = (-1);
        goto yyloop;
    }
yyreduce:
#if YYDEBUG
    if (yydebug)
        printf("%sdebug: state %d, reducing by rule %d (%s)\n",
                YYPREFIX, yystate, yyn, yyrule[yyn]);
#endif
    yym = yylen[yyn];
    yyval = yyvsp[1-yym];
    switch (yyn)
    {
case 1:
#line 170 "config_parser.y"
{
#ifdef _TRACE_PARSER
    printf("Show def.\n");
#endif
}
break;
case 4:
#line 185 "config_parser.y"
{
#ifdef _TRACE_PARSER
    printf("Empty Cue def.\n");
#endif
}
break;
case 5:
#line 192 "config_parser.y"
{
#ifdef _TRACE_PARSER
    printf("Cue def.\n");
#endif
/**/
    set_step_duration_for_current_cue(resultShow, 0);
    add_cue(resultShow);
}
break;
case 6:
#line 202 "config_parser.y"
{
#ifdef _TRACE_PARSER
    printf("Cue def with duration time: %d\n", yyvsp[-4].val);
#endif
/**/
    set_step_duration_for_current_cue(resultShow, yyvsp[-4].val);
    add_cue(resultShow);
}
break;
case 14:
#line 232 "config_parser.y"
{
#ifdef _TRACE_PARSER
    printf("set channel %d : %d\n", yyvsp[-1].val, yyvsp[0].val);
#endif
    set_channel_value_for_current_cue(resultShow, yyvsp[-1].val, yyvsp[0].val);
}
break;
case 15:
#line 242 "config_parser.y"
{
#ifdef _TRACE_PARSER
    printf("flicker setting: %d\n", yyvsp[-1].val);
#endif
    set_flicker_channel_for_current_cue(resultShow, yyvsp[-1].val);
}
break;
case 16:
#line 253 "config_parser.y"
{
#ifdef _TRACE_PARSER
    printf("analyzer setting: %s ,", yyvsp[-7].text);
    printf("channel value: ");
    printf(" %d channel(s)", yyvsp[-6].chan_list.count);
    int i=0;
    for(i=0; i<yyvsp[-6].chan_list.count; i++){
        printf(" %d, ", yyvsp[-6].chan_list.channels[i]);
    }
    printf("freq value: %d ,", yyvsp[-2].val);
    printf("threshold: %5.3f ,", yyvsp[-5].dval);
    printf("threshold value: %d ,", yyvsp[-4].val);
    printf("bands: %d\n", yyvsp[-3].val);
#endif
    analyzer_data_t *aData = NEW_ANALYZER_DATA_T(aData);
    aData->movieFile = yyvsp[-7].text;
    aData->dmxChannelList = COPY_CHANNEL_LIST(aData->dmxChannelList, yyvsp[-6].chan_list.channels, yyvsp[-6].chan_list.count);
    aData->threshold = yyvsp[-5].dval;
    aData->dmxValue = yyvsp[-4].val;
    aData->numberOfBandLevels = yyvsp[-3].val;
    aData->frequency = yyvsp[-2].val;
    aData->flags = yyvsp[-1].val;
    set_timer_data_for_current_cue(resultShow, aData);
}
break;
case 17:
#line 280 "config_parser.y"
{
#ifdef _TRACE_PARSER
    printf("analyzer setting: %s ,", yyvsp[-7].text);
    printf("channel value: %d ,", yyvsp[-6].val);
    printf("freq value: %d ,", yyvsp[-2].val);
    printf("threshold: %5.3f ,", yyvsp[-5].dval);
    printf("threshold value: %d ,", yyvsp[-4].val);
    printf("bands: %d\n", yyvsp[-3].val);
#endif
    analyzer_data_t *aData = NEW_ANALYZER_DATA_T(aData);
    aData->movieFile = yyvsp[-7].text;
    int length = sizeof(int) * ( 1 + 1);
    aData->dmxChannelList = malloc(length);
    aData->dmxChannelList[1] = 0;
    aData->dmxChannelList[0] = yyvsp[-6].val;
    aData->threshold = yyvsp[-5].dval;
    aData->dmxValue = yyvsp[-4].val;
    aData->numberOfBandLevels = yyvsp[-3].val;
    aData->frequency = yyvsp[-2].val;
    aData->flags = yyvsp[-1].val;
    set_timer_data_for_current_cue(resultShow, aData);
}
break;
case 18:
#line 306 "config_parser.y"
{
#ifdef _TRACE_PARSER
    printf("Oscillator setting: ch-- %d, low-- %d, high-- %d, speed-- %d\n", yyvsp[-4].val, yyvsp[-3].val, yyvsp[-2].val, yyvsp[-1].val );
#endif
    oscillator_data_t* oData = NEW_OSCILLATOR_DATA_T(oData);
    oData->channel = yyvsp[-4].val;
    oData->lowThreshold = yyvsp[-3].val;
    oData->highThreshold = yyvsp[-2].val;
    oData->speed = yyvsp[-1].val;
    set_oscillator_data_for_current_cue(resultShow, oData);
}
break;
case 19:
#line 321 "config_parser.y"
{
#ifdef _TRACE_PARSER
    printf("Timer setting channel-- %d, on-- %d, off-- %d\n",yyvsp[-4].val, yyvsp[-2].val, yyvsp[-1].val);
#endif
    timed_effect_data_t* timer = malloc(sizeof(timed_effect_data_t));
    memset(timer, 0, sizeof(timed_effect_data_t));
    timer->channel = yyvsp[-4].val;
    timer->value = yyvsp[-3].val;
    timer->on_time = yyvsp[-2].val;
    timer->off_time = yyvsp[-1].val;
    timer->timer_handle = 0;
    setTimerDataForCurrentCue(resultShow, timer);
}
break;
case 20:
#line 338 "config_parser.y"
{
    yyval.val = yyvsp[0].val;
}
break;
case 21:
#line 345 "config_parser.y"
{
    yyval.val = yyvsp[0].val;
}
break;
case 22:
#line 351 "config_parser.y"
{
    yyval.val = yyvsp[0].val;
}
break;
case 23:
#line 357 "config_parser.y"
{
    yyval.dval = yyvsp[0].dval;
}
break;
case 24:
#line 363 "config_parser.y"
{
    yyval.val = yyvsp[0].val;
}
break;
case 25:
#line 369 "config_parser.y"
{
    yyval.val = yyvsp[0].val;
}
break;
case 26:
#line 375 "config_parser.y"
{
    yyval.val = yyvsp[0].val;
}
break;
case 27:
#line 381 "config_parser.y"
{
    yyval.val = yyvsp[0].val;
}
break;
case 28:
#line 387 "config_parser.y"
{
    yyval.val = yyvsp[0].val;
}
break;
case 29:
#line 393 "config_parser.y"
{
    yyval.val = yyvsp[0].val;
}
break;
case 30:
#line 399 "config_parser.y"
{
    yyval.val = yyvsp[0].val;
}
break;
case 31:
#line 405 "config_parser.y"
{
    yyval.val = yyvsp[0].val;
}
break;
case 32:
#line 411 "config_parser.y"
{
    yyval.val = yyvsp[-1].val;
}
break;
case 33:
#line 417 "config_parser.y"
{   
    yyval.dval = yyvsp[-1].dval;
}
break;
case 34:
#line 423 "config_parser.y"
{
    yyval.text = yyvsp[-1].text;
}
break;
case 35:
#line 429 "config_parser.y"
{
    yyval.chan_list = yyvsp[-1].chan_list;
}
break;
case 36:
#line 435 "config_parser.y"
{
    fprintf(stderr, "%s not recognized.\n", yyvsp[0].text);
    free(yyvsp[0].text);
    /*free_show(resultShow);    */
}
break;
#line 759 "config_parser.tab.c"
    }
    yyssp -= yym;
    yystate = *yyssp;
    yyvsp -= yym;
    yym = yylhs[yyn];
    if (yystate == 0 && yym == 0)
    {
#if YYDEBUG
        if (yydebug)
            printf("%sdebug: after reduction, shifting from state 0 to\
 state %d\n", YYPREFIX, YYFINAL);
#endif
        yystate = YYFINAL;
        *++yyssp = YYFINAL;
        *++yyvsp = yyval;
        if (yychar < 0)
        {
            if ((yychar = yylex()) < 0) yychar = 0;
#if YYDEBUG
            if (yydebug)
            {
                yys = 0;
                if (yychar <= YYMAXTOKEN) yys = yyname[yychar];
                if (!yys) yys = "illegal-symbol";
                printf("%sdebug: state %d, reading %d (%s)\n",
                        YYPREFIX, YYFINAL, yychar, yys);
            }
#endif
        }
        if (yychar == 0) goto yyaccept;
        goto yyloop;
    }
    if ((yyn = yygindex[yym]) && (yyn += yystate) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yystate)
        yystate = yytable[yyn];
    else
        yystate = yydgoto[yym];
#if YYDEBUG
    if (yydebug)
        printf("%sdebug: after reduction, shifting from state %d \
to state %d\n", YYPREFIX, *yyssp, yystate);
#endif
    if (yyssp >= yysslim && yygrowstack())
    {
        goto yyoverflow;
    }
    *++yyssp = yystate;
    *++yyvsp = yyval;
    goto yyloop;
yyoverflow:
    yyerror("yacc stack overflow");
yyabort:
    return (1);
yyaccept:
    return (0);
}
