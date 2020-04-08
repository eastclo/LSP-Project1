#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "ssu_score.h"
#include "blank.h"
#define DEBUG 1

extern struct ssu_scoreTable score_table[QNUM];
extern char id_table[SNUM][10];

struct ssu_scoreTable score_table[QNUM];
char id_table[SNUM][10];

char stuDir[BUFLEN]; //학생폴더
char ansDir[BUFLEN]; //정답폴더
char errorDir[BUFLEN];
char threadFiles[ARGNUM][FILELEN];
char cIDs[ARGNUM][FILELEN];

int eOption = false;
int tOption = false;
int pOption = false;
int cOption = false;

void ssu_score(int argc, char *argv[]) //사실상 메인함수
{
	char saved_path[BUFLEN];
	int i;

	for(i = 0; i < argc; i++){
		if(!strcmp(argv[i], "-h")){	//if there is '-h' option, print manual and exit ssu_score.
			print_usage();
			return;
		}
	}

	memset(saved_path, 0, BUFLEN);	//initialize local parameter
//	if(argc >= 3 && strcmp(argv[1], "-c") != 0){	//c옵션 아니고 인자 3개이상
	/******TODO EXCEPTION*******/
	strcpy(stuDir, argv[1]);
	strcpy(ansDir, argv[2]);
//	}

//	if(!check_option(argc, argv))	//if it is out of form, throw exception.
//		exit(1);
/*
	if(!eOption && !tOption && !pOption && cOption){
		do_cOption(cIDs);
		return;
	}
*/
	/*******Initialize parameter : stuDir, ansDir, saved_path**********/
	getcwd(saved_path, BUFLEN);	//get current working space
	if(chdir(stuDir) < 0){	//change directory
		fprintf(stderr, "%s doesn't exist\n", stuDir);
		return;
	}
	getcwd(stuDir, BUFLEN);	

	chdir(saved_path);
	if(chdir(ansDir) < 0){
		fprintf(stderr, "%s doesn't exist\n", ansDir);
		return;
	}
	getcwd(ansDir, BUFLEN);

	chdir(saved_path);
	////////////////////////////////////////////////////////////////////

	set_scoreTable(saved_path);	//set score table
	set_idTable();	//set scoring result table

	printf("grading student's test papers..\n");
	score_students();	//calculate score

	/*
	if(cOption)
		do_cOption(cIDs);
*/
	return;
}

int check_option(int argc, char *argv[])
{
	int i, j;
	int c;

	while((c = getopt(argc, argv, "e:thpc")) != -1)
	{
		switch(c){
			case 'e':
				eOption = true;
				strcpy(errorDir, optarg);

				if(access(errorDir, F_OK) < 0)
					mkdir(errorDir, 0755);
				else{
					rmdirs(errorDir);
					mkdir(errorDir, 0755);
				}
				break;
			case 't':
				tOption = true;
				i = optind;
				j = 0;

				while(i < argc && argv[i][0] != '-'){

					if(j >= ARGNUM)
						printf("Maximum Number of Argument Exceeded.  :: %s\n", argv[i]);
					else
						strcpy(threadFiles[j], argv[i]);
					i++; 
					j++;
				}
				break;
			case 'p':
				pOption = true;
				break;
			case 'c':
				cOption = true;
				i = optind;
				j = 0;

				while(i < argc && argv[i][0] != '-'){

					if(j >= ARGNUM)
						printf("Maximum Number of Argument Exceeded.  :: %s\n", argv[i]);
					else
						strcpy(cIDs[j], argv[i]);
					i++; 
					j++;
				}
				break;
			case '?':
				printf("Unkown option %c\n", optopt);
				return false;
		}
	}

	return true;
}


void do_cOption(char (*ids)[FILELEN])
{
	FILE *fp;
	char tmp[BUFLEN];
	int i = 0;
	char *p, *saved;

	if((fp = fopen("score.csv", "r")) == NULL){
		fprintf(stderr, "file open error for score.csv\n");
		return;
	}

	fscanf(fp, "%s\n", tmp);

	while(fscanf(fp, "%s\n", tmp) != EOF)
	{
		p = strtok(tmp, ",");

		if(!is_exist(ids, tmp))
			continue;

		printf("%s's score : ", tmp);

		while((p = strtok(NULL, ",")) != NULL)
			saved = p;

		printf("%s\n", saved);
	}
	fclose(fp);
}

