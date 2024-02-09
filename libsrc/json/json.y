// based on vectree.y ...
%{
#include "quip_config.h"

#include <stdio.h>
#include <ctype.h>
#include <string.h>

// NOW USE built-in %name-prefix=   - see below

#include "json.h"
#include "quip_prot.h"
#include "query_stack.h"

//#define QLEVEL	qs_level(SINGLE_QSP_ARG)

#define YY_LLEN 1024

static const char *json_cp;
#define YY_CP	json_cp
#define SET_YY_CP(v)	json_cp = v
static int json_qlevel;
static int last_line_num;
#define LAST_LINE_NUM last_line_num
#define SET_LAST_LINE_NUM(v) last_line_num = v
static String_Buf *last_line=NULL;
#define YY_LAST_LINE	last_line
static String_Buf *input_line=NULL;
#define YY_INPUT_LINE	input_line
static int parser_line_num;
#define SET_PARSER_LINE_NUM(v)	parser_line_num = v

#define MAX_STRINGS	128
static const char *string_tbl[MAX_STRINGS];
static int curr_string_idx = 0;
static String_Buf *json_exp_strs[MAX_STRINGS];
#define JSON_EXP_STR json_exp_strs[curr_string_idx]
#define JSON_CURR_STRING	string_tbl[curr_string_idx]
#define ADVANCE_STRING					\
	{						\
		curr_string_idx++;			\
		if( curr_string_idx >= MAX_STRINGS ){	\
			curr_string_idx = 0;		\
		}					\
	}

#define SET_JSON_CURR_STRING(v)	JSON_CURR_STRING = v

static JSON_Obj *parsed_json_obj = NULL;
static Query_Stack *parser_qsp;


void yyerror(Query_Stack *qsp,  char *);

/* what yylval can be */

typedef union {
	JSON_Obj *jt_yy_obj;
	JSON_Thing *jt_yy_thing;
	double jt_yy_number;
	const char *jt_yy_string;
	char jt_yy_char;
} YYSTYPE;

static int name_token(QSP_ARG_DECL  YYSTYPE *yylvp);

#define YYSTYPE_IS_DECLARED			/* necessary on a 2.6 machine?? */

int yylex(YYSTYPE *yylvp, Query_Stack *qsp);

// prototypes
//static JSON_Obj *make_object
static JSON_Obj *make_pair(const char *key,JSON_Thing *thing_p);
static JSON_Obj *make_pair_list(JSON_Obj *list,JSON_Obj *pair);
static JSON_Thing *make_string_thing(const char *s);
static JSON_Thing *make_number_thing(double n);
static JSON_Thing *make_object_thing(JSON_Obj *obj);
static JSON_Obj *make_thing_array(JSON_Obj *list, JSON_Thing *thing_p);
static JSON_Thing *make_obj_id(const char *s);
static JSON_Thing *make_null_thing();

// Can't define this to ERROR_STRING, because ERROR_STRING gets overwritten in yyerror...
#define YY_ERR_STR	JSON_PARSER_ERROR_STRING

%}

%pure-parser	/* updated syntax - make the parser rentrant (thread-safe) */

// equal sign generates a warning w/ bison 3.0!?
%name-prefix="json_"

// parse-param also affects yyerror!

%parse-param{ Query_Stack *qsp }
%lex-param{ Query_Stack *qsp }


%token <jt_yy_number> NUMBER
%token <jt_yy_string> LEX_STRING
%token <jt_yy_obj> OBJECT
%token <jt_yy_obj> ARRAY
%token <jt_yy_name> NAME
%token <jt_yy_expr> EXPR
%token <jt_yy_string> OBJECT_ID
%token <jt_yy_string> NULL_THING

%type <jt_yy_obj> pair_list
%type <jt_yy_obj> pair
%type <jt_yy_obj> object
%type <jt_yy_thing> object_id
%type <jt_yy_obj> top_object
%type <jt_yy_obj> array
%type <jt_yy_obj> thing_list
%type <jt_yy_string> string
%type <jt_yy_number> number
%type <jt_yy_thing> thing

