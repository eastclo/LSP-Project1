#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include "blank.h"

char datatype[DATATYPE_SIZE][MINLEN] = {"int", "char", "double", "float", "long"
	, "short", "ushort", "FILE", "DIR","pid"
		,"key_t", "ssize_t", "mode_t", "ino_t", "dev_t"
		, "nlink_t", "uid_t", "gid_t", "time_t", "blksize_t"
		, "blkcnt_t", "pid_t", "pthread_mutex_t", "pthread_cond_t", "pthread_t"
		, "void", "size_t", "unsigned", "sigset_t", "sigjmp_buf"
		, "rlim_t", "jmp_buf", "sig_atomic_t", "clock_t", "struct"};


operator_precedence operators[OPERATOR_CNT] = { //연산자별 우선순위
	{"(", 0}, {")", 0}
	,{"->", 1}	
	,{"*", 4}	,{"/", 3}	,{"%", 2}	
	,{"+", 6}	,{"-", 5}	
	,{"<", 7}	,{"<=", 7}	,{">", 7}	,{">=", 7}
	,{"==", 8}	,{"!=", 8}
	,{"&", 9}
	,{"^", 10}
	,{"|", 11}
	,{"&&", 12}
	,{"||", 13}
	,{"=", 14}	,{"+=", 14}	,{"-=", 14}	,{"&=", 14}	,{"|=", 14}
};

void compare_tree(node *root1,  node *root2, int *result)
{
	node *tmp;
	int cnt1, cnt2;

	if(root1 == NULL || root2 == NULL){
		*result = false;
		return;
	}

	if(!strcmp(root1->name, "<") || !strcmp(root1->name, ">") || !strcmp(root1->name, "<=") || !strcmp(root1->name, ">=")){
		if(strcmp(root1->name, root2->name) != 0){ //해당 연산자에서 서로 다를 경우

			if(!strncmp(root2->name, "<", 1)) //부등호를 반대방향으로 바꿔주고
				strncpy(root2->name, ">", 1);

			else if(!strncmp(root2->name, ">", 1))
				strncpy(root2->name, "<", 1);

			else if(!strncmp(root2->name, "<=", 2))
				strncpy(root2->name, ">=", 2);

			else if(!strncmp(root2->name, ">=", 2))
				strncpy(root2->name, "<=", 2);

			root2 = change_sibling(root2); //현재 root2와 연결된 자식노드를 바꿔준다
		}
	}

	if(strcmp(root1->name, root2->name) != 0){ //현재 둘이 다를경우 false
		*result = false;
		return;
	}

	if((root1->child_head != NULL && root2->child_head == NULL) //둘 중 하나만 자식노드가 없다면 false
			|| (root1->child_head == NULL && root2->child_head != NULL)){ 
		*result = false; 
		return;
	}

	else if(root1->child_head != NULL){ //자식노드가 존재한다면
		if(get_sibling_cnt(root1->child_head) != get_sibling_cnt(root2->child_head)){ //자식노드의 형제노드의 개수가 같아야한다
			*result = false;
			return;
		}

		if(!strcmp(root1->name, "==") || !strcmp(root1->name, "!=")) //해당 연산자의 경우
		{
			compare_tree(root1->child_head, root2->child_head, result); //자식노드로 재귀호출

			if(*result == false) //false일 경우
			{
				*result = true;
				root2 = change_sibling(root2); //자식노드를 바꾸고
				compare_tree(root1->child_head, root2->child_head, result); //다시 재귀호출을 한다
			}
		}
		else if(!strcmp(root1->name, "+") || !strcmp(root1->name, "*")
				|| !strcmp(root1->name, "|") || !strcmp(root1->name, "&")
				|| !strcmp(root1->name, "||") || !strcmp(root1->name, "&&")) //해당 연산자의 경우
		{
			if(get_sibling_cnt(root1->child_head) != get_sibling_cnt(root2->child_head)){ //자식노드의 형제노드의 개수가 같아야하고
				*result = false;
				return;
			}

			tmp = root2->child_head; 

			while(tmp->prev != NULL)  
				tmp = tmp->prev; //자식노드 중 가장 왼쪽의 노드에서부터

			while(tmp != NULL) //더이상 탐색할 형제노드가 없으면 종료
			{
				compare_tree(root1->child_head, tmp, result); //result가 true가 나올 때까지 tmp의 형제노드를 탐색한다

				if(*result == true)
					break;
				else{
					if(tmp->next != NULL)
						*result = true;
					tmp = tmp->next;
				}
			}
		}
		else{
			compare_tree(root1->child_head, root2->child_head, result); //자식노드로 재귀호출한다
		}
	}	


	if(root1->next != NULL){ //형제노드가 있을 경우

		if(get_sibling_cnt(root1) != get_sibling_cnt(root2)){ //둘이 형제노드의 개수가 같아야한다
			*result = false;
			return;
		}

		if(*result == true) //현재 결과가 true일 때
		{
			tmp = get_operator(root1);  //현재 노드의 연산자 노드를 가져옴

			if(!strcmp(tmp->name, "+") || !strcmp(tmp->name, "*")
					|| !strcmp(tmp->name, "|") || !strcmp(tmp->name, "&")
					|| !strcmp(tmp->name, "||") || !strcmp(tmp->name, "&&"))
			{ //가져온 연산자노드가 이와같은 연산자일 경우	
				tmp = root2;

				while(tmp->prev != NULL) //가장 왼쪽 형제노드로 이동
					tmp = tmp->prev;

				while(tmp != NULL) //더이상 탐색할 형제노드가 없으면 종료
				{
					compare_tree(root1->next, tmp, result); //result가 true가 나올 때까지 tmp의 형제노드를 탐색한다

					if(*result == true)
						break;
					else{
						if(tmp->next != NULL)
							*result = true;
						tmp = tmp->next;
					}
				}
			}

			else
				compare_tree(root1->next, root2->next, result); //형제노드로 재귀호출한다
		}
	}
}

