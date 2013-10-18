%{
#include "config_parser.tab.h"
#include "utils.h"
#include <string.h>
#include <stdlib.h>

char linebuf[1024];
int lines = 1;
%}


%%

analyzer                    return ANALYZER; 
bands:                      return BANDS;
ch:                         return CHAN;
chaser                      return CHASER;
chase_value:                return CHASE_VALUE;
cue                         return CUE;
fader                       return FADER;
file:                       return FILENAME;
flicker                     return FLICKER; 
freq:                       return FREQ;
from:                       return FROM;
to:                         return TO;
high:                       return HIGH;
low:                        return LOW;
onoff                       return ON_OFF;
offtime:                    return OFFTIME;
offvalue:                   return OFFVALUE;
ontime:                     return ONTIME;
onvalue:                    return ONVALUE;
oscillator                  return OSCILLATOR;
pre_delay:                  return PRE_DELAY;
post_delay:                 return POST_DELAY;
repeat:                     return REPEAT;
speed:                      return SPEED;
step                        return STEP;
threshold:                  return THRESHOLD;
threshold_value:            return THRESHOLD_VALUE;
timer                       return TIMER; 
type:                       return TYPE;
"{"                         return LBRACE;
"}"                         return RBRACE;
"("                         return LPAREN;
")"                         return RPAREN;

ch[0-9]{1,3}:                {
                                char v[4];
                                strncpy(v, &yytext[2], 3);
                                v[3] = '\0';
                                int channel = atoi(v);
                                if( channel < DMX_CHANNELS ) {
                                    yylval.channel = (unsigned int)channel;
                                    return CHANNEL;
                                } else {
                                    yylval.text = strdup("DMX channel out of range");
                                    return ERROR;
                                }
                            }

[0-9]+                      {
                                /* grab endptr on error parsing */
                                long val = strtol(yytext, NULL, 10);
                                if( val > 255) {
                                    yylval.time_val = (unsigned long)val;
                                    return LONGVAL;
                                } else if(val >= 0) {
                                    yylval.val = (unsigned char)val;
                                    return VALUE;
                                }  else {
                                    yylval.text = strdup("value must be positive");
                                    return ERROR;
                                }
                            }

[0-9]+?\.[0-9]+             { 
                                yylval.dval = atof(yytext);
                                return FLOAT_VALUE;
                            }

[0-9]{1,3}(,[0-9]{1,3})+    {
                                size_t length = sizeof(char) * strlen(yytext) + 1;
                                char *cpy = malloc(length);
                                memset(cpy,0,length);
                                strcpy(cpy, yytext);
                                char *item = NULL, *brk;
                                char *tok = ",";
                                int i=0;
                                for(item = strtok_r(cpy, tok, &brk); 
                                    item;
                                    item = strtok_r(NULL, tok, &brk)){
                                    // add error check
                                    yylval.array.channels[i] = (unsigned char)strtol(item, NULL, 10);
                                    yylval.array.count = (unsigned char)++i;
                                }
                                free(cpy);
                                return CHANNEL_LIST;
                            }

"/"[0-9a-zA-Z \/\\\.\-]+    { 
                                yylval.text = strdup(yytext);
                                return FILE_SPEC;
                            }

[;]                        return SEMICOLON;
[\r\n ]                    ;
\n.*                        {
                                strncpy(linebuf, yytext+1, sizeof(linebuf));
                                yyless(1);
                                lines++;
                            }
[\t]                       ;
 "//".*$                   printf("%s\n", yytext);// line comments echo'd
.                           {
                                yylval.text = strdup(yytext);
                                return ERROR;                              
                            }
%%

extern void yyerror(const char *str)
{
    log_error("%s on line %d - unexpected: \"%s\" in source: \"%s\"\n", str, lines, yytext, linebuf);
}

extern int yywrap()
{
    return 1;
}