/* keyword tokens */

%start top_object


%%

top_object	: object
			{
			$$ = parsed_json_obj = $1;
dump_json_obj(parsed_json_obj);
			}
		;

object		: '{' pair_list '}'
			{
				$$ = $2;
dump_json_obj($$);
//dump_obj($$);
			}
		| '{' '}'
			{
				$$ = make_pair_list(NULL,NULL);
			}
		| array
			{
				$$ = $1;
			}
		;

pair_list	: pair
			{
			$$ = $1;
dump_json_obj($$);
			}
		| pair_list ',' pair
			{
			$$ = make_pair_list($1,$3);
			}
		;

pair		: string ':' thing
			{
			$$=make_pair($1,$3);
			}
		;

object_id:	OBJECT_ID '(' string ')'
			{
			$$ = make_obj_id($3);
			}
		;

thing		: string
			{
			$$ = make_string_thing($1);
			}
		| number
			{
			$$ = make_number_thing($1);
			}
		| object
			{
			$$ = make_object_thing($1);
			}
		| object_id
			{
			$$ = $1;
			}
		| NULL_THING
			{
			$$ = make_null_thing();
			}
		;

string		: LEX_STRING
			{
			$$ = $1;
			}
		;

number		: NUMBER
			{
			$$ = $1;
			}
		;

array		: '[' thing_list ']'
			{
			$$ = $2;
			}
		;

thing_list	: thing
			{
			$$ = make_thing_array(NULL,$1);
			}
		| thing_list ',' thing
			{
			$$ = make_thing_array($1,$3);
			}
		;



%%

/* table of keywords */

#define parse_num(strptr) _parse_num(QSP_ARG  strptr)

static double _parse_num(QSP_ARG_DECL  const char **strptr)
{
	const char *ptr;
	double place, value=0.0;
	unsigned long intval=0;
	int c;
	int radix;

	int decimal_seen=0;

	ptr=(*strptr);

	radix=10;
	if( *ptr=='0' ){
		ptr++;
		if( *ptr == 'x' ){
			radix=16;
			ptr++;
		} else if( isdigit( *ptr ) ){
			radix=8;
		}
	}
	switch(radix){
		case 10:
			while( isdigit(*ptr) ){
				value*=10;
				value+=(*ptr) - '0';
				ptr++;
			}
			break;
		case 8:
			while( isdigit(*ptr)
				&& *ptr!='8'
				&& *ptr!='9' ){

				intval<<=3;
				intval+=(*ptr) - '0';
				ptr++;
			}
			value=intval;
			break;
		case 16:
			while( isdigit(*ptr)
				|| (*ptr>='a'
					&& *ptr<='f')
				|| (*ptr>='A'
					&& *ptr<='F' ) ){

				/* value*=16; */
				intval <<= 4;
				if( isdigit(*ptr) )
					intval+=(*ptr)-'0';
				else if( isupper(*ptr) )
					intval+= 10 + (*ptr)-'A';
				else intval+= 10 + (*ptr)-'a';
				ptr++;
			}
			value=intval;
			break;
		default:
			yyerror(THIS_QSP,  (char *)"bizarre radix in intscan");
	}
	if( radix==10 && *ptr == '.' ){
		decimal_seen=1;
		ptr++;
		place =1.0;
		while( isdigit(*ptr) ){
			c=(*ptr++);
			place /= 10.0;
			value += (c-'0') * place;
		}
	}
	if( radix == 10 && *ptr == 'e' ){	/* read an exponent */
		int ponent=0;
		int sign;

		decimal_seen=1;
		ptr++;

		if( *ptr == '-' ){
			sign= -1;
			ptr++;
		} else if( *ptr == '+' ){
			sign = 1;
			ptr++;
		} else sign=1;

		if( !isdigit(*ptr) )
			yyerror(THIS_QSP,  (char *)"no digits for exponent!?");
		while( *ptr && 
			isdigit( *ptr ) ){
			ponent *= 10;
			ponent += *ptr - '0';
			ptr++;
		}
		if( sign > 0 ){
			while( ponent-- )
				value *= 10;
		} else {
			while( ponent-- )
				value /= 10;
		}
	}
	*strptr = ptr;
	return(value);
}