int make_tokens(char *str, char tokens[TOKEN_CNT][MINLEN])  //토큰 생성
{
	char *start, *end;
	char tmp[BUFLEN];
	char str2[BUFLEN];
	char *op = "(),;><=!|&^/+-*\""; 
	int row = 0;
	int i;
	int isPointer;
	int lcount, rcount;
	int p_str;

	clear_tokens(tokens); //토큰 변수 초기화

	start = str; //정답의 시작 위치 포인터

	if(is_typeStatement(str) == 0)  //타입이 잘못 써져있으므로 에러
		return false;	

	while(1) //토큰을 하나씩 처리해서 tokens 변수에 넣기
	{
		if((end = strpbrk(start, op)) == NULL) //op에서 가장 첫 번째로 일치되는 문자
			break;

		if(start == end){ //찾은 문자가 남은 문자열의 가장 첫 번째라면
			//현재 처리해야할 토큰이 구분자, 연산자 또는 "asd asdf" 같이 공백이 포함될 수도 있는 string-literal이라면
			if(!strncmp(start, "--", 2) || !strncmp(start, "++", 2)){ //현재 찾은 기호가 ++, --의 증감연산자인 경우
				//증감연산자의 경우 뒤의 변수와 같이 토큰에 들어간다.
				if(!strncmp(start, "++++", 4)||!strncmp(start,"----",4)) //증감 연산자 2개 연속으로 되어있을 경우 에러처리 
					return false;

				if(is_character(*ltrim(start + 2))){ //증감연산자의 뒤에 변수가 붙어야 하므로 문자인지 검사한다.(전위 증감연산자)
					if(row > 0 && is_character(tokens[row - 1][strlen(tokens[row - 1]) - 1]))//증감연산자 앞뒤로 변수가 되면 안되므로
						return false; //에러처리

					end = strpbrk(start + 2, op); //증감연산자 뒤로 다른 기호가 있는지 검사하고 
					if(end == NULL) //없으면 밑에 루프에서 증감연산자 뒤의 변수 전체를 토큰에 넣어줘야 하니까 
						end = &str[strlen(str)]; //가장 마지막 주소를 넘겨줌
					while(start < end) { //증감연산자와 뒤에 붙은 문자를 탐색
						if(*(start - 1) == ' ' && is_character(tokens[row][strlen(tokens[row]) - 1]))  
							return false; //문자열 사이에 연산자 없이 공백만 존재하므로 에러처리
						else if(*start != ' ') //공백이 아니면 토큰에 계속 넣어줌
							strncat(tokens[row], start, 1); 
						start++; //다음 토큰 검색을 위해 start이동
					}
				}

				else if(row>0 && is_character(tokens[row - 1][strlen(tokens[row - 1]) - 1])){ //이전의 토큰이 문자열이라면 후위증감연산자
					if(strstr(tokens[row - 1], "++") != NULL || strstr(tokens[row - 1], "--") != NULL) //단 앞의 토큰이 증감연산자가 붙어있지 않아야 한다.
						return false;

					memset(tmp, 0, sizeof(tmp));
					strncpy(tmp, start, 2);
					strcat(tokens[row - 1], tmp); //이전 토큰에 후위증감연산자를 붙여준다
					start += 2;
					row--; //이전 토큰을 수정했으므로 다시 현재 토큰을 생성하기 위해(loop 마지막에 row++을 한다)
				}
				else{ //그 이외의 경우
					memset(tmp, 0, sizeof(tmp));
					strncpy(tmp, start, 2); //증감연산자를 토큰에 이어준다
					strcat(tokens[row], tmp);
					start += 2;
				}
			} // ex) --a 또는 b++ 가 토큰에 들어감

			else if(!strncmp(start, "==", 2) || !strncmp(start, "!=", 2) || !strncmp(start, "<=", 2)
					|| !strncmp(start, ">=", 2) || !strncmp(start, "||", 2) || !strncmp(start, "&&", 2) 
					|| !strncmp(start, "&=", 2) || !strncmp(start, "^=", 2) || !strncmp(start, "!=", 2) 
					|| !strncmp(start, "|=", 2) || !strncmp(start, "+=", 2)	|| !strncmp(start, "-=", 2) 
					|| !strncmp(start, "*=", 2) || !strncmp(start, "/=", 2)){ //현재 찾은 기호가 비교, 논리, 비트, 대입 연산자일 경우

				strncpy(tokens[row], start, 2); //해당 연산자는 개별 토큰으로 들어간다.
				start += 2;
			}
			else if(!strncmp(start, "->", 2)) //현재 찾은 기호가 멤버 포인터 연산자일 경우
			{
				end = strpbrk(start + 2, op); //멤버 포인터 연산자 뒤에 다음 기호 전까지의 문자열을 
				//토큰에 넣기 위해
				if(end == NULL)				//다음 기호가 없다면
					end = &str[strlen(str)]; //마지막까지 토큰에 넣기 위해

				while(start < end){ //asd -> asd adf가 들어가는 경우 예외처리를 해야함, 설계과제엔 상관없음
					if(*start != ' ')
						strncat(tokens[row - 1], start, 1); 
					start++;
				}
				row--; //이전 토큰을 수정했으므로
			} //ex) asd -> asd 전체가 토큰에 들어감
			else if(*end == '&') //현재 기호가 &일 경우
			{
				if(row == 0 || (strpbrk(tokens[row - 1], op) != NULL)){ //주소 연산일 때
					end = strpbrk(start + 1, op);//다음 기호 전까지
					if(end == NULL)				//다음 기호가 없으면
						end = &str[strlen(str)];//마지막까지 토큰에 넣기 위해

					strncat(tokens[row], start, 1); //일단 &를 토큰에 넣고
					start++;

					while(start < end){ //그 이후 문자를 넣을 때
						if(*(start - 1) == ' ' && tokens[row][strlen(tokens[row]) - 1] != '&') //&와 변수 사이에 공백이 있으면 안됨
							return false;
						else if(*start != ' ') //공뱅이 아닐 때만 토큰에 넣는다
							strncat(tokens[row], start, 1); 
						start++;
					}
				}

				else{ //비트연산일 경우 해당 연산자는 토큰에 그대로 들어감
					strncpy(tokens[row], start, 1);
					start += 1;
				}

			} //ex) 비트연산은 &, 주소연산은 &asd
			else if(*end == '*') //현재 문자열의 첫 번째가 * 문자일 경우
			{
				isPointer=0;

				if(row > 0)
				{

					for(i = 0; i < DATATYPE_SIZE; i++) {
						if(strstr(tokens[row - 1], datatype[i]) != NULL){ 
							strcat(tokens[row - 1], "*");//포인터 변수 선언부일 때 이전 토큰에 붙여줌
							start += 1;					//ex)이전 토큰에 int* 가 들어감	
							isPointer = 1;
							break;
						}
					}
					if(isPointer == 1) //*은 이전 토큰에 붙여줬으니 다시 현재토큰 생성을 위해 continue
						continue;
					if(*(start+1) !=0) //*이 마지막이 아니라면 이후 문자에 위치함
						end = start + 1;


					if(row>1 && !strcmp(tokens[row - 2], "*") && (all_star(tokens[row - 1]) == 1)){ //이전 토큰이 참조연산이면
						strncat(tokens[row - 1], start, end - start); //이전 토큰에 *추가한다
						row--;
					}


					else if(is_character(tokens[row - 1][strlen(tokens[row - 1]) - 1]) == 1){ //이전 토큰이 문자열이라면
						strncat(tokens[row], start, end - start);  // 곱하기 연산이므로 개별로 토큰에 추가
					}


					else if(strpbrk(tokens[row - 1], op) != NULL){ //이전 토큰이 기호라면		
						strncat(tokens[row] , start, end - start); //포인터 연산이므로 토큰에 추가

					}
					else //그 외의 경우 개별 토큰으로 저장
						strncat(tokens[row], start, end - start);

					start += (end - start);
				}

				else if(row == 0) //무조건 포인터 연산
				{
					if((end = strpbrk(start + 1, op)) == NULL){ //다른 기호가 없다면
						strncat(tokens[row], start, 1);//토큰에 개별로 들어감
						start += 1;
					}
					else{ //다른 기호가 있다면 포인터 연산인데
						while(start < end){
							if(*(start - 1) == ' ' && is_character(tokens[row][strlen(tokens[row]) - 1]))//뒤 변수명에 공백이 있으며 에러
								return false;
							else if(*start != ' ') //변수명 사이의 공백이 아닌경우는 토큰에 넣어줌
								strncat(tokens[row], start, 1);
							start++; 	
						}
						if(all_star(tokens[row])) //all_star일 경우는 *이 하나가 추가된 것
							row--; //포인터 연산인데 아직 뒷문자가 안 왔으므로 토큰에 같이 넣어주기 위해

					}
				}
			}
			else if(*end == '(')  //현재 찾은 기호가 ( 일 경우
			{
				lcount = 0;
				rcount = 0;
				if(row>0 && (strcmp(tokens[row - 1],"&") == 0 || strcmp(tokens[row - 1], "*") == 0)){ //전 토큰이 *이나 &일 경우
					while(*(end + lcount + 1) == '(') //괄호가 연속해서 있는경우 개수 체크
						lcount++;
					start += lcount;

					end = strpbrk(start + 1, ")"); //이후 닫힌 괄호 체크

					if(end == NULL) //닫힌 괄호가 없으면 에러
						return false;
					else{
						while(*(end + rcount +1) == ')') //닫힌괄호가 연속해서 있는경우 개수 체크
							rcount++;
						end += rcount;

						if(lcount != rcount) //괄호 개수가 맞지 않으면 에러
							return false;

						if( (row > 1 && !is_character(tokens[row - 2][strlen(tokens[row - 2]) - 1])) || row == 1){ 
							strncat(tokens[row - 1], start + 1, end - start - rcount - 1); //괄호 제외하고 내부만 * 또는 &과 이어서 토큰으로 만듬
							row--;
							start = end + 1;
						} //ex) +*( 이나 &( 같은 상황
						else{  //그 외의 경우는 *, &이 이항연산자일 경우
							strncat(tokens[row], start, 1); //개별 토큰으로 만듬
							start += 1;
						}
					}

				}
				else{ //전 토큰이 *이나 &가 아니면 개별 토큰으로 만듬
					strncat(tokens[row], start, 1);
					start += 1;
				}

			}
			else if(*end == '\"')  //현재 찾은 기호가 " 문자인 경우
			{
				end = strpbrk(start + 1, "\""); //문자열이므로 다음 "를 찾는다.

				if(end == NULL) //없으면 문자열이 제대로 된게 아니므로 에러
					return false;

				else{
					strncat(tokens[row], start, end - start + 1); //문자열 전체를 토큰으로 만들어주고
					start = end + 1; //다음 토큰을 위해 start 이동
				}

			}

			else{ //현재 찾은 기호가 그 외 다른 문자인 경우

				if(row > 0 && !strcmp(tokens[row - 1], "++")) //단항연산자 뒤에 다시 기호가 오면 에러
					return false;

				if(row > 0 && !strcmp(tokens[row - 1], "--"))
					return false;

				strncat(tokens[row], start, 1); //일단 개별 토큰으로 만들고 
				start += 1;

				if(!strcmp(tokens[row], "-") || !strcmp(tokens[row], "+") || !strcmp(tokens[row], "--") || !strcmp(tokens[row], "++")){
					if(row == 0) //-, +, --, ++이 토큰에 들어갔는데 첫 번째 토큰이라면
						row--; //단항연산이므로 현재 토큰에 이어붙이기 위해 row--

					else if(!is_character(tokens[row - 1][strlen(tokens[row - 1]) - 1])){ //이전 토큰의 마지막이 기호였을 때

						if(strstr(tokens[row - 1], "++") == NULL && strstr(tokens[row - 1], "--") == NULL) //--, ++이 아닌 다른 기호라면 단항연산이다
							row--;
					}
				}
			}
		} 
		else{ //start != end (start에 op 이외의 문자가 있다) :start에서 end이전까지는 문자열만 들어가거나 참조연산자와 같이 들어간다.
			if(all_star(tokens[row - 1]) && row > 1 && !is_character(tokens[row - 2][strlen(tokens[row - 2]) - 1]))   
				row--; //이전 토큰이 전부 *이고, 전전 토큰이 의 가장 마지막이 알파벳, 숫자가 아닌경우				
			//이전 토큰에 start ~ end-1 까지의 문자를 이어주기 위해 row--
			if(all_star(tokens[row - 1]) && row == 1) //전전 토큰이 없을 때는 이전 토큰만 확인하면 됨
				row--;	//포인터 연산일 경우 같이 토큰에 넣기 위함 

			for(i = 0; i < end - start; i++){ //결국 token[row]에는 start에서 end 직전까지의 문자들이 저장된다.
				if(i > 0 && *(start + i) == '.'){ //.이라면 token에 복사
					strncat(tokens[row], start + i, 1);

					while( *(start + i +1) == ' ' && i< end - start ) //이후 공백 패스
						i++; 
				}
				else if(start[i] == ' '){ //공백 패스
					while(start[i] == ' ')
						i++;
					break;
				}
				else
					strncat(tokens[row], start + i, 1); //.이 아니라면 token에 한 글자씩 복사
			}

			if(start[0] == ' '){ //만약 현재 남은 문자열의 첫 번째가 0이면 이하 과정 생략하고
				start += i;		//다시 while루트로 돌아가 op를 탐색한다.
				continue;
			}			//다음 while 루프 탐색을 위해 남은 문자열을 조정해준다.
			start += i; //start에서 start + i - 1까지는 token[row]에 들어간 상태
		}

		strcpy(tokens[row], ltrim(rtrim(tokens[row]))); //token[row]에 저장된 문자열에 왼쪽,오른쪽 공백을 제거한다.
		if(row > 0 && is_character(tokens[row][strlen(tokens[row]) - 1])//현재 처리중인 토큰의 마지막이 숫자 또는 알파벳일 때 
				&& (is_typeStatement(tokens[row - 1]) == 2 //1.이전 토큰이 데이터 타입 문장이거나
					|| is_character(tokens[row - 1][strlen(tokens[row - 1]) - 1])//2.이전 토큰 마지막이 숫자 또는 알파벳이거나 
					|| tokens[row - 1][strlen(tokens[row - 1]) - 1] == '.' ) ){//3. 이전 토큰 마지막이 .일 경우 
			//대략 int asd, struct student asd, asd.cnt 같은 상황일 때 
			if(row > 1 && strcmp(tokens[row - 2],"(") == 0) //전전 토큰이 열린괄호일 때
			{
				if(strcmp(tokens[row - 1], "struct") != 0 && strcmp(tokens[row - 1],"unsigned") != 0) //struct나 unsigned가 아니면 안 된다
					return false; 
			}
			else if(row == 1 && is_character(tokens[row][strlen(tokens[row]) - 1])) { //row 1 에서, 현재 토큰의 마지막이 숫자 또는 알파벳일 때 
				if(strcmp(tokens[0], "extern") != 0 && strcmp(tokens[0], "unsigned") != 0 && is_typeStatement(tokens[0]) != 2)	
					return false; //그 이전 토큰이 extern, unsigned이 아닌데 type statement도 아니라면
			}
			else if(row > 1 && is_typeStatement(tokens[row - 1]) == 2){ //이전 토큰이 type문장일 경우
				if(strcmp(tokens[row - 2], "unsigned") != 0 && strcmp(tokens[row - 2], "extern") != 0) 
					return false; //unsinged나 extern은 이후에 type statement가 와야하므로 아니면 에러
			} 

		}

		if((row == 0 && !strcmp(tokens[row], "gcc")) ){ //gcc문장일 경우
			clear_tokens(tokens); //토큰 초기화 하고
			strcpy(tokens[0], str);	 //전체 문장을 그대로 넣는다
			return 1; //gcc 문장인 경우 전체 결과가 똑같아야 하기에 토큰을 나눠야할 필요가 없다
		} 

		row++; //다음 토큰 생성을 위해
	}

	if(all_star(tokens[row - 1]) && row > 1 && !is_character(tokens[row - 2][strlen(tokens[row - 2]) - 1]))  
		row--;			//참조연산일 경우 이전토큰하고 붙이기 위해 row를 줄여줌	
	if(all_star(tokens[row - 1]) && row == 1)   
		row--;	

	for(i = 0; i < strlen(start); i++) //현재 start 이후 op가 없다 
	{
		if(start[i] == ' ')
		{
			while(start[i] == ' ') //남은 문자열에서 공백은 스킵
				i++;
			if(start[0]==' ') { //해당 공백이 문자열 왼쪽 공백이라면
				start += i; //문자열 왼쪽 공백이 없는 상태로 만듦
				i = 0;
			}
			else
				row++; //공백이면 다음 토큰으로

			i--;
		} 
		else
		{
			strncat(tokens[row], start + i, 1); //문자 하나씩 토큰에 넣는다
			if( start[i] == '.' && i<strlen(start)){  //.을 넣었다면 이후 공백 스킵 
				while(start[i + 1] == ' ' && i < strlen(start))
					i++;
			}
		}
		strcpy(tokens[row], ltrim(rtrim(tokens[row]))); //현재 토큰에 왼쪽, 오른쪽 공백 제거

		if(!strcmp(tokens[row], "lpthread") && row > 0 && !strcmp(tokens[row - 1], "-")){ //전 토큰이 -, 현재 토큰이 lpthread면  
			strcat(tokens[row - 1], tokens[row]);  //전 토큰을 -lpthread로 만들어주고
			memset(tokens[row], 0, sizeof(tokens[row])); //현재 토큰 초기화
			row--; 
		}
		else if(row > 0 && is_character(tokens[row][strlen(tokens[row]) - 1])  //현재 처리중인 토큰의 마지막이 숫자 또는 알파벳일 때 
				&& (is_typeStatement(tokens[row - 1]) == 2  //1.이전 토큰이 데이터 타입 문장이거나
					|| is_character(tokens[row - 1][strlen(tokens[row - 1]) - 1]) //2.이전 토큰 마지막이 숫자 또는 알파벳이거나 
					|| tokens[row - 1][strlen(tokens[row - 1]) - 1] == '.') ){ //3. 이전 토큰 마지막이 .일 경우 
			//대략 int asd, struct student asd, asd.cnt 같은 상황일 때 	
			if(row > 1 && strcmp(tokens[row-2],"(") == 0) //전전 토큰이 열린괄호일 때
			{
				if(strcmp(tokens[row-1], "struct") != 0 && strcmp(tokens[row-1], "unsigned") != 0)//struct나 unsigned가 아니면 안 된다
					return false;
			}
			else if(row == 1 && is_character(tokens[row][strlen(tokens[row]) - 1])) {//row 1 에서, 현재 토큰의 마지막이 숫자 또는 알파벳일 때 
				if(strcmp(tokens[0], "extern") != 0 && strcmp(tokens[0], "unsigned") != 0 && is_typeStatement(tokens[0]) != 2) //그 이전 토큰이 extern, unsigned이 아닌데 type statement도 아니라면
					return false;
			}
			else if(row > 1 && is_typeStatement(tokens[row - 1]) == 2){//이전 토큰이 type문장일 경우
				if(strcmp(tokens[row - 2], "unsigned") != 0 && strcmp(tokens[row - 2], "extern") != 0)
					return false; //unsinged나 extern은 이후에 type statement가 와야하므로 아니면 에러
			}//line 443과 같은 예외처리
		} 
	}


	if(row > 0)
	{
		if(strcmp(tokens[0], "#include") == 0 || strcmp(tokens[0], "include") == 0 || strcmp(tokens[0], "struct") == 0){  //선언부여서 공백을 제외하고 같아야함
			clear_tokens(tokens);  //토큰 분리한 것들 전부 초기화
			strcpy(tokens[0], remove_extraspace(str));  //최대 연속 공백 1개로 만들어 토큰에 저장
		}
	}

	if(is_typeStatement(tokens[0]) == 2 || strstr(tokens[0], "extern") != NULL){ //extern이 있거나 type문장일 경우
		for(i = 1; i < TOKEN_CNT; i++){
			if(strcmp(tokens[i],"") == 0)  //더이상 토큰이 없으면 끝
				break;		       

			if(i != TOKEN_CNT -1 ) //가장 마지막 토큰이 아니라면 사이마다 공백을 넣어줌
				strcat(tokens[0], " ");
			strcat(tokens[0], tokens[i]); //뒤에 토큰을 첫 번째 토큰에 이어주고
			memset(tokens[i], 0, sizeof(tokens[i])); //쓸모 없어진 토큰은 초기화
		}
	}


	while((p_str = find_typeSpecifier(tokens)) != -1){  //조건에 맞는 타입지정자가 있으면
		if(!reset_tokens(p_str, tokens))//분리된 토큰을 다시 합쳐준다.
			return false;
	}


	while((p_str = find_typeSpecifier2(tokens)) != -1){ //조건2에 맞는 타입지정자가 있으면
		if(!reset_tokens(p_str, tokens))//분리된 토큰을 다시 합쳐준다.
			return false;
	}

	return true;
}

