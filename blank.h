#ifndef BLANK_H_
#define BLANK_H_

#ifndef true
	#define true 1
#endif
#ifndef false
	#define false 0
#endif
#ifndef BUFLEN
	#define BUFLEN 1024
#endif

#define OPERATOR_CNT 24
#define DATATYPE_SIZE 35
#define MINLEN 64
#define TOKEN_CNT 50

typedef struct node{
	int parentheses;
	char *name;
	struct node *parent;
	struct node *child_head;
	struct node *prev;
	struct node *next;
}node;

typedef struct operator_precedence{
	char *operator;
	int precedence; //연산자 우선순위
}operator_precedence;

void compare_tree(node *root1,  node *root2, int *result); //두 트리를 비교하여 result값에 정답이 같으면 1 다르면 0을 할당한다
node *make_tree(node *root, char (*tokens)[MINLEN], int *idx, int parentheses); //토큰을 연산자 우선순위에 맞게 트리구조로 만든다. 루트노드를 리턴함
node *change_sibling(node *parent); //부모와 연결된 자식노드를 해당 자식노드의 형제노드와 위치를 바꾸고 부모노드를 리턴한다.
node *create_node(char *name, int parentheses); //인자로 넘겨준 이름을 가지는 노드 생성후 리턴
int get_precedence(char *op); //op의 우선순위 리턴
int is_operator(char *op); //op가 연산자면 true리턴
void print(node *cur); //트리 구조를 출력하는 함수
node *get_operator(node *cur); //현재 노드의 연산자 노드를 리턴한다
node *get_root(node *cur); //현재트리의 루트 노드를 리턴
node *get_high_precedence_node(node *cur, node *new); //cur기준으로 new보다 우선순위 높은 노드 리턴
node *get_most_high_precedence_node(node *cur, node *new);//cur기준으로 new보다 우선순위 높은 노드의 최상위 부모노드 리턴
node *insert_node(node *old, node *new); //new를 old의 상위 노드로 만들고 new리턴
node *get_last_child(node *cur); //해당 노드의 자식 중 가장 마지막에 추가한 자식노드를 리턴
void free_node(node *cur); //트리에 할당된 메모리 해제하는 함수
int get_sibling_cnt(node *cur); //cur의 형제노드의 개수를 리턴한다

int make_tokens(char *str, char tokens[TOKEN_CNT][MINLEN]); //토큰을 분리하는 함수. 성공하면 true
int is_typeStatement(char *str); //0:타입 잘못 써짐, 1:타입 안 써짐, 2:타입 잘 써짐
int find_typeSpecifier(char tokens[TOKEN_CNT][MINLEN]); //조건에 맞는 타입지정자 인덱스 리턴
int find_typeSpecifier2(char tokens[TOKEN_CNT][MINLEN]);//조건에 맞는 구조체 타입지정자 인덱스 리턴
int is_character(char c); //숫자, 알파벳일 경우 1 리턴(기호일 경우 0리턴 된다는 소리)
int all_star(char *str); //전부 *이면 1, 하나라도 0이면 0을 리턴
int all_character(char *str); //숫자, 알파벳이 하나라도 있을 경우 1리턴, 하나도 없으면 0
int reset_tokens(int start, char tokens[TOKEN_CNT][MINLEN]); //분리된 토큰을 다시 합침. 실패시 0리턴
void clear_tokens(char tokens[TOKEN_CNT][MINLEN]); //토큰 변수 0으로 초기화
int get_token_cnt(char tokens[TOKEN_CNT][MINLEN]); //토큰의 개수를 리턴한다
char *rtrim(char *_str); //문자열의 오른쪽 공백 제거  
char *ltrim(char *_str); //문자열의 왼쪽 공백 제거
void remove_space(char *str); //문자열의 모든 공백 제거
int check_brackets(char *str); //괄호 짝이 맞는지 체크
char* remove_extraspace(char *str); //연속되는 공백을 최대1개로 줄인 문자열 리턴

#endif
