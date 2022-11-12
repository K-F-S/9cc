#include "9cc.h"

// 次のトークンが期待している記号の時には、トークンを１つ読み進めて
// 真を返す。それ以外の場合には偽を返す。
bool consume(char *op){
	if (token->kind != TK_RESERVED ||
		strlen(op) != token->len ||
		memcmp(token->str, op, token->len))
		return false;
	token = token->next;
	return true;
}

// 次のトークンが変数の場合は、トークンを１つ読み進めて
// その変数を返す。それ以外はNULLを返す
Token *consume_ident(void) {
	Token *tok = NULL;

	if (token->kind != TK_IDENT)
		return tok;
	tok = token;
	token = token->next;
	return tok;
}

// 次のトークンが期待している記号の時には、トークンを１つ読み進める
// それ以外の場合はエラーを報告する
void expect(char *op){
	if (strlen(op) != token->len ||
		memcmp(token->str, op, token->len))
		error_at(token->str,"'%s'ではありません", op);
	token = token->next;
}

// 次のトークンが数値の場合、トークンを１つ読み進めてその数値を返す
// それ以外の場合にはエラーを報告する
int expect_number(){
	if (token->kind != TK_NUM)
		error_at(token->str, "数ではありません");
	int val = token->val;
	token = token->next;
	return val;
}

bool at_eof() {
	return token->kind == TK_EOF;
}

//-----------------------------
// 	 抽象構造木
//------------------------------

Node *new_node(NodeKind kind, Node *lhs, Node *rhs){
	Node *node = calloc(1, sizeof(Node));
	node->kind = kind;
	node->lhs = lhs;
	node->rhs = rhs;
	return node;
}

Node *new_node_num(int val){
	Node *node = calloc(1, sizeof(Node));
	node->kind = ND_NUM;
	node->val = val;
	return node;
}

// program = stmt*
void program() {
	int i = 0;
	while (!at_eof())
		code[i++] = stmt();
	code[i] = NULL;
}

// stmt = expr ";"
Node *stmt() {
	Node *node = expr();
	expect(";");
	return node;
}

// expr = assign
Node *expr(){
	return assign();
}

// assign = equality ("=" assign)?
Node *assign() {
	Node *node = equality();
	if (consume("="))
		node = new_node(ND_ASSIGN, node, assign());
	return node;
}

// equality = relational("==" relational | "!=" relational)*
Node *equality(){
	Node *node = relational();

	for(;;){
		if (consume("=="))
			node = new_node(ND_EQ, node, relational());
		else if (consume("!="))
			node = new_node(ND_NE, node, relational());
		else
			return node;
	}
}

// relational = add("<" add | "<=" add | ">" add | ">=" add)*
Node *relational(){
	Node *node = add();

	for(;;){
		if (consume("<"))
			node = new_node(ND_LT, node, add());
		else if (consume("<="))
			node = new_node(ND_LE, node, add());
		else if (consume(">"))
			node = new_node(ND_LT, add(), node);
		else if (consume(">="))
			node = new_node(ND_LE, add(), node);
		else
			return node;
	}
}

// add = mul("+" mul | "-" mul)*
Node *add(){
	Node *node = mul();

	for(;;){
		if (consume("+"))
			node = new_node(ND_ADD, node, mul());
		else if (consume("-"))
			node = new_node(ND_SUB, node, mul());
		else
			return node;
	}
}

// mul = unary ("*" unary | "/" unary)*
Node *mul(){
	Node *node = unary();

	for(;;){
		if(consume("*"))
			node = new_node(ND_MUL, node, unary());
		else if (consume("/"))
			node = new_node(ND_DIV, node, unary());
		else
			return(node);
	}
}

// unary = ("+" | "-")? primary
Node *unary(){
	if (consume("+"))
		return primary();
	if (consume("-"))
		return new_node(ND_SUB, new_node_num(0), primary());	// 0 - primary　にする
	return primary();
}

// primary = num | ident | "(" expr ")"
Node *primary(){
	// 次のトークンが ( なら、) のはず
	if (consume("(")){
		Node *node = expr();
		expect(")");
		return node;
	}
	
	// 次のトークンがident（変数）の場合
	Token *tok = consume_ident();
	if (tok) {
		Node *node = calloc(1, sizeof(Node));
		node->kind = ND_LVAR;
		node->offset = (tok->str[0] - 'a' + 1) * 8;
		return node;
	}

	// そうでなければ数値のはず
	return new_node_num(expect_number());
}