node *make_tree(node *root, char (*tokens)[MINLEN], int *idx, int parentheses)//우선순위에 따라 토큰을 트리구조로 만듦
{
	node *cur = root;
	node *new;
	node *saved_operator;
	node *operator;
	int fstart;
	int i;

	while(1)	
	{
		if(strcmp(tokens[*idx], "") == 0)//다음 토큰이 없으면 종료
			break;

		if(!strcmp(tokens[*idx], ")")) //재귀호출 리턴
			return get_root(cur);

		else if(!strcmp(tokens[*idx], ","))
			return get_root(cur);

		else if(!strcmp(tokens[*idx], "(")) //현재 토큰이 열린괄호라면
		{
			if(*idx > 0 && !is_operator(tokens[*idx - 1]) && strcmp(tokens[*idx - 1], ",") != 0){//이전 토큰이 연산자나 ,가 아닐 때
				fstart = true;//first start

				while(1)
				{
					*idx += 1;

					if(!strcmp(tokens[*idx], ")"))
						break;

					new = make_tree(NULL, tokens, idx, parentheses + 1); //괄호 내부를 하위 트리로 만듦

					if(new != NULL){ //하위트리가 생성되었다면
						if(fstart == true){
							cur->child_head = new; //현재 노드의 자식으로 이어주고
							new->parent = cur; //하위트리의 부모노드로 현재 노드 입력

							fstart = false;
						}
						else{ //자식노드가 이미있다면 자식의 형제 노드로 만들어줌
							cur->next = new;
							new->prev = cur;
						}

						cur = new; //현재 노드를 자식노드로 변경
					}

					if(!strcmp(tokens[*idx], ")"))
						break;
				}
			}
			else{ //열린괄호 이전 토큰이 연산자나 ,일때
				*idx += 1;

				new = make_tree(NULL, tokens, idx, parentheses + 1); //괄호 내부는 하위트리로 만듦

				if(cur == NULL) //현재 노드가 루트라면
					cur = new;  //만든 트리를 그대로 가져오고

				else if(!strcmp(new->name, cur->name)){//현재 노드의 이름과 괄호 내부 최상위 노드의 이름이 같을 때
					if(!strcmp(new->name, "|") || !strcmp(new->name, "||") 
							|| !strcmp(new->name, "&") || !strcmp(new->name, "&&"))
					{  //이와 같은 연산자라면
						cur = get_last_child(cur);  //cur의 자식노드와 new자식노드를 형제노드로 연결하기 위해 가장 마지막에 추가된 자식노드를 가져온다

						if(new->child_head != NULL){ //new의 자식 노드와 cur의 자식노드를 형제노드로 연결한다.
							new = new->child_head;

							new->parent->child_head = NULL; //new와 cur가 같은 연산자이므로 중복해서 있을 필요는 없기에 
							new->parent = NULL;//new는 연결을 끊어버린다
							new->prev = cur;
							cur->next = new;
						}
					}
					else if(!strcmp(new->name, "+") || !strcmp(new->name, "*")) 
					{
						i = 0;

						while(1)
						{
							if(!strcmp(tokens[*idx + i], "")) //이후로 다른 토큰이 없을 때 종료
								break;

							if(is_operator(tokens[*idx + i]) && strcmp(tokens[*idx + i], ")") != 0) //다른 연산자나 닫힌 괄호를 만나면 종료
								break;

							i++;
						}

						if(get_precedence(tokens[*idx + i]) < get_precedence(new->name)) //이후의 연산자보다 우선순위가 낮을 경우
						{
							cur = get_last_child(cur); //new를 형제노드로 이어준다
							cur->next = new;
							new->prev = cur;
							cur = new;
						}
						else //new의 우선순위가 더 높거나 같은 경우
						{
							cur = get_last_child(cur); //cur의 자식노드와 new자식노드를 형제노드로 연결하기 위해 가장 마지막에 추가된 자식노드를 가져온다

							if(new->child_head != NULL){ //new의 자식 노드와 cur의 자식노드를 형제노드로 연결한다.
								new = new->child_head;

								new->parent->child_head = NULL; //new와 cur가 같은 연산자이므로 중복해서 있을 필요는 없기에
								new->parent = NULL; //new는 연결을 끊어버린다
								new->prev = cur;
								cur->next = new;
							}
						}
					}
					else{ //new노드를 cur자식노드의 형제노드로 이어준다
						cur = get_last_child(cur); 
						cur->next = new;
						new->prev = cur;
						cur = new;
					}
				}

				else 
				{ //new노드를 cur자식노드의 형제노드로 이어준다
					cur = get_last_child(cur);

					cur->next = new;
					new->prev = cur;

					cur = new;
				}
			}
		}
		else if(is_operator(tokens[*idx])) //현재 토큰이 연산자라면
		{
			if(!strcmp(tokens[*idx], "||") || !strcmp(tokens[*idx], "&&")
					|| !strcmp(tokens[*idx], "|") || !strcmp(tokens[*idx], "&") 
					|| !strcmp(tokens[*idx], "+") || !strcmp(tokens[*idx], "*"))
			{ //해당 연산자일 경우
				if(is_operator(cur->name) == true && !strcmp(cur->name, tokens[*idx]))
					operator = cur;

				else
				{
					new = create_node(tokens[*idx], parentheses); //현재 토큰을 트리 노드로 만듦
					operator = get_most_high_precedence_node(cur, new); //cur기준 우선순위 높은 연산자 노드의 최상위 부모노드를 가져온다

					if(operator->parent == NULL && operator->prev == NULL){  //operator가 최상위 노드라면

						if(get_precedence(operator->name) < get_precedence(new->name)){ //new가 operator보다 낮은 우선순위를 가지면 
							cur = insert_node(operator, new); //operator의 상위 노드로 만듦
						}

						else if(get_precedence(operator->name) > get_precedence(new->name)) //operator가 우선순위가 낮다면
						{
							if(operator->child_head != NULL){ //operator의 자식 노드 중
								operator = get_last_child(operator); //가장 마지막에 추가한 자식노드를
								cur = insert_node(operator, new); //new로 만들어주고 해당자식노드는 new의 자식으로 만듦
							}
						}
						else //operator와 new의 우선순위가 같다면
						{
							operator = cur; //cur에서 탐색하여 

							while(1)
							{
								if(is_operator(operator->name) == true && !strcmp(operator->name, tokens[*idx])) //이름이 같다면 우선순위도 같으므로 종료
									break;

								if(operator->prev != NULL)
									operator = operator->prev; //가장 끝 prev로 이동
								else
									break;
							}

							if(strcmp(operator->name, tokens[*idx]) != 0) //이름이 다르다면 상위노드로
								operator = operator->parent;

							if(operator != NULL){ 
								if(!strcmp(operator->name, tokens[*idx]))
									cur = operator; //이름이 같을 때 cur 변경
							}
						}
					}

					else 
						cur = insert_node(operator, new); //operator의 상위노드로 만듦
				}

			}
			else  //그외 다른 연산자라면
			{
				new = create_node(tokens[*idx], parentheses);  //해당 토큰으로 노드를 만들고

				if(cur == NULL) //아직 만들어진 트리가 없다면
					cur = new;  //new를 트리로

				else
				{
					operator = get_most_high_precedence_node(cur, new); //new보다 우선순위 높은 연산자 노드의 최상위 노드를 가져온다

					if(operator->parentheses > new->parentheses) //operator가 더 내부에 있는 괄호라면 우선순위가 높으므로
						cur = insert_node(operator, new); //new를 상위 노드로 만들어준다

					else if(operator->parent == NULL && operator->prev ==  NULL){  //operator가 최상위 노드라면

						if(get_precedence(operator->name) > get_precedence(new->name)) //new가 우선순위가 높다면
						{
							if(operator->child_head != NULL){ //operator의 자식노드중

								operator = get_last_child(operator); //가장 마지막에 추가한 자식노드자리에
								cur = insert_node(operator, new); //new를 삽입한다
							}
						}

						else //opertor의 연산자 우선순위가 높다면	
							cur = insert_node(operator, new);//new를 operator의 상위 노드로 만듦
					}

					else
						cur = insert_node(operator, new); //operator의 상위노드로 만듦
				}
			}
		}
		else //토큰이 ,나 연산자,괄호가 아닐 때
		{
			new = create_node(tokens[*idx], parentheses); //현재 토큰을 트리 노드로 만듦

			if(cur == NULL) //현재 노드가 루트일 경우
				cur = new; //그대로 복사

			else if(cur->child_head == NULL){ //현재 노드의 자식노드가 없으면 자식 노드로 이어주고
				cur->child_head = new;
				new->parent = cur;

				cur = new; //현재 노드를 자식 노드로 변경
			}
			else{

				cur = get_last_child(cur); //있으면 자식 노드의 형제 노드로 이어준다

				cur->next = new;
				new->prev = cur;

				cur = new; //현재 노드를 자식노드로 변경
			}
		}

		*idx += 1; //다음 토큰 탐색
	} 

	return get_root(cur); //현재 만들어진 트리의 최상위 노드를 리턴
}

