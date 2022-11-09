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
bool at_eof() {
	return token->kind == TK_EOF;
}

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

		if (startswitch(p, "==") || startswitch(p, "!=") ||
				startswitch(p, "<=") || startswitch(p, ">=")){
			cur = new_token(TK_RESERVED, cur, p, 2);
			p += 2;
			continue;
		}

		if (strchr( "+-*/()<>", *p)) {
			cur = new_token(TK_RESERVED, cur, p++, 1);
			continue;
		}

		if (isdigit(*p)){
			cur = new_token(TK_NUM, cur, p, 0);
			char *q = p;
			cur->val = strtol(p, &p, 10);
			cur->len = p - q;
			continue;
		}

		error_at(p, "トークナイズできません");
	}

	new_token(TK_EOF, cur, p, 0);
	return head.next;
}

//------------------------------------
//	  メイン
//------------------------------------

int main(int argc, char **argv){
	if(argc != 2){
		error("引数の個数が正しくありません");
		return 1;
	}
	
	// 入力した文字列をuser_inputに保存
	user_input = argv[1];

	// トークナイズしてパースする
	token = tokenize(user_input);
	Node *node = expr();
	
	// アセンブリの前半部分を出力
	printf(".intel_syntax noprefix\n");
	printf(".globl main\n");
	printf("main:\n");
	
	// 抽象構文木を下りながらコード生成
	gen(node);

	// スタックトップに式全体の値が残っているはず	
	// それをRAXにロードして関数の返り値とする
	printf("	pop rax\n");
	printf("	ret\n");
	return 0;
}