int is_exist(char (*src)[FILELEN], char *target)
{
	int i = 0;

	while(1)
	{
		if(i >= ARGNUM)
			return false;
		else if(!strcmp(src[i], ""))
			return false;
		else if(!strcmp(src[i++], target))
			return true;
	}
	return false;
}

void set_scoreTable(char *curDir) //set score table
{
	char filename[FILELEN];

	sprintf(filename, "%s/%s", curDir, "score_table.csv");

	if(access(filename, F_OK) == 0) //if there is the accessible file already exist.
		read_scoreTable(filename);
	else{
		make_scoreTable(curDir);
		write_scoreTable(filename);
	}
}

void read_scoreTable(char *path) //score_table.csv에 적힌 문제별 점수를 score_table 배열에 저장 
{
	FILE *fp;
	char qname[FILELEN];
	char score[BUFLEN];
	int idx = 0;

	if((fp = fopen(path, "r")) == NULL){ //file open readonly option.
		fprintf(stderr, "file open error for %s\n", path);
		return ;
	}

	while(fscanf(fp, "%[^,],%s\n", qname, score) != EOF){
		strcpy(score_table[idx].qname, qname); //save question name
		score_table[idx++].score = atof(score); //save score
	}

	fclose(fp);
}

void make_scoreTable(char *curDir) //score_table 배열에 문제별 점수를 저장 
{
	int type, num;
	double score, bscore, pscore;
	struct dirent *dirp, *c_dirpl;
/*	{
		long d_ino	//i-node number
		off_t d_off //offset
		unsigned short d_reclen //length of file name
		char d_name[NAME_MAX+1] //file name
	 }  */
	DIR *dp, *c_dp;	//directory stream pointer
	char tmp[BUFLEN];
	int idx = 0;
	int i;

	num = get_create_type(); //각 문제마다 점수를 책정할지, 빈칸/프로그램로 점수를 책정할지 결정. 

	if(num == 1)
	{
		printf("Input value of blank question : ");
		scanf("%lf", &bscore);
		printf("Input value of program question : ");
		scanf("%lf", &pscore);
	}


	if((dp = opendir(ansDir)) == NULL){	//get directory stream pointer
		fprintf(stderr, "open dir error for %s\n", ansDir);
		return;
	}	

	while((dirp = readdir(dp)) != NULL) //디렉토리 내 파일 목록을 순회함.
	{
		if(!strcmp(dirp->d_name, ".") || !strcmp(dirp->d_name, ".."))
			continue;

		strcpy(score_table[idx++].qname, dirp->d_name);
	}

	closedir(dp);
	sort_scoreTable(idx);	//문제 번호순서대로 정렬

	for(i = 0; i < idx; i++)
	{
		type = get_file_type(score_table[i].qname);

		if(num == 1)
		{
			if(type == TEXTFILE)	//텍스트 파일이면 빈칸 문제
				score = bscore;
			else if(type == CFILE) //c파일이면 프로그램 문제
				score = pscore;
		}
		else if(num == 2)
		{
			printf("Input of %s: ", score_table[i].qname);
			scanf("%lf", &score);
		}

		score_table[i].score = score;
	}
}

void write_scoreTable(char *filename) //score_table.csv 생성
{
	int fd;
	char tmp[BUFLEN];
	int i;
	int num = sizeof(score_table) / sizeof(score_table[0]);

	if((fd = creat(filename, 0666)) < 0){ //score_table.csv 생성
		fprintf(stderr, "creat error for %s\n", filename);
		return;
	}

	for(i = 0; i < num; i++)
	{
		sprintf(tmp, "%s,%.2f\n", score_table[i].qname, score_table[i].score); //문제, 점수 작성
		write(fd, tmp, strlen(tmp));
	}

	close(fd);
}