node *change_sibling(node *parent) //부모와 연결된 자식노드를 해당 자식노드의 형제노드와 위치를 바꾼다
{
	node *tmp;

	tmp = parent->child_head;  //부모와 연결된 자식노드 임시 저장

	parent->child_head = parent->child_head->next; //부모와 연결된 자식노드를 해당 노드의 형제노드로 변경
	parent->child_head->parent = parent; //부모노드 연결해주기
	parent->child_head->prev = NULL; //형제노드와의 연결 끊기

	parent->child_head->next = tmp; //원래 부모와 연결된 노드는
	parent->child_head->next->prev = parent->child_head; //형제노드로 만들어줌
	parent->child_head->next->next = NULL; //기존에 있던 연결 제거
	parent->child_head->next->parent = NULL;//기존에 있던 연결 제거

	return parent; //부모노드 리턴
}

node *create_node(char *name, int parentheses) //인자로 넘겨준 이름을 가지는 노드 생성
{
	node *new;

	new = (node *)malloc(sizeof(node));
	new->name = (char *)malloc(sizeof(char) * (strlen(name) + 1)); 
	strcpy(new->name, name); //노드 인자 초기화

	new->parentheses = parentheses; //현재 몇 중 괄호 안에 있는지
	new->parent = NULL; //기타 정보 초기화
	new->child_head = NULL;
	new->prev = NULL;
	new->next = NULL;

	return new; //생성한 노드 리턴
}