/* Read text up to a matching quote, and update the pointer */

#define match_quote(spp) _match_quote(QSP_ARG  spp)

static const char *_match_quote(QSP_ARG_DECL  const char **spp)
{
	String_Buf *sbp;
	int c;

	sbp=JSON_EXP_STR;
	if( sbp == NULL ){
		sbp = JSON_EXP_STR = new_stringbuf();
	}
	copy_string(sbp,"");

	while( (c=(**spp)) && c!='"' ){
		//*s++ = (char) c;
		cat_string_n(sbp,*spp,1);
		(*spp)++; /* YY_CP++; */
	}
	if( c != '"' ) {
		warn("missing quote");
		snprintf(ERROR_STRING,LLEN,"string \"%s\" stored",JSON_CURR_STRING);
		advise(ERROR_STRING);
	} else (*spp)++;			/* skip over closing quote */

	ADVANCE_STRING
	SET_JSON_CURR_STRING( sb_buffer(sbp) );

	return(JSON_CURR_STRING);
} // match_quote

/* remember the name of the current input file if we don't already have one */

static void read_next_fragment(SINGLE_QSP_ARG_DECL)
{
	String_Buf *sbp;
	sbp = new_stringbuf();
	// BUG - memory leak!?

	// we copy this now, because script functions may damage
	// the contents of nameof's buffer.
	copy_string(sbp,NAMEOF("JSON fragment"));
	YY_CP = sb_buffer(sbp);
}

/* #define islegalkeywordchar(c)	(isalpha(c) || (c)=='.' || (c) == '_' )  */
#define islegalkeywordchar(c)	(isalpha(c) || (c) == '_' )

#define read_word(spp)	_read_word(QSP_ARG  spp);

static const char *_read_word(QSP_ARG_DECL  const char **spp)
{
	//char *s;
	String_Buf *sbp;
	int c;

	if( JSON_EXP_STR == NULL ){
		JSON_EXP_STR = new_stringbuf();
	}
	sbp = JSON_EXP_STR;
	assert(sbp!=NULL);
	copy_string(sbp,"");	// clear it
				// does anyone expect this value to persist???  BUG?

	while( islegalkeywordchar(c=(**spp)) || isdigit(c) ){
		cat_string_n(sbp,*spp,1);
		(*spp)++; /* YY_CP++; */
	}

	// BUG?  do we need to allocate a new buffer?
	//return(save_parse_string(VEXP_STR));
	return(sb_buffer(JSON_EXP_STR));
}

static int name_token(QSP_ARG_DECL  YYSTYPE *yylvp)
{
	const char *s;
	const char *sptr;

	sptr=YY_CP;
	s=read_word(&sptr);
	assert(s!=NULL);
	SET_YY_CP(sptr);

	SET_JSON_CURR_STRING(s);

fprintf(stderr,"name_token:  checking \"%s\"\n",JSON_CURR_STRING);
	if( ! strcmp(JSON_CURR_STRING,"ObjectId") ){
		return OBJECT_ID;
	}
	if( ! strcmp(JSON_CURR_STRING,"null") ){
		return NULL_THING;
	}
	return(-1);
} // name_token

