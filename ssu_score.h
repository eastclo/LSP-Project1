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

void ssu_score(int argc, char *argv[]);
int check_option(int argc, char *argv[]); //if it is out of form, throw exception.
void print_usage();	//print program manual option '-h'

void score_students();
double score_student(int fd, char *id);
void write_first_row(int fd);

char *get_answer(int fd, char *result);
int score_blank(char *id, char *filename);
double score_program(char *id, char *filename);
double compile_program(char *id, char *filename);
int execute_program(char *id, char *filname);
pid_t inBackground(char *name);
double check_error_warning(char *filename);
int compare_resultfile(char *file1, char *file2);

void do_cOption(char (*ids)[FILELEN]);
int is_exist(char (*src)[FILELEN], char *target);

int is_thread(char *qname);
void redirection(char *command, int newfd, int oldfd);
int get_file_type(char *filename); //get file type : .c or .txt
void rmdirs(const char *path);
void to_lower_case(char *c);

void set_scoreTable(char *curDir); //set scoreTable.
void read_scoreTable(char *path); //score_table.csv에 적힌 문제별 점수를 score_table 배열에 저장
void make_scoreTable(char *curDir); //score_table 배열에 문제별 점수를 저장
void write_scoreTable(char *filename); //score_table.csv 생성
void set_idTable(char *stuDir); //id_table변수에 학번 저장
int get_create_type(); //점수 테이블을 어떻게 생성할건지 묻는 질문

void sort_idTable(int size); //id_table변수 오름차순으로 정렬
void sort_scoreTable(int size); //점수테이블을 문제번호 오름차순으로 정렬
void get_qname_number(char *qname, int *num1, int *num2); //문제 번호 리턴

#endif