int get_precedence(char *op) //op의 우선순위 리턴
{
	int i;

	for(i = 2; i < OPERATOR_CNT; i++){
		if(!strcmp(operators[i].operator, op)) //미리 정의된 연산자 배열을 참고하여
			return operators[i].precedence; //해당 op의 우선순위 리턴
	}
	return false;
}

int is_operator(char *op) //op가 연산자면 true 리턴
{
	int i;

	for(i = 0; i < OPERATOR_CNT; i++)
	{
		if(operators[i].operator == NULL)
			break;
		if(!strcmp(operators[i].operator, op)){ //연산자가 맞으면 true
			return true;
		}
	}

	return false; //아니면 false
}

void print(node *cur) //트리 구조를 출력하는 함수
{
	if(cur->child_head != NULL){
		print(cur->child_head); //자식노드는 개행으로
		printf("\n");
	}

	if(cur->next != NULL){
		print(cur->next); //형제노드는 동일선상에 출력
		printf("\t");
	}
	printf("%s", cur->name);
}

node *get_operator(node *cur) //현재 노드의 연산자 노드를 리턴한다
{
	if(cur == NULL) 
		return cur;

	if(cur->prev != NULL)
		while(cur->prev != NULL) //가장 왼쪽의 노드로 이동
			cur = cur->prev;

	return cur->parent; //부모 노드를 출력
}