void set_idTable() //id_table변수에 학번 저장
{
	struct stat statbuf;
	struct dirent *dirp;
	DIR *dp;
	char tmp[BUFLEN];
	int num = 0;

	if((dp = opendir(stuDir)) == NULL){
		fprintf(stderr, "opendir error for %s\n", stuDir);
		exit(1);
	}

	while((dirp = readdir(dp)) != NULL){
		if(!strcmp(dirp->d_name, ".") || !strcmp(dirp->d_name, ".."))
			continue;

		sprintf(tmp, "%s/%s", stuDir, dirp->d_name); //학번 디렉토리 경로 저장
		stat(tmp, &statbuf); //statbuf에 해당 파일의 정보를 복사

		if(S_ISDIR(statbuf.st_mode)) //해당 폴더가 디렉토리면
			strcpy(id_table[num++], dirp->d_name); //id_table에 학번 저장
		else
			continue;
	}

	sort_idTable(num); 
}

void sort_idTable(int size) //학번 오름차순으로 정렬
{
	int i, j;
	char tmp[10];

	//버블 정렬
	for(i = 0; i < size - 1; i++){
		for(j = 0; j < size - 1 -i; j++){
			if(strcmp(id_table[j], id_table[j+1]) > 0){ //오름차순
				strcpy(tmp, id_table[j]);
				strcpy(id_table[j], id_table[j+1]);
				strcpy(id_table[j+1], tmp);
			}
		}
	}
}

void sort_scoreTable(int size) //문제 번호 오름차순으로 정렬
{
	int i, j;
	struct ssu_scoreTable tmp;
	int num1_1, num1_2;
	int num2_1, num2_2;

	//버블정렬
	for(i = 0; i < size - 1; i++){
		for(j = 0; j < size - 1 - i; j++){

			get_qname_number(score_table[j].qname, &num1_1, &num1_2);
			get_qname_number(score_table[j+1].qname, &num2_1, &num2_2);


			if((num1_1 > num2_1) || ((num1_1 == num2_1) && (num1_2 > num2_2))){
				//j와 j+1번째 위치 변경.
				memcpy(&tmp, &score_table[j], sizeof(score_table[0]));
				memcpy(&score_table[j], &score_table[j+1], sizeof(score_table[0]));
				memcpy(&score_table[j+1], &tmp, sizeof(score_table[0]));
			}
		}
	}
}

void get_qname_number(char *qname, int *num1, int *num2) //문제 번호 리턴
{
	char *p;
	char dup[FILELEN];

	strncpy(dup, qname, strlen(qname)); //문자열 길이만큼만 복사(strcpy를 쓰면 FILELEN만큼 복사하여 효율x)
	*num1 = atoi(strtok(dup, "-.")); //"-."기준으로 문자열 dup 분리.
	
	p = strtok(NULL, "-."); //NULL이 들어오면 이전에 분리된 부분 부터 문자열 분리.
	if(p == NULL)
		*num2 = 0;
	else
		*num2 = atoi(p);
}

int get_create_type() //점수 테이블 생성 질문
{
	int num;

	while(1)
	{
		printf("score_table.csv file doesn't exist!\n");
		printf("1. input blank question and program question's score. ex) 0.5 1\n");
		printf("2. input all question's score. ex) Input value of 1-1: 0.1\n");
		printf("select type >> ");
		scanf("%d", &num);

		if(num != 1 && num != 2)
			printf("not correct number!\n");
		else
			break;
	}

	return num;
}

void score_students() //학생 점수 채점(채점 결과 테이블 작성)
{
	double score = 0;
	int num;
	int fd;
	char tmp[BUFLEN];
	int size = sizeof(id_table) / sizeof(id_table[0]); //SNUM==100

	if((fd = creat("score.csv", 0666)) < 0){ //채점 결과 파일 생성 
		fprintf(stderr, "creat error for score.csv");
		return;
	}
	write_first_row(fd); //채점 결과 테이블의 첫 행 채우기 

	for(num = 0; num < size; num++)
	{
		if(!strcmp(id_table[num], "")) //학생 입력 끝
			break;

		sprintf(tmp, "%s,", id_table[num]); //해당 학생의 학번
		write(fd, tmp, strlen(tmp)); //테이블의 첫 열에 입력

		score += score_student(fd, id_table[num]); //학생별 점수 계산 
	}

	printf("Total average : %.2f\n", score / num); //마지막에 학생들의 점수 평균 출력

	close(fd);
}