int yylex(YYSTYPE *yylvp, Query_Stack *qsp)	/* return the next token */
{
	register int c;
	int in_comment=0;


nexttok:

	if( *YY_CP == 0 ){	/* nothing in the buffer? */
		int ql;
		int l;

		// BUG since qword doesn't tell us about line breaks,
		// it is hard to know when to zero the line buffer.
		// We can try to handle this by watcing q_lineno.


		lookahead_til(json_qlevel-1);
		if( json_qlevel > (ql=QLEVEL) ){
			return(0);	/* EOF */
		}

		/* why disable stripping quotes? */
		disable_stripping_quotes(SINGLE_QSP_ARG);

		read_next_fragment(SINGLE_QSP_ARG);

		/* BUG?  lookahead advances lineno?? */
		/* BUG no line numbers in macros? */
		// Should we compare lineno or rdlineno???
		if( (l=current_line_number(SINGLE_QSP_ARG)) != LAST_LINE_NUM ){
			assert( YY_LAST_LINE != NULL );
			assert( YY_INPUT_LINE != NULL );
			copy_strbuf(YY_LAST_LINE,YY_INPUT_LINE);
			copy_string(YY_INPUT_LINE,"");	// clear input
			SET_PARSER_LINE_NUM( l );
			SET_LAST_LINE_NUM( l );
		}

		cat_string(YY_INPUT_LINE,YY_CP);
		cat_string(YY_INPUT_LINE," ");
	}

#ifdef QUIP_DEBUG
if( debug ){
snprintf(ERROR_STRING,LLEN,"yylex scanning \"%s\"",YY_CP);
advise(ERROR_STRING);
}
#endif /* QUIP_DEBUG */
	/* skip spaces */
	while( *YY_CP && isspace(*YY_CP) ){
	// Not flagging newlines will screw up line numbering!?
	/*
		if( *YY_CP == '\n' ){
			YY_CP++;
#ifdef QUIP_DEBUG
if( debug ){ advise("yylex returning NEWLINE"); }
#endif // QUIP_DEBUG
			return(NEWLINE);
		}
		*/
		YY_CP++;
	}

	c=(*YY_CP);

	/* check for C comment */

	if( in_comment ){
		if( c=='*' && *(YY_CP+1)=='/' ){
			in_comment=0;
			YY_CP+=2;
			goto nexttok;
		} else if( c=='/' && *(YY_CP+1)=='*' ){
			/* BUG print the line number here */
			warn("comment within a comment!?");
			YY_CP+=2;
			goto nexttok;
		}
		YY_CP++;
		goto nexttok;
	}

	if( (! in_comment) && c=='/' && *(YY_CP+1)=='*' ){
		in_comment=1;
		YY_CP+=2;
		goto nexttok;
	}

	// don't honor shell type comments
	if( c == 0 /*|| c == '#'*/ ) goto nexttok;

	if( isdigit(c) || c=='.' ) {
		const char *s;

		// In objC, we can't take the address of a property...
		s=YY_CP;
		yylvp->jt_yy_number = parse_num(&s) ;
		SET_YY_CP(s);

		return(NUMBER);
	} else if( c == '"' ){		/* read to matching quote */
		const char *s;
		//SET_YY_CP( YY_CP + 1 );
		// objC can't take the address of a property...
		s=YY_CP+1;
		yylvp->jt_yy_string = match_quote(&s);
		SET_YY_CP(s);
#ifdef QUIP_DEBUG
if( debug ){ advise("yylex returning LEX_STRING"); }
#endif /* QUIP_DEBUG */
		return(LEX_STRING);
	} 


	else if( islegalkeywordchar(c) ){
		int tok;

		tok=name_token(QSP_ARG yylvp);

		// when a macro name is encountered, we
		// read the args, and push the string onto the
		// input stack
#ifdef QUIP_DEBUG
//if( debug ){ snprintf(ERROR_STRING,LLEN,"yylex returning token %d (%s)",tok,name_for_token(tok)); advise(ERROR_STRING); }
#endif // QUIP_DEBUG
		return(tok);

	} else if( ispunct(c) ){
		SET_YY_CP( YY_CP + 1 );
		yylvp->jt_yy_char = c;

#ifdef QUIP_DEBUG
if( debug ){ snprintf(ERROR_STRING,LLEN,"yylex returning char '%c' (0x%x)",c,c); advise(ERROR_STRING); }
#endif /* QUIP_DEBUG */
		return(c);
	} else {
		snprintf(ERROR_STRING,LLEN,"yylex:  no case for char '%c' (0x%x)",
			c,c);
		warn(ERROR_STRING);
		return(0);
	}
	/* NOTREACHED */
	return(0);
}