node *get_root(node *cur) //현재 트리의 루트를 리턴
{
	if(cur == NULL) 
		return cur;

	while(cur->prev != NULL) //부모와 연결된 트리로 이동
		cur = cur->prev;

	if(cur->parent != NULL) //부모 노드가 없을 때까지 재귀호출
		cur = get_root(cur->parent);

	return cur; //현재 트리의 최상위 부모노드를 리턴
}

node *get_high_precedence_node(node *cur, node *new) //new보다 높은 우선순위를 가지는 노드 리턴
{
	if(is_operator(cur->name))
		if(get_precedence(cur->name) < get_precedence(new->name)) //precedence가 낮은게 높은 우선순위를 가진다
			return cur;

	if(cur->prev != NULL){
		while(cur->prev != NULL){ //다른 형제노드가 있다면
			cur = cur->prev;

			return get_high_precedence_node(cur, new); //형제노드들과 우선순위를 비교
		}


		if(cur->parent != NULL) //형제노드 비교가 끝나면 부모노드와 우선순위를 비교
			return get_high_precedence_node(cur->parent, new);
	}

	if(cur->parent == NULL) //cur의 부모노드가 없다면 cur리턴
		return cur;
}

node *get_most_high_precedence_node(node *cur, node *new) //우선순위 높은 노드의 최상위 부모노드 리턴
{
	node *operator = get_high_precedence_node(cur, new); //new보다 높은 우선순위를 가지는 노드를 가져옴
	node *saved_operator = operator;

	while(1)
	{
		if(saved_operator->parent == NULL) //부모노드가 없으면 종료
			break;

		if(saved_operator->prev != NULL) //형제 노드가 있다면
			operator = get_high_precedence_node(saved_operator->prev, new); //형제노드와 비교하여 높은 우선순위를 가지는 노드를 가져옴

		else if(saved_operator->parent != NULL) //형제 노드가 없고 부모노드가 있다면
			operator = get_high_precedence_node(saved_operator->parent, new); // 부모노드와 비교하여 높은 우선순위를 가지는 노드를 가져옴

		saved_operator = operator; //비교할 노드 변경
	}

	return saved_operator;
}

node *insert_node(node *old, node *new) //old자리에 new를 넣고 old를 new의 자식노드로 만듦. 상위 노드가 된 new를 리턴
{
	if(old->prev != NULL){ //old와 연결된 prev가 있을 경우
		new->prev = old->prev; //old와의 연결을 끊고
		old->prev->next = new; //new와 연결한다
		old->prev = NULL;
	}

	new->child_head = old; //old를 new의 자식노드로 연결한다
	old->parent = new;

	return new;
}

node *get_last_child(node *cur) //해당 노드의 자식 중 가장 마지막에 추가한 자식노드를 리턴
{
	if(cur->child_head != NULL) //자식노드가 있을 때 자식노드로 이동하여
		cur = cur->child_head;

	while(cur->next != NULL) //마지막에 추가한 자식노드로 이동
		cur = cur->next;

	return cur;
}