double score_student(int fd, char *id) //학생별 점수 계산, 해당 학생 총 점수 리턴
{
	int type;
	double result;
	double score = 0;
	int i;
	char tmp[BUFLEN];
	int size = sizeof(score_table) / sizeof(score_table[0]); //QNUM==100

	for(i = 0; i < size ; i++)
	{
		if(score_table[i].score == 0) //문제 끝
			break;

		sprintf(tmp, "%s/%s/%s", stuDir, id, score_table[i].qname); //문제별로 탐색하기 위한 경로지정

		if(access(tmp, F_OK) < 0) //해당 파일이 존재하는지 체크
			result = false; //파일이 없으면 해당 문제를 제출하지 않은 것
		else
		{
			if((type = get_file_type(score_table[i].qname)) < 0) //txt인지 c인지 검사
				continue;
			
			if(type == TEXTFILE)
				result = score_blank(id, score_table[i].qname); //txt파일이면 빈칸문제 채점
			else if(type == CFILE)
				result = score_program(id, score_table[i].qname); //c파일이면 프로그램문제 채점
		}

		if(result == false) //0점
			write(fd, "0,", 2);
		else{
			if(result == true){ //채점 점수 입력
				score += score_table[i].score; 
				sprintf(tmp, "%.2f,", score_table[i].score);
			}
			else if(result < 0){ //감점 처리(warning시 -0.1)
				score = score + score_table[i].score + result;
				sprintf(tmp, "%.2f,", score_table[i].score + result);
			}
			write(fd, tmp, strlen(tmp));
		}
	}

	sprintf(tmp, "%.2f\n", score); //마지막에 총점 입력
	write(fd, tmp, strlen(tmp));
	printf("%s is finished.. score : %.2lf\n", id, score); //학생 점수 출력 

	return score;
}

void write_first_row(int fd) //채점 결과 테이블의 첫 번째 행 채우기
{
	int i;
	char tmp[BUFLEN];
	int size = sizeof(score_table) / sizeof(score_table[0]); //QNUM==100 

	write(fd, ",", 1); //첫 번째 열은 학번일 들어가고 두 번째열부터 문제와 점수가 기록된다.

	for(i = 0; i < size; i++){
		if(score_table[i].score == 0) //문제 입력 끝
			break;
		
		sprintf(tmp, "%s,", score_table[i].qname);
		write(fd, tmp, strlen(tmp)); //문제 이름 입력
	}
	write(fd, "sum\n", 4); //마지막 열은 총점
}

char *get_answer(int fd, char *result)
{
	char c;
	int idx = 0;

	memset(result, 0, BUFLEN);
	while(read(fd, &c, 1) > 0)
	{
		if(c == ':')
			break;
		
		result[idx++] = c;
	}
	if(result[strlen(result) - 1] == '\n')
		result[strlen(result) - 1] = '\0';

	return result;
}

