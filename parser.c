#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>
#include <sys/mman.h>

#define BUF_SIZE 2048
#define MEMORY_SIZE 1024*8
#define NAME_SIZE 128


#define ASCII (1)
#define HEX (1<<1)
#define DIGITS (1<<2)


/*
flags: 
-a - convert to ascii
-i - convert to int
-h - make chain of hex-symbols 
TODO: make one-byte, 2-bytes, 4- and 8-bytes chains

example: ./parser -sf encode.txt decode.txt
*/



typedef struct{
	char symbol;
	uint8_t code;
} OPTIONS;



void close_all(FILE *in, FILE *out){
	if(in!=NULL){
		fclose(in);
	}
	if(out!=NULL){
		fclose(out);
	}
	return;
}

void check_input(int amount, char *argv[]){
	if(amount<3){
		puts("too few arguments");
		exit(-1);
	}
	for(int i=1; i<amount; i++){
		if(argv[i][0]=='-')return;
	}
	puts("where is flag?");
	exit(-1);
}

void set_flag(uint32_t *flag, char *argv[], int argc, OPTIONS opt[]){
	for(int i=1; i<argc; i++){
		if(argv[i][0]=='-'){
			for(int j=0; opt[j].code!=0; j++){
				if(opt[j].symbol==argv[i][1]){
					(*flag)|=opt[j].code;
				}
			}
		}
	}
}

void set_path(int argc, char *argv[], char names[][128]){
	char is_first=0;
	for(int i=1; i<argc; i++){
		if(argv[i][0]=='-')continue;
		if(is_first==0){
			strncpy(names[0], argv[i], NAME_SIZE-1); //copy first name as source of data
			names[0][NAME_SIZE-1]='\0';
			is_first=1;
			continue;
		}
		strncpy(names[1], argv[i], NAME_SIZE-1);
		names[1][NAME_SIZE-1]='\0';
		return;
	}
	names[1][0]='\0';
	return;
}

void open_files(int *in, int *out, char str[][128]){
	*in = open(str[0], O_RDONLY);
	if(str[1][0]=='\0') *out=STDOUT_FILENO;
	else *out=open(str[1], O_WRONLY | O_CREAT);
	if(*in<0 || *out<0){
		puts("someting went wrong with opening files");
		exit(-1);
	}
}

void map_file(int infd, char **ptr){
	*ptr = mmap(NULL, MEMORY_SIZE, PROT_READ, MAP_SHARED, infd, 0);
	if(*ptr==MAP_FAILED){
		puts("something went wrong with mapping output file");
		exit(-1);
	}
	close(infd);
	return;
}

void reading(char *ptr, char *buf){
	int i, j;
	for(i=0, j=0; ptr[i]!='\0' && i<BUF_SIZE; i++){
		if(ptr[i]=='h'){
			buf[j++]=ptr[i-2];
			buf[j++]=ptr[i-1];
		}
	}
	buf[j]='\0';
}

void printing(int oufd, char *buf){
	int buf_len = strlen(buf);
	write(oufd, buf, buf_len);
	close(oufd);
}

int convert_to_hex(char *str){
	int i;
	for(i=0; str[i]!='\0'; i++){
		if(str[i]>=48 && str[i]<=57)str[i]=str[i]-48;
		else str[i]=str[i]-55;
	}
	return i; //amount of elements
}

char *convert_to_int(char *str){
	char *new_str = malloc(BUF_SIZE*10);
	int j=0, k=0, n=0, c=0;
	convert_to_hex(str);
	int len = strlen(str);
	for(k=0; k+3<len; k+=4){
		c = snprintf(&new_str[n], BUF_SIZE*10 -n, "%d, ", str[k]*16*16*16 + str[k+1]*16*16 + str[k+2]*16 +str[k+3]);
		n+=c;
	}
	free(str);
	return new_str;
}	

char *convert_to_hex_chains(char *str){
	int n=0, c=0, k=0, i=0;
	int len = strlen(str);
	char *new_str=malloc(BUF_SIZE*4);
	for(i=0, k=0; i+1<len; i+=2, k+=4){
		c = snprintf(&new_str[n], BUF_SIZE*4-n, "0x%c%c, ", str[i], str[i+1]);
		n+=c;
	}
	free(str);
	return new_str;
}

void convert_to_ascii(char *str){
	int j;
	int k;
	int i = convert_to_hex(str);
	//well, hex numbers at ghidra always come in pairs 
	for(j=0, k=0; j<i/2; j++, k+=2){
		str[j]=str[k]*16+str[k+1];
	}
	str[j]='\0';

}

int main(int argc, char *argv[]){
	char paths[2][128]={0}; //first - input, second - output
	char *buffer = malloc(BUF_SIZE);
	OPTIONS opt[]={{'a', ASCII}, {'h', HEX}, {'d', DIGITS}, {0, 0}};
	uint32_t flag=0;
	int input;
	int output;
	char *fptr;

	check_input(argc, argv);

	set_flag(&flag, argv, argc, opt);
	
	set_path(argc, argv, paths);
	

	open_files(&input, &output, paths);

	map_file(input,&fptr);

	reading(fptr, buffer);
	

	if((flag&ASCII)==ASCII){
		convert_to_ascii(buffer);
	}
	else if((flag&HEX)==HEX){
		buffer = convert_to_hex_chains(buffer);
	}
	else if((flag&DIGITS)==DIGITS){
		buffer = convert_to_int(buffer);
	}	
	printing(output, buffer);

	munmap(fptr, MEMORY_SIZE);
	free(buffer);
}
