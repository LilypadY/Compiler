#include<stdio.h>
#include<string.h>
#include<ctype.h>
#include<stdlib.h>
#include"lex.h"
#include"err.h"
#define MAXBUF 1000
char *ResvList[]={"const","int","char","void","main","if","do","while","switch","case","return","scanf","printf"};
char SingleSptListonly[]= {'(',')','+','-','*','/',':',';',',','{','}','[',']'};
char DoubleSptHeadListonly[] = {'!'};
char SingleandDoubleHead[]={'<','>','='};

int isAlpha(char c){
    if(isalpha(c)||c=='_') return 1;
    return 0;
}
/*return index of reserved words if exists else 0(identifier)*/
int isReserved(char token[]){
	int i;
	for(i = 0;i<RES_ZONE_END-RES_ZONE_ST;i ++){
		if(!strcmp(token,ResvList[i])){
			return i+RES_ZONE_ST;
		}
	}
	return 0;
}
int isSeperator(char token){
	int i;
	int l1 = sizeof(SingleSptListonly)/sizeof(char);
	for(i = 0;i<l1;i ++){
		if(token==SingleSptListonly[i]){
			return i+SPT_ZONE_ST;
		}
	}
	int l2 = sizeof(DoubleSptHeadListonly)/sizeof(char);
	char nt = buf[pt++];
	for(i = 0;i < l2;i ++){
        if(token==DoubleSptHeadListonly[i]){
            if(nt!='='){
                error(LEX_ERR,NEQERR);
                return 0;
            }
            else return NEQSY;
        }
	}
	int l3 = sizeof(SingleandDoubleHead)/sizeof(char);
	for(i = 0;i < l3;i ++){
        if(token==SingleandDoubleHead[i]){
            if(nt!='='){
                pt--;
                return i+l1+SPT_ZONE_ST;
            }
            else{
                return i+l1+l2+l3+SPT_ZONE_ST;
            }
        }
	}
	pt--;
	return 0;
}
int isChar(char c){
    if(c=='+'||c=='-'||c=='*'||c=='/'||isAlpha(c)||isdigit(c)){
        return 1;
    }
    return 0;
}
int isStrEle(char c){
    if(c==32||c==33||(c>=35&&c<=126)) return 1;
    return 0;
}
char toLower(char c){
    if(isalpha(c)||isdigit(c)) return tolower(c);
    if(c=='_') return c;
    return -1;
}
int nextsym(){
	char token[MAXBUF];
	char c;
	int tkp = 0;

	int sym = 0;

	while(pt<f_len){
		c = buf[pt++];
		if(c=='\n') lineno ++;
		if(isspace(c)) continue;
		if(isdigit(c)){
            tkp = 0;
            while(c!='\0'&&isdigit(c)){
                token[tkp++] = c;
                c = buf[pt++];
            }
            pt --;
			token[tkp] = '\0';
            if(token[0]=='0'){
                if(tkp>1){
                    error(LEX_ERR,LEADINGZERO);
                    continue;
                }
                else{
                    return ZEROSY;
                }
            }
			tkp = 0;

			intsy_value = atoi(token);//ignore minus-number cases
			return INTVALSY;
		}
		else if(isAlpha(c)){
			do{
				token[tkp++] = toLower(c);
				c = buf[pt++];
			}while((c!='\0')&&(isAlpha(c)||isdigit(c)));
			token[tkp] = '\0';
			tkp = 0;

			pt--;//retract

			sym = isReserved(token);

			if(sym){
				return sym;
			}
			else{
				strcpy(idsy_value,token);
				return IDSY;
			}
		}
		else if(c=='\''){
            c=buf[pt++];
            if(isChar(c)){
                charsy_value = c;
                if(buf[pt++]=='\''){
                    return CHARVALSY;
                }
                else{
                    error(LEX_ERR,QMISMATCH);//missing another '
                    continue;
                }
            }
            else{
                if(buf[pt++]=='\''){
                    error(LEX_ERR,INVALIDCHAR);//invalid character
                    continue;
                }
                else{
                    error(LEX_ERR,NOMORE);
                    pt --;
                    continue;
                }
            }
		}
		else if(c=='\"'){
            int strpt = 0;
            while((c=buf[pt++])!='\0' && c!='\n'){
                if(c=='\"'){
                    strsy_value[strpt]='\0';
                    return STRVALSY;
                }
                if(!isStrEle(c)){
                    error(LEX_ERR,INVALIDSTR);
                    continue;
                }
                if(strpt==MAX_STR_LEN){
                    error(MEMORY_ERR,STRTOOLONG);
                }
                else strsy_value[strpt++] = c;
            }
            error(LEX_ERR,DQMISMATCH);
		}
        else if((sym = isSeperator(c))){
			return sym;
		}
		else{
            error(LEX_ERR,INVALID);
        }
	}
	return sym;
}