int score_blank(char *id, char *filename)
{
	char tokens[TOKEN_CNT][MINLEN];
	node *std_root = NULL, *ans_root = NULL;
	int idx, start;
	char tmp[BUFLEN];
	char s_answer[BUFLEN], a_answer[BUFLEN];
	char qname[FILELEN];
	int fd_std, fd_ans;
	int result = true;
	int has_semicolon = false;

	memset(qname, 0, sizeof(qname));
	memcpy(qname, filename, strlen(filename) - strlen(strrchr(filename, '.')));

	sprintf(tmp, "%s/%s/%s", stuDir, id, filename);
	fd_std = open(tmp, O_RDONLY);
	strcpy(s_answer, get_answer(fd_std, s_answer));

	if(!strcmp(s_answer, "")){
		close(fd_std);
		return false;
	}

	if(!check_brackets(s_answer)){
		close(fd_std);
		return false;
	}

	strcpy(s_answer, ltrim(rtrim(s_answer)));

	if(s_answer[strlen(s_answer) - 1] == ';'){
		has_semicolon = true;
		s_answer[strlen(s_answer) - 1] = '\0';
	}

	if(!make_tokens(s_answer, tokens)){
		close(fd_std);
		return false;
	}

	idx = 0;
	std_root = make_tree(std_root, tokens, &idx, 0);

	sprintf(tmp, "%s/%s/%s", ansDir, qname, filename);
	fd_ans = open(tmp, O_RDONLY);

	while(1)
	{
		ans_root = NULL;
		result = true;

		for(idx = 0; idx < TOKEN_CNT; idx++)
			memset(tokens[idx], 0, sizeof(tokens[idx]));

		strcpy(a_answer, get_answer(fd_ans, a_answer));

		if(!strcmp(a_answer, ""))
			break;

		strcpy(a_answer, ltrim(rtrim(a_answer)));

		if(has_semicolon == false){
			if(a_answer[strlen(a_answer) -1] == ';')
				continue;
		}

		else if(has_semicolon == true)
		{
			if(a_answer[strlen(a_answer) - 1] != ';')
				continue;
			else
				a_answer[strlen(a_answer) - 1] = '\0';
		}

		if(!make_tokens(a_answer, tokens))
			continue;

		idx = 0;
		ans_root = make_tree(ans_root, tokens, &idx, 0);

		compare_tree(std_root, ans_root, &result);

		if(result == true){
			close(fd_std);
			close(fd_ans);

			if(std_root != NULL)
				free_node(std_root);
			if(ans_root != NULL)
				free_node(ans_root);
			return true;

		}
	}
	
	close(fd_std);
	close(fd_ans);

	if(std_root != NULL)
		free_node(std_root);
	if(ans_root != NULL)
		free_node(ans_root);

	return false;
}

double score_program(char *id, char *filename) //TODO
{
	//컴파일 시 warning은 -0.1점, 에러는 0점
	double compile;
	int result;

	compile = compile_program(id, filename); //문제 컴파일하여 실행파일 생성

	if(compile == ERROR || compile == false) //컴파일 안 되는거 예외처리
		return false;
	
	result = execute_program(id, filename); //파일 실행해서 결과 리턴

	if(!result)
		return false;

	if(compile < 0) //컴파일시 warning은 -0.1점
		return compile; 

	return true;
}

int is_thread(char *qname) //-t옵션으로 -lpthread 옵션을 추가할 문제인지 검사
{
	int i;
	int size = sizeof(threadFiles) / sizeof(threadFiles[0]);

	for(i = 0; i < size; i++){
		if(!strcmp(threadFiles[i], qname))
			return true;
	}
	return false;
}

double compile_program(char *id, char *filename) //프로그램 문제 컴파일
{
	int fd;
	char tmp_f[BUFLEN], tmp_e[BUFLEN];
	char command[BUFLEN];
	char qname[FILELEN];
	int isthread;
	off_t size;
	double result;

	memset(qname, 0, sizeof(qname)); //qname초기화
	memcpy(qname, filename, strlen(filename) - strlen(strrchr(filename, '.'))); //확장자를 제거한 문제번호 만큼만 qname에 저장
	
	isthread = is_thread(qname); //t옵션 체크

	sprintf(tmp_f, "%s/%s", ansDir, filename); //정답폴더/문제번호.c
	sprintf(tmp_e, "%s/%s.exe", ansDir, qname); //정답폴더/문제번호.exe

	if(tOption && isthread) //t옵션시 -lpthread옵션 추가하여 컴파일
		sprintf(command, "gcc -o %s %s -lpthread", tmp_e, tmp_f);
	else
		sprintf(command, "gcc -o %s %s", tmp_e, tmp_f); //컴파일해서 문제번호.exe파일 생성

	sprintf(tmp_e, "%s/%s/%s_error.txt", ansDir, qname, qname);
	fd = creat(tmp_e, 0666);

	redirection(command, fd, STDERR); //에러메시지 파일로 저장
	size = lseek(fd, 0, SEEK_END);
	close(fd);
	unlink(tmp_e); //정답파일은 에러가 없으므로 삭제
	if(size > 0) //정답파일 에러시 파일이 잘못됨. 채점하지 않고 종료
		return false;

	sprintf(tmp_f, "%s/%s/%s", stuDir, id, filename); //학생폴더/문제번호.c
	sprintf(tmp_e, "%s/%s/%s.stdexe", stuDir, id, qname); //학생폴더/문제번호.stdexe

	if(tOption && isthread)
		sprintf(command, "gcc -o %s %s -lpthread", tmp_e, tmp_f);
	else
		sprintf(command, "gcc -o %s %s", tmp_e, tmp_f); //학생이 푼 문제 컴파일

	sprintf(tmp_f, "%s/%s/%s_error.txt", stuDir, id, qname);
	fd = creat(tmp_f, 0666);

	redirection(command, fd, STDERR); //에러메시지 파일로 저장
	size = lseek(fd, 0, SEEK_END);
	close(fd);

	if(size > 0){ //에러메시지가 있을 경우
		if(eOption)
		{
			sprintf(tmp_e, "%s/%s", errorDir, id); //에러메시지를 저장할 에러폴더가 접근 가능하면
			if(access(tmp_e, F_OK) < 0)
				mkdir(tmp_e, 0755);

			sprintf(tmp_e, "%s/%s/%s_error.txt", errorDir, id, qname);
			rename(tmp_f, tmp_e); //임시로 만든 에러폴더를 그대로 가져옴 

			result = check_error_warning(tmp_e); //error인지 warning인지 확인
		}
		else{ 
			result = check_error_warning(tmp_f); //error인지 warning인지 확인 
			unlink(tmp_f);
		}

		return result; //error 또는 warning결과 리턴
	}

	unlink(tmp_f); //e옵션 없을 경우 에러메시지 파일 삭제
	return true;
}