/* this function should go in the lexical analyser... */

static void init_parser(SINGLE_QSP_ARG_DECL)
{
	if( YY_INPUT_LINE == NULL ){
		YY_INPUT_LINE = new_stringbuf();
	}
	if( YY_LAST_LINE == NULL ){
		YY_LAST_LINE = new_stringbuf();
	}
	if( JSON_EXP_STR == NULL ){
		JSON_EXP_STR = new_stringbuf();
	}
	//assert( YY_INPUT_LINE != NULL );
	copy_string(YY_INPUT_LINE,"");	// clear record of input
	LAST_LINE_NUM=(-1);
	YY_CP="";

	json_qlevel = QLEVEL;
	parser_qsp = THIS_QSP;

	// disable_lookahead();	// to keep line numbers straight -
				// but may screw up EOF detection!?

	//SET_TOP_NODE(NULL);
}

JSON_Obj * _parse_json(SINGLE_QSP_ARG_DECL)		/** parse expression */
{
	int stat;

	init_parser(SINGLE_QSP_ARG);

	stat=yyparse(THIS_QSP);

	if( parsed_json_obj != NULL )	/* successful parsing */
	{
		advise("Parsing succeeded!");
	} else {
		// Do we get here on a syntax error???
		warn("Unsuccessfully parsed statement (top_node=NULL");
		snprintf(ERROR_STRING,LLEN,"status = %d\n",stat);	// suppress compiler warning
		advise(ERROR_STRING);
	}

	return parsed_json_obj;
} // end parse_stuff

static void emit_indentation(QSP_ARG_DECL  int lvl)
{
	while( lvl-- ){
		prt_msg_frag("  ");
	}
}

static void emit_one_thing(QSP_ARG_DECL  JSON_Thing * thing_p, int lvl);
static void emit_pairs(QSP_ARG_DECL  JSON_Obj *obj_p, int lvl);

static void emit_array_elements(QSP_ARG_DECL  JSON_Obj *obj_p, int lvl){
	int i;
	for(i=0;i<obj_p->jo_len;i++){
		JSON_Thing *thing_p;
		thing_p = obj_p->jo_thing_tbl[i];
		emit_indentation(QSP_ARG  lvl);
		emit_one_thing(QSP_ARG  thing_p, lvl);
		if( i < obj_p->jo_len-1 ){
			prt_msg(",");
		} else {
			prt_msg("");
		}
	}
}

static void emit_obj_after(QSP_ARG_DECL  JSON_Obj *obj_p, int lvl){
	switch( obj_p->jo_type ){
		case JSON_OBJ_TYPE_PAIRS:
			prt_msg("{");
			emit_pairs(QSP_ARG  obj_p,lvl+1);
			emit_indentation(QSP_ARG  lvl);
			prt_msg_frag("}");
			break;

		case JSON_OBJ_TYPE_ARRAY:
			prt_msg("[");
			emit_array_elements(QSP_ARG  obj_p,lvl+1 );
			emit_indentation(QSP_ARG  lvl);
			prt_msg_frag("]");
			break;
		default:
			snprintf(ERROR_STRING,LLEN,"Bad object type %d",obj_p->jo_type);
			warn(ERROR_STRING);
			break;
	}
}

static void emit_one_thing(QSP_ARG_DECL  JSON_Thing * thing_p, int lvl){
	switch( thing_p->jt_type ){
		case JSON_TYPE_OBJECT:
			emit_obj_after(QSP_ARG  thing_p->jt_u.u_obj_p, lvl);
			break;

		case JSON_TYPE_NUMBER:
			snprintf(MSG_STR,LLEN,"\"%g\"",thing_p->jt_u.u_number);
			prt_msg_frag(MSG_STR);
			break;
		case JSON_TYPE_STRING:
			snprintf(MSG_STR,LLEN,"\"%s\"",thing_p->jt_u.u_string);
			prt_msg_frag(MSG_STR);
			break;
		case JSON_TYPE_ID:
			snprintf(MSG_STR,LLEN,"ObjectId(\"%s\")",
				thing_p->jt_u.u_string);
			prt_msg_frag(MSG_STR);
			break;
		case JSON_TYPE_NULL:
			prt_msg_frag("null");
			break;
			
			
		default:
			snprintf(MSG_STR,LLEN,"Unhandled thing type case %d!?",
				thing_p->jt_type);
			prt_msg(MSG_STR);
			warn(MSG_STR);
			break;
	}
}

