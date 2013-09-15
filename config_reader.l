%{
#include "config_parser.tab.h"
#include <string.h>
#include <stdlib.h>

int lines = 0;

%}


%%

cue                         return CUE;
ch:                         return CHAN;
flicker                     return FLICKER; 
oscillator                  return OSCILLATOR; 
analyzer                    return ANALYZER; 
timer                       return TIMER; 
speed:                      return SPEED;
low:                        return LOW;
high:                       return HIGH;
file:                       return FILENAME;
type:                       return TYPE;
freq:                       return FREQ;
threshold:                  return THRESHOLD;
threshold_value:            return THRESHOLD_VALUE;
bands:                      return BANDS;
dmx-value:                  return DMX_VALUE;
ontime:                     return ONTIME;
offtime:                    return OFFTIME;
onvalue:                    return ONVALUE;
offvalue:                   return OFFVALUE;
"{"                         return LBRACE;
"}"                         return RBRACE; 
"("                         return LPAREN;
")"                         return RPAREN;

ch[0-9]{1,3}:                {
                                char v[4];
                                strncpy(v, &yytext[2], 3);
                                v[3] = '\0';
                                yylval.val = atoi(v);
                                return CHANNEL;
                            }

[0-9]+                      { 
                                yylval.val = atoi(yytext);
                                return VALUE;
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
                                    yylval.chan_list.channels[i] = (unsigned char)strtol(item, NULL, 10);
                                    yylval.chan_list.count = (unsigned char)++i;
                                }
                                free(cpy);
                                return CHANNEL_LIST;
                            }

"/"[0-9a-zA-Z \/\\\.\-]+    { 
                                yylval.text = strdup(yytext);
                                return FILE_SPEC;
                            }

[;]                        return SEMICOLON; 
[\r\n ]                    lines++;
[\t]                       ;
.                           {
                                printf("Warning: unknown character '%s' (line: %d)\n", yytext, lines);
                                yylval.text = strdup(yytext);
                                return ERROR;                              
                            }
%%