double check_error_warning(char *filename) //에러면 0점, warning이면 감점된 점수 리턴
{
	FILE *fp;
	char tmp[BUFLEN];
	double warning = 0;

	if((fp = fopen(filename, "r")) == NULL){
		fprintf(stderr, "fopen error for %s\n", filename);
		return false;  //에러가 있으나 열리지 않음
	}

	while(fscanf(fp, "%s", tmp) > 0){
		if(strstr(tmp, "error:")!=NULL)
			return ERROR; //error가 있으면 0리턴
		else if(strstr(tmp, "warning:")!=NULL)
			warning += WARNING; //warning당 0.1점 감점
	}

	return warning;
}

int execute_program(char *id, char *filename) //학생답안과 정답을 실행하는 함수
{
	char std_fname[BUFLEN], ans_fname[BUFLEN]; //실행 결과가 기록될 파일
	char tmp[BUFLEN];
	char qname[FILELEN];
	time_t start, end;
	pid_t pid;
	int fd;

	memset(qname, 0, sizeof(qname));
	memcpy(qname, filename, strlen(filename) - strlen(strrchr(filename, '.'))); //확장자를 제거한 문제 이름을 저장

	sprintf(ans_fname, "%s/%s.stdout", ansDir, qname); //정답 실행결과를 저장할 파일 생성
	fd = creat(ans_fname, 0666);

	sprintf(tmp, "%s/%s.exe", ansDir, qname); //정답 실행결과를 ans_fname에 저장
	redirection(tmp, fd, STDOUT); //표준출력을  ans_fname에 출력하도록 변경
	close(fd);

	sprintf(std_fname, "%s/%s/%s.stdout", stuDir, id, qname); //학생 정답 실행 결과를 저장할 파일 생성
	fd = creat(std_fname, 0666);

	sprintf(tmp, "%s/%s/%s.stdexe &", stuDir, id, qname); //백그라운드 프로세스로 학생 정답파일 실행

	start = time(NULL); //프로그램 실행 시작시간 기록 
	redirection(tmp, fd, STDOUT); //실행하여 std_fname에 결과 저장
	
	sprintf(tmp, "%s.stdexe", qname); 
	while((pid = inBackground(tmp)) > 0){ //프로세스가 계속 실행 중 인지 체크
		end = time(NULL);

		if(difftime(end, start) > OVER){ //실행시간이 5초가 넘어가면
			kill(pid, SIGKILL); //해당 프로세스 종료 후
			close(fd);
			return false; //0점 리턴
		}
	}

	close(fd);

	return compare_resultfile(std_fname, ans_fname); //정답과 학생답안 비교
}