static void emit_one_pair(QSP_ARG_DECL JSON_Pair *pair_p, int lvl){
	emit_indentation(QSP_ARG  lvl);
	snprintf(MSG_STR,LLEN,"\"%s\"",pair_p->jp_key);
	prt_msg_frag(MSG_STR);
	prt_msg_frag(" : ");
	emit_one_thing(QSP_ARG  pair_p->jp_thing_p,lvl);
}

static void emit_pairs(QSP_ARG_DECL  JSON_Obj *obj_p, int lvl){
	int i;
	for(i=0;i<obj_p->jo_len;i++){
		emit_one_pair(QSP_ARG  obj_p->jo_pair_tbl[i],lvl);
		if( i < obj_p->jo_len-1 ){
			prt_msg(",");
		} else {
			prt_msg("");
		}
	}
}

#define dump_obj_with_indentation(obj_p, lvl) _dump_obj_with_indentation(QSP_ARG  obj_p, lvl)

static void _dump_obj_with_indentation(QSP_ARG_DECL  JSON_Obj *obj_p, int lvl)
{
	emit_indentation(QSP_ARG  lvl);
	emit_obj_after(QSP_ARG  obj_p,lvl);
}

static void _dump_json_thing(QSP_ARG_DECL  JSON_Thing *thing_p,int lvl)
{
	emit_indentation(QSP_ARG  lvl);
	emit_one_thing(QSP_ARG  thing_p,lvl);
}

void _dump_json_obj(QSP_ARG_DECL  JSON_Obj *obj_p)
{
	int indentation_level = 0;
	dump_obj_with_indentation(obj_p,indentation_level);
	prt_msg("");
}

void yyerror(Query_Stack *qsp,  char *s)
{
	const char *filename;
	int ql,n;
	/* get the filename and line number */

	filename=CURRENT_FILENAME;
	ql = QLEVEL;
	//n = THIS_QSP->qs_query[ql].q_lineno;
	n = current_line_number(SINGLE_QSP_ARG);

	snprintf(ERROR_STRING,LLEN,"%s, line %d:  %s",filename,n,s);
	warn(ERROR_STRING);

	snprintf(ERROR_STRING,LLEN,"\t%s",sb_buffer(YY_INPUT_LINE));
	advise(ERROR_STRING);
	/* print an arrow at the problem point... */
	n=(int)(strlen(sb_buffer(YY_INPUT_LINE))-strlen(YY_CP));
	n-=2;
	if( n < 0 ) n=0;
	strcpy(ERROR_STRING,"\t");
	while(n--) strcat(ERROR_STRING," ");
	strcat(ERROR_STRING,"^");
	advise(ERROR_STRING);

	/* we might use this to print an arrow at the problem point... */
	/*
	if( *YY_CP ){
		snprintf(ERROR_STRING,LLEN,"\"%s\" left in the buffer",YY_CP);
		advise(ERROR_STRING);
	} else advise("no buffered text");
	*/

}

#ifdef STANDALONE
double dstrcmp(char *s1,char *s2)
{
	double d;
	d=strcmp(s1,s2);
	return(d);
}
#endif /* STANDALONE */

