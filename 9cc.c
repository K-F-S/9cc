#include <ctype.h>
#include <stdarg.h>

#include "9cc.h"

//----------------------------------
//	 エラー処理
//----------------------------------

// エラーを報告するための関数
// printfと同じ引数を取る
void error(char *fmt, ...){
	va_list ap;
	va_start(ap, fmt);
	fprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	exit(1);
}

// エラーの個所を指定して報告する
void error_at(char *loc, char *fmt, ...){
	va_list ap;
	va_start(ap, fmt);

	int pos = loc - user_input;
	fprintf(stderr, "%s\n", user_input);
	fprintf(stderr, "%*s", pos, " ");	// pos個の空白を出力
	fprintf(stderr, "^");

	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	exit(1);
}


//----------------------------------
//	 トークナイズ
//----------------------------------

// 新しいトークンを作成してcurに繋げる
Token *new_token(TokenKind kind, Token *cur, char *str, int len) {
	Token *tok = calloc(1, sizeof(Token));
	tok->kind = kind;
	tok->str = str;
	tok->len = len;
	cur->next = tok;
	return tok;
}

// ２つの文字列が同じ内容なら１を返す
bool startswitch(char *p, char *q){
	return (memcmp(p, q, strlen(q)) == 0);
}

// 入力文字列pをトークナイズしてそれを返す
Token *tokenize(char *p){
	Token head;
	head.next = NULL;
	Token *cur = &head;

	while (*p) {
		// 空白文字をスキップ
		if (isspace(*p)){
			p++;
			continue;
		}
		
		if ('a' <= *p && *p <= 'z') {
			cur = new_token(TK_IDENT, cur, p++, 1);
			continue;
		}

		if (startswitch(p, "==") || startswitch(p, "!=") ||
				startswitch(p, "<=") || startswitch(p, ">=")){
			cur = new_token(TK_RESERVED, cur, p, 2);
			p += 2;
			continue;
		}

		if (strchr( "+-*/()<>=", *p)) {
			cur = new_token(TK_RESERVED, cur, p++, 1);
			continue;
		}

		if (isdigit(*p)){
			cur = new_token(TK_NUM, cur, p, 0);
			char *q = p;
			cur->val = strtol(p, &p, 10); // 変換終了位置が &p に格納される
			cur->len = p - q;
			continue;
		}
		
		if(*p == ';') {
			cur = new_token(TK_EOF, cur, p++, 1);
			continue;
		}

		error_at(p, "トークナイズできません");
	}

	new_token(TK_EOF, cur, p, 0);
	return head.next;
}