int get_sibling_cnt(node *cur) //cur의 형제노드의 개수를 리턴한다
{
	int i = 0;

	while(cur->prev != NULL) //가장 왼족 노드로 가서
		cur = cur->prev;

	while(cur->next != NULL){ //형제 노드의 개수를 센 후
		cur = cur->next;
		i++;
	}
 
	return i; //리턴
}

void free_node(node *cur) //트리에 할당된 메모리 해제하는 함수
{
	if(cur->child_head != NULL)
		free_node(cur->child_head); //자식노드 재귀호출

	if(cur->next != NULL)
		free_node(cur->next); //형제노드 재귀호출

	if(cur != NULL){ //관련 변수 초기화
		cur->prev = NULL;
		cur->next = NULL;
		cur->parent = NULL;
		cur->child_head = NULL;
		free(cur); //메모리 해제
	}
}


int is_character(char c) //숫자, 알파벳일 경우 1 리턴
{
	return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

int is_typeStatement(char *str) //데이터 타입과 관련된 문자가 있는지 확인
{ //0: 타입이 잘못 써져있음. 1: 타입이 안 써져있음. 2: 타입이 제대로 써져있음
	char *start;
	char str2[BUFLEN] = {0}; 
	char tmp[BUFLEN] = {0}; 
	char tmp2[BUFLEN] = {0}; 
	int i;	 

	start = str; //str원본을 가리키는 start
	strncpy(str2,str,strlen(str));
	remove_space(str2); //str에서 공백이 제거된 str2

	while(start[0] == ' ') //제일 앞부분이 공백이 아닐 때까지 이동
		start += 1; 

	if(strstr(str2, "gcc") != NULL) //만약 gcc 문장이라면
	{
		strncpy(tmp2, start, strlen("gcc")); //start의 가장 첫 부분은 gcc여야한다. 
		if(strcmp(tmp2,"gcc") != 0) //복사후 비교했는데 gcc가 아니면
			return 0; //0리턴
		else
			return 2; //맞으면 2
	}

	for(i = 0; i < DATATYPE_SIZE; i++)
	{
		if(strstr(str2,datatype[i]) != NULL) //특정 데이터 타입인지 검사
		{	
			strncpy(tmp, str2, strlen(datatype[i])); //공백을 지웠으니까 strstr로 검색한 데이터 타입은
			strncpy(tmp2, start, strlen(datatype[i])); //st2 나 start나 같은 위치에 있어야함

			if(strcmp(tmp, datatype[i]) == 0) 
				if(strcmp(tmp, tmp2) != 0) //따라서 비교했는데 같지 않으면
					return 0;  //0
				else
					return 2; //맞으면 2
		}

	}
	return 1;

}

int find_typeSpecifier(char tokens[TOKEN_CNT][MINLEN])  //괄호와 특정 방식으로 관련된 타입지정자의 인덱스 리턴
{
	int i, j;

	for(i = 0; i < TOKEN_CNT; i++)
	{
		for(j = 0; j < DATATYPE_SIZE; j++)
		{
			if(strstr(tokens[i], datatype[j]) != NULL && i > 0) //토큰에서 type문장이 있을 때
			{
				if(!strcmp(tokens[i - 1], "(") && !strcmp(tokens[i + 1], ")") //해당 토큰 앞뒤로 괄호가 있고
						&& (tokens[i + 2][0] == '&' || tokens[i + 2][0] == '*' 
							|| tokens[i + 2][0] == ')' || tokens[i + 2][0] == '(' 
							|| tokens[i + 2][0] == '-' || tokens[i + 2][0] == '+' 
							|| is_character(tokens[i + 2][0]))) //괄호 이후 토큰이 문자열이거나 해당 기호이면
					return i; //토큰 위치 리턴
			}
		}
	}
	return -1;
}

int find_typeSpecifier2(char tokens[TOKEN_CNT][MINLEN]) //struct 타입지정자 인덱스 리턴 
{
	int i, j;


	for(i = 0; i < TOKEN_CNT; i++)
	{
		for(j = 0; j < DATATYPE_SIZE; j++)
		{
			if(!strcmp(tokens[i], "struct") && (i+1) <= TOKEN_CNT && is_character(tokens[i + 1][strlen(tokens[i + 1]) - 1])) //struct 다음 토큰이 숫자 또는 알파벳으로 끝날 경우 
				return i; //해당 토큰 위치 리턴
		}
	}
	return -1;
}

int all_star(char *str) //전부 *이면 1, 하나라도 아니면 0을 리턴
{
	int i;
	int length= strlen(str);

	if(length == 0)	//길이가 0일 때도 0 리턴
		return 0;

	for(i = 0; i < length; i++)
		if(str[i] != '*')
			return 0;
	return 1;

}

int all_character(char *str) //숫자 , 알파벳이 하나라도 있으면 1 리턴
{
	int i;

	for(i = 0; i < strlen(str); i++)
		if(is_character(str[i]))
			return 1;
	return 0; //하나도 없으면 0

}

int reset_tokens(int start, char tokens[TOKEN_CNT][MINLEN]) 
{ //find_typeSpecifier1,2에 의해 탐색된, 잘못 분리된 토큰을 합치는 함수, 실패시 0리턴
	int i;
	int j = start - 1;
	int lcount = 0, rcount = 0;
	int sub_lcount = 0, sub_rcount = 0;

	if(start > -1){ //정상적인 인덱스일 때만
		if(!strcmp(tokens[start], "struct")) { //잘못 분리된 토큰이 struct일 경우
			strcat(tokens[start], " "); //공백을 추가하여
			strcat(tokens[start], tokens[start+1]);	//다음 토큰하고 합친다     

			for(i = start + 1; i < TOKEN_CNT - 1; i++){ //다음 토큰부터 하나씩 앞으로 당김
				strcpy(tokens[i], tokens[i + 1]);
				memset(tokens[i + 1], 0, sizeof(tokens[0]));
			}
		}

		else if(!strcmp(tokens[start], "unsigned") && strcmp(tokens[start+1], ")") != 0) { 
			//잘못 분리된 토큰이 unsinged일 경우, 다음 토큰이 괄호가 아니라면 잘못 분리된게 맞으니 합쳐줌	
			strcat(tokens[start], " "); 
			strcat(tokens[start], tokens[start + 1]);	     
			strcat(tokens[start], tokens[start + 2]); //변수명까지 합쳐준다

			memset(tokens[i + 1], 0, sizeof(tokens[0])); //이후 토큰을 2칸씩 앞으로 당김
			for(i = start + 1; i < TOKEN_CNT - 2; i++){
				strcpy(tokens[i], tokens[i + 2]);
				memset(tokens[i + 2], 0, sizeof(tokens[0]));
			}
		}

		j = start + 1;           
		while(!strcmp(tokens[j], ")")){ //연속으로 )가 있을 때 반복
			rcount ++; //오른쪽 닫힌괄호의 개수 카운트
			if(j==TOKEN_CNT)
				break;
			j++;
		}

		j = start - 1;
		while(!strcmp(tokens[j], "(")){ //연속으로 (가 있을 때 반복
			lcount ++; //왼쪽 열린괄호의 개수 카운트
			if(j == 0)
				break;
			j--;
		}
		if( (j!=0 && is_character(tokens[j][strlen(tokens[j])-1]) ) || j==0) 
			//괄호 바로 왼쪽이 기호가 아닌 문자이거나 j가 0일 땐 lcout, rcount가 같지 않을 수 있다.
			lcount = rcount;

		if(lcount != rcount ) //개수가 맞지 않으면 에러
			return false;

		if( (start - lcount) >0 && !strcmp(tokens[start - lcount - 1], "sizeof")){ //sizeof구문이었을 경우
			return true; 
		}

		else if((!strcmp(tokens[start], "unsigned") || !strcmp(tokens[start], "struct")) && strcmp(tokens[start+1], ")")) {	//unsinged구문과 struct구문이 괄호 안에 있을 경우	
			strcat(tokens[start - lcount], tokens[start]); //괄호포함 해서
			strcat(tokens[start - lcount], tokens[start + 1]); //하나의 토큰으로 만든다.
			strcpy(tokens[start - lcount + 1], tokens[start + rcount]);

			for(int i = start - lcount + 1; i < TOKEN_CNT - lcount -rcount; i++) {//앞으로 당겨오는 과정
				strcpy(tokens[i], tokens[i + lcount + rcount]);
				memset(tokens[i + lcount + rcount], 0, sizeof(tokens[0]));
			}


		}
		else{
			if(tokens[start + 2][0] == '('){ //오른쪽에 또다른 괄호가 있다면
				j = start + 2;
				while(!strcmp(tokens[j], "(")){
					sub_lcount++;
					j++;
				} 	
				if(!strcmp(tokens[j + 1],")")){ //토큰 하나이후 바로 닫힌 괄호가 있어야한다.
					j = j + 1;
					while(!strcmp(tokens[j], ")")){
						sub_rcount++;
						j++;
					}
				}
				else 
					return false;

				if(sub_lcount != sub_rcount) //열린,닫힌 괄호 수가 맞지 않으면 에러
					return false;

				strcpy(tokens[start + 2], tokens[start + 2 + sub_lcount]);	
				for(int i = start + 3; i<TOKEN_CNT; i++)
					memset(tokens[i], 0, sizeof(tokens[0]));

			}
			strcat(tokens[start - lcount], tokens[start]); //양옆 연속된 모든 괄호를 포함하여
			strcat(tokens[start - lcount], tokens[start + 1]);//하나의 토큰으로 만든다.
			strcat(tokens[start - lcount], tokens[start + rcount + 1]);

			for(int i = start - lcount + 1; i < TOKEN_CNT - lcount -rcount -1; i++) {//앞으로 당겨오는 과정
				strcpy(tokens[i], tokens[i + lcount + rcount +1]);
				memset(tokens[i + lcount + rcount + 1], 0, sizeof(tokens[0]));

			}
		}
	}
	return true;
}

void clear_tokens(char tokens[TOKEN_CNT][MINLEN]) //토큰변수 0으로 초기화
{
	int i;

	for(i = 0; i < TOKEN_CNT; i++)
		memset(tokens[i], 0, sizeof(tokens[i]));
}

char *rtrim(char *_str) //문자열의 오른쪽에 공백 제거
{
	char tmp[BUFLEN];
	char *end;

	strcpy(tmp, _str);
	end = tmp + strlen(tmp) - 1; //tmp의 마지막 인덱스
	while(end != _str && isspace(*end)) //공백이 아닐 때까지 오른쪽으로 포인터 이동
		--end;

	*(end + 1) = '\0'; //문자열 자름
	_str = tmp;
	return _str;
}

char *ltrim(char *_str) //문자열의 왼쪽 공백 제거
{
	char *start = _str;

	while(*start != '\0' && isspace(*start)) //공백이 아닐 때까지 왼쪽으로 포인터 이동
		++start;
	_str = start;
	return _str;
}

char* remove_extraspace(char *str) //연속되는 공백을 최대 1개로 줄인 문자열 리턴
{
	int i;
	char *str2 = (char*)malloc(sizeof(char) * BUFLEN);
	char *start, *end;
	char temp[BUFLEN] = "";
	int position;

	if(strstr(str,"include<")!=NULL){ //"include<" 가 있을 경우 "include <"로 만듦
		start = str;
		end = strpbrk(str, "<");
		position = end - start; //str에서 <의 인덱스를 가져옴 

		strncat(temp, str, position);// <전까지 복사 후
		strncat(temp, " ", 1); //공백을 넣고
		strncat(temp, str + position, strlen(str) - position + 1);//나머지를 복사한다

		str = temp;		
	}

	for(i = 0; i < strlen(str); i++)
	{
		if(str[i] ==' ') //현재 문자가 공백일 때
		{
			if(i == 0 && str[0] ==' ') //왼쪽 공백이라면
				while(str[i + 1] == ' ') //연속된 공백 개수만큼 스킵
					i++;	
			else{ 
				if(i > 0 && str[i - 1] != ' ') //이전 문자가 공백이 아니라면
					str2[strlen(str2)] = str[i]; //공백 하나 넣어
				while(str[i + 1] == ' ') //그 이후 공백은 스킵
					i++;
			} 
		}
		else //일반 문자는 그대로 넣는다.
			str2[strlen(str2)] = str[i];
	}

	return str2;
}



void remove_space(char *str) //모든 공백 제거
{
	char* i = str;
	char* j = str;

	while(*j != 0) //널문자가 아닐 때 까지
	{
		*i = *j++; //i는 공백이 아닐 때만 옆으로 한 칸씩 이동하여
		if(*i != ' ') //공백일 경우 자동으로 덮어쓰기가 되면서
			i++; //공백이 지워진다.
	}
	*i = 0; //마지막에 널문자로 마무리
}

int check_brackets(char *str) //괄호가 정상인지 체크
{
	char *start = str;
	int lcount = 0, rcount = 0;

	while(1){ //근데 이거 잘못 만든거 아닌가? 횟수 체크해서 홀수, 짝수 cnt일 때 ( ) 탐색되야 할텐데 흠....
		if((start = strpbrk(start, "()")) != NULL){ //두 번째 인수에 적힌 문자가 포함된 위치 포인터 반환
			if(*(start) == '(')
				lcount++;
			else
				rcount++;

			start += 1; //찾은 문자 다음 부터 검색하기 위해 		
		}
		else
			break;
	}

	if(lcount != rcount) //괄호가 맞지 않으면 에러
		return 0;
	else 
		return 1;
}

int get_token_cnt(char tokens[TOKEN_CNT][MINLEN]) //토큰의 개수를 리턴한다
{
	int i;

	for(i = 0; i < TOKEN_CNT; i++)
		if(!strcmp(tokens[i], "")) //다음 토큰이 없을 때까지 탐색
			break;

	return i;
}