static JSON_Obj *make_pair_list(JSON_Obj *old_obj,JSON_Obj *pair_obj)
{
	JSON_Obj *obj_p;
	int i, old_len, pair_len, new_len;

	obj_p = getbuf(sizeof(*obj_p));
	obj_p->jo_type = JSON_OBJ_TYPE_PAIRS;
	if( old_obj == NULL ){
		old_len = 0;
	} else {
		old_len = old_obj->jo_len;
		assert( old_obj->jo_type == JSON_OBJ_TYPE_PAIRS );
	}
	if( pair_obj == NULL ){
		assert(old_obj == NULL);
		pair_len = 0;
	} else {
		assert(pair_obj->jo_len == 1);
		pair_len = 1;
	}
	new_len = old_len + pair_len;
	obj_p->jo_len = new_len;
	if( new_len > 0 ){
		obj_p->jo_pair_tbl = getbuf( sizeof(JSON_Pair) * (old_len + 1) );
		for(i=0;i<old_len;i++){
			obj_p->jo_pair_tbl[i] = old_obj->jo_pair_tbl[i];
		}
		obj_p->jo_pair_tbl[i] = pair_obj->jo_pair_tbl[0];
	} else {
		obj_p->jo_pair_tbl = NULL;
	}
	return obj_p;
}

static JSON_Obj *make_pair(const char *key,JSON_Thing *thing_p){
	JSON_Obj *obj_p;
	obj_p = getbuf(sizeof(*obj_p));
	obj_p->jo_len = 1;
	obj_p->jo_type = JSON_OBJ_TYPE_PAIRS;
	obj_p->jo_pair_tbl = getbuf(sizeof(JSON_Pair *));
	obj_p->jo_pair_tbl[0] = getbuf(sizeof(JSON_Pair));
	obj_p->jo_pair_tbl[0]->jp_key = savestr(key);
	obj_p->jo_pair_tbl[0]->jp_thing_p = thing_p;
	return obj_p;
}

static JSON_Thing *make_obj_id(const char *id_str){
	JSON_Thing *thing_p;
fprintf(stderr,"make_obj_id:  id_str = %s\n",id_str);
	thing_p = getbuf(sizeof(JSON_Thing));
	thing_p->jt_type = JSON_TYPE_ID;
	thing_p->jt_u.u_string = savestr(id_str);
	return thing_p;
}

static JSON_Thing *make_string_thing(const char *s){
	JSON_Thing *thing_p;
	thing_p = getbuf(sizeof(JSON_Thing));
	thing_p->jt_type = JSON_TYPE_STRING;
	thing_p->jt_u.u_string = savestr(s);
	return thing_p;
}

static JSON_Thing *make_number_thing(double n){
	JSON_Thing *thing_p;
	thing_p = getbuf(sizeof(JSON_Thing));
	thing_p->jt_type = JSON_TYPE_NUMBER;
	thing_p->jt_u.u_number = n;
	return thing_p;
}

static JSON_Thing *make_object_thing(JSON_Obj *obj_p){
	JSON_Thing *thing_p;
	thing_p = getbuf(sizeof(JSON_Thing));
	thing_p->jt_type = JSON_TYPE_OBJECT;
	thing_p->jt_u.u_obj_p = obj_p;
	return thing_p;
}

static JSON_Thing *make_null_thing(){
	JSON_Thing *thing_p;
	thing_p = getbuf(sizeof(JSON_Thing));
	thing_p->jt_type = JSON_TYPE_NULL;
	thing_p->jt_u.u_obj_p = NULL;
	return thing_p;
}

static JSON_Obj *make_thing_array(JSON_Obj *old_obj_p, JSON_Thing *thing_p){
	JSON_Obj *new_obj_p;
	int old_len, i;

	if( old_obj_p == NULL ){
		old_len = 0;
	} else {
		old_len = old_obj_p->jo_len;
	}
	new_obj_p = getbuf( sizeof(JSON_Obj) );
	new_obj_p->jo_len = old_len+1;
	new_obj_p->jo_type = JSON_OBJ_TYPE_ARRAY;
	new_obj_p->jo_thing_tbl = getbuf( (old_len+1)*sizeof(JSON_Thing *) );
	for(i=0;i<old_len;i++){
		new_obj_p->jo_thing_tbl[i] = old_obj_p->jo_thing_tbl[i];
	}
	new_obj_p->jo_thing_tbl[i] = thing_p;
	return new_obj_p;
}