pid_t inBackground(char *name) //name실행파일이 백그라운드에서 아직 실행 중인지 확인
{
	pid_t pid;
	char command[64];
	char tmp[64];
	int fd;
	off_t size;
	
	memset(tmp, 0, sizeof(tmp));
	fd = open("background.txt", O_RDWR | O_CREAT | O_TRUNC, 0666); //ps | grep 명령어 결과 저장할 임시 파일

	sprintf(command, "ps | grep %s", name);
	redirection(command, fd, STDOUT); //파일에 명령어 결과 저장

	lseek(fd, 0, SEEK_SET);
	read(fd, tmp, sizeof(tmp));

	if(!strcmp(tmp, "")){ //프로세스가 실행중이지 않으면
		unlink("background.txt"); //임시 파일 삭제후
		close(fd);
		return 0; //0이하 값 리턴하면 된다
	}

	pid = atoi(strtok(tmp, " ")); //ps명령어는 "PID ~~"이므로 공백을 기준으로 쪼개면 pid가 나온다
	close(fd);

	unlink("background.txt");
	return pid; //프로세스 번호 리턴
}

int compare_resultfile(char *file1, char *file2) //학생 답안과 정답 결과 비교하여 채점
{
	int fd1, fd2;
	char c1, c2;
	int len1, len2;

	fd1 = open(file1, O_RDONLY); //학생 답안 실행 결과
	fd2 = open(file2, O_RDONLY); //정답 실행 결과

	while(1) //대소문자와 공백 구분하지 않고 채점
	{
		while((len1 = read(fd1, &c1, 1)) > 0){
			if(c1 == ' ')  //공백이면 패스
				continue;
			else 
				break;
		}
		while((len2 = read(fd2, &c2, 1)) > 0){
			if(c2 == ' ')  //공백이면 패스
				continue;
			else 
				break;
		}
		
		if(len1 == 0 && len2 == 0) //둘 다 더이상 읽을게 없으면 루프 종료 후 true리턴
			break;

		to_lower_case(&c1); //대소문자 구분하지 않기 위해
		to_lower_case(&c2); //소문자로 변경

		if(c1 != c2){ //답이 다르면 0점
			close(fd1);
			close(fd2);
			return false;
		}
	}
	close(fd1);
	close(fd2);
	return true;
}

void redirection(char *command, int new, int old)//stdout, stderr를 화면에 출력하지 않기 위해 사용 
{
	int saved;

	saved = dup(old);
	dup2(new, old);

	system(command); //command 실행

	dup2(saved, old);
	close(saved);
}

int get_file_type(char *filename)	//get file type : c or txt
{
	char *extension = strrchr(filename, '.'); //문자열에서 해당 문자가 마지막으로 나타나는 위치 포인터 리턴

	if(!strcmp(extension, ".txt"))
		return TEXTFILE;
	else if (!strcmp(extension, ".c"))
		return CFILE;
	else
		return -1;
}

void rmdirs(const char *path)
{
	struct dirent *dirp;
	struct stat statbuf;
	DIR *dp;
	char tmp[50];
	
	if((dp = opendir(path)) == NULL)
		return;

	while((dirp = readdir(dp)) != NULL)
	{
		if(!strcmp(dirp->d_name, ".") || !strcmp(dirp->d_name, ".."))
			continue;

		sprintf(tmp, "%s/%s", path, dirp->d_name);

		if(lstat(tmp, &statbuf) == -1)
			continue;

		if(S_ISDIR(statbuf.st_mode))
			rmdirs(tmp);
		else
			unlink(tmp);	
	}

	closedir(dp);
	rmdir(path);
}

void to_lower_case(char *c) //대문자를 소문자로 변경
{
	if(*c >= 'A' && *c <= 'Z')
		*c = *c + 32;
}

void print_usage() //print program manual option '-h'
{
	printf("Usage : ssu_score <STD_DIR> <ANS_DIR> [OPTION]\n");
	printf("Option : \n");
	printf(" -m                modify question's score\n");
	printf(" -e <DIRNAME>      print error on 'DIRNAME/ID/qname_error.txt' file\n");
	printf(" -t <QNAMES>       compile QNAME.C with -lpthread option\n");
	printf(" -i <IDS>          print ID's wrong questions\n");
	printf(" -h                print usage\n");
}
