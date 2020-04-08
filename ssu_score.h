#ifndef MAIN_H_
#define MAIN_H_

#ifndef true
	#define true 1
#endif
#ifndef false
	#define false 0
#endif
#ifndef STDOUT
	#define STDOUT 1
#endif
#ifndef STDERR
	#define STDERR 2
#endif
#ifndef TEXTFILE
	#define TEXTFILE 3
#endif
#ifndef CFILE
	#define CFILE 4
#endif
#ifndef OVER
	#define OVER 5
#endif
#ifndef WARNING
	#define WARNING -0.1
#endif
#ifndef ERROR
	#define ERROR 0
#endif

#define FILELEN 64
#define BUFLEN 1024
#define SNUM 100
#define QNUM 100
#define ARGNUM 5

struct ssu_scoreTable{
	char qname[FILELEN];
	double score;
};

void ssu_score(int argc, char *argv[]); //사실상 메인함수
int check_option(int argc, char *argv[]); //if it is out of form, throw exception.
void print_usage();	//print program manual option '-h'

void score_students(); //학생 점수 채점(채점 결과 테이블 작성)
double score_student(int fd, char *id); //학생별 점수 계산, 해당 학생 총 점수 리턴
void write_first_row(int fd); //채점 결과 테이블의 첫 행 채우기

char *get_answer(int fd, char *result);
int score_blank(char *id, char *filename); //빈칸문제 채점 시작
double score_program(char *id, char *filename); //프로그램 문제 채점 시작
double compile_program(char *id, char *filename); //프로그램 문제 학생 답안과 정답을 컴파일
int execute_program(char *id, char *filname); //프로그램 문제 학생 답안과 정답을 실행 
pid_t inBackground(char *name); //학생답안의 프로그램이 실행중인지 체크
double check_error_warning(char *filename);//애러면 0점, warning이면 감점된 점수 리턴
int compare_resultfile(char *file1, char *file2); //프로그램 문제 학생 답안과 정답 결과를 비교하여 채점

void do_cOption(char (*ids)[FILELEN]);
int is_exist(char (*src)[FILELEN], char *target);

int is_thread(char *qname);
void redirection(char *command, int newfd, int oldfd); //command실행시 stdout, stderr를 화면에 출력하지 않기 위해 사용
int get_file_type(char *filename); //get file type : .c or .txt
void rmdirs(const char *path);
void to_lower_case(char *c); //대문자를 소문자로 변경

void set_scoreTable(char *curDir); //set scoreTable.
void read_scoreTable(char *path); //score_table.csv에 적힌 문제별 점수를 score_table 배열에 저장
void make_scoreTable(char *curDir); //score_table 배열에 문제별 점수를 저장
void write_scoreTable(char *filename); //score_table.csv 생성
void set_idTable(); //id_table변수에 학번 저장
int get_create_type(); //점수 테이블을 어떻게 생성할건지 묻는 질문

void sort_idTable(int size); //id_table변수 오름차순으로 정렬
void sort_scoreTable(int size); //점수테이블을 문제번호 오름차순으로 정렬
void get_qname_number(char *qname, int *num1, int *num2); //문제 번호 리턴

#endif
