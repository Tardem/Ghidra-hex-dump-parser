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
#define INTEGER (1<<2)
#define CHAIN1 (1<<3)
#define CHAIN2 (1<<4)
#define CHAIN4 (1<<5)
#define CHAIN8 (1<<6)

/*
flags: 
-a - convert to ascii
-i - convert to int
-h - make chain of hex-symbols
-h1, -h2, -h4, -h8 - codes for amount of bytes in one element of input str 

*/

typedef struct{
	char symbol;
	uint8_t code;
} OPTIONS;




int check_hex(int second, int first){
	if((second>=48 && second<=57) || (second>=65 && second<=70)){
		if((first>=48 && first<=57) || (first>=65 && first<=70)){
			return 1;
		}
	}
	return 0;

}


void check_input(int amount, char *argv[], OPTIONS opt[]){
	int i, valid_flag;
	if(amount<3){
		puts("too few arguments");
		exit(-1);
	}
	for(i=1; i<amount; i++){
		if(argv[i][0]=='-')break;
	}
	if(i>=amount){
		puts("you are forgot the \"-\" for flag");
		exit(-1);
	}
	for(int j=1; argv[i][j]!='\0'; j++){
		valid_flag=0;
		for(int k=0; opt[k].code!=0; k++){
			if(argv[i][j]==opt[k].symbol){
				valid_flag=1;
				break;
			}
			
		}
		if(valid_flag==0){
				printf("you are inter uncorrect flag \"%c\"", argv[i][j]);
				exit(-1);
			}	
	}
}

void set_flag(uint32_t *flag, char *argv[], int argc, OPTIONS opt[]){
	for(int i=1; i<argc; i++){
		if(argv[i][0]=='-'){
			for(int j=0; opt[j].code!=0; j++){
				for(int k=1; argv[i][k]!='\0'; k++){
					if(opt[j].symbol==argv[i][k]){
						(*flag)|=opt[j].code;
					}	
				}
			}
			return;
		}
	}
}


//check flags:


int check_main_flags(uint32_t flag, OPTIONS opt[]){
	int i=0, j=0;
	char  main_flag=0; //-h, -i, -a
	for(i=0; opt[i].symbol!='1'; i++){
		if(main_flag){
			if((flag&opt[i].code)==opt[i].code){
				puts("you enter more than one main flag!");
				exit(-1);
			}
		}
		else if((flag&opt[i].code)==opt[i].code){
			main_flag=1;
		}
	}
	if((flag&HEX)==HEX){
		return 1;
	}
	return 0;
}


void check_chains_flag(uint32_t flag, OPTIONS opt[], int is_hex_set, int *amount_of_bytes){
	int i;
	int chain_flag_set=0;
	//begin from i=3, becouse chain-flags located starting from this num
	for(i=3; opt[i].code!=0; i++){
		if(chain_flag_set){
			if((flag&opt[i].code)==opt[i].code){
				puts("you are enter more than one chain flag");
				exit(-1);
			}	
		}
		if((flag&opt[i].code)==opt[i].code){
			if(!is_hex_set){
				puts("you cant use chain flag without flah -h");
				exit(-1);
			}
			chain_flag_set=1;
			*amount_of_bytes=opt[i].symbol-48;
		}
	}
	if(chain_flag_set==0 && (flag&HEX)==HEX){
		puts("you forget enter amount of half-bytes at on element");
		exit(-1);
	}	
	printf("\n\namount_of_bytes: %d\n\n", *amount_of_bytes);
	fflush(stdout);
}

void check_flags(uint32_t flag, OPTIONS opt[], int *amount_of_bytes){
	int is_hex_set;
	is_hex_set = check_main_flags(flag, opt);
	check_chains_flag(flag, opt, is_hex_set, amount_of_bytes);
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


//file section


void open_files(int *in, int *out, char str[][128], int* fsize){
	struct stat st;
	*in = open(str[0], O_RDONLY);
	if(str[1][0]=='\0') *out=STDOUT_FILENO;
	else *out=open(str[1], O_WRONLY | O_CREAT | O_TRUNC, (mode_t)0644);
	if(*in<0 || *out<0){
		puts("someting went wrong with opening files");
		exit(-1);
	}
	if(fstat((*in), &st)<0){
		puts("fstat called error");
		exit(-1);
	}
	*fsize=st.st_size;
}

void map_file(int infd, char **ptr, int fsize){
	*ptr = mmap(NULL, fsize, PROT_READ, MAP_SHARED, infd, 0);
	if(*ptr==MAP_FAILED){
		puts("something went wrong with mapping output file");
		exit(-1);
	}
	close(infd);
	return;
}



//read and write



void reading(char *ptr, char *buf, int fsize, int *data_len){
	int i, j;
	for(i=2, j=0; i<fsize; i++){
		if(ptr[i]=='h' && check_hex(ptr[i-2], ptr[i-1])){
				buf[j++]=ptr[i-2];
				buf[j++]=ptr[i-1];
			}
		}
	buf[j]='\0';
	*data_len=j;
}

void printing(int oufd, char *buf){
	int buf_len = strlen(buf);
	write(oufd, buf, buf_len);
	close(oufd);
}


//converting section


int convert_to_hex(char *str, int data_len){
	int i;
	for(i=0; i<data_len; i++){
		if(str[i]>=48 && str[i]<=57)str[i]=str[i]-48;
		else str[i]=str[i]-55;
	}
	str[i-1]='\0';
	return i; //amount of elements
}

char *convert_to_int(char *str, int data_len){
	char *new_str = malloc(data_len*10);
	int j=0, k=0, n=0, c=0;
	int len = convert_to_hex(str, data_len);
	for(k=0; k+3<len; k+=4){
		c = snprintf(&new_str[n], data_len*10 -n, "%d, ", (str[k]<<12) + (str[k+1]<<8) + (str[k+2]<<4) +str[k+3]);
		n+=c;
	}
	new_str[n]='\0';
	free(str);
	return new_str;
}	

//amount_of_bytes - amount of bytes in one element
char *convert_to_hex_chains(char *str, int data_len, int amount_of_bytes){
	int n=0, c=0, k=0, i=0, j=0;
	char temp_arr[amount_of_bytes*2+1]; //amount_of_bytes + "0x" + ", "
	char *new_str=malloc(data_len*4);
	for(i=0; i+1<data_len; i+=(amount_of_bytes*2)){
		for(j=0; j<amount_of_bytes*2; j++){
			temp_arr[j]=str[i+j];
		}
		temp_arr[j]='\0';
		c = snprintf(&new_str[n], data_len*4-n, "0x%s, ", temp_arr);
		n+=c;
	}
	new_str[n]='\0';
	free(str);
	return new_str;
}

char *convert_to_ascii(char *str, int data_len){
	int j;
	int k;
	int i = convert_to_hex(str, data_len);
	char *new_str = malloc(i);
	//well, hex numbers at ghidra always come in pairs 
	for(j=0, k=0; j<i/2; j++, k+=2){
		new_str[j]=str[k]*16+str[k+1];
	}
	new_str[j]='\0';
	free(str);
	return new_str;
}




int main(int argc, char *argv[]){
	char paths[2][128]={0}; //first - input, second - output
	OPTIONS opt[]={{'a', ASCII}, {'h', HEX}, {'i', INTEGER}, {'1', CHAIN1}, {'2', CHAIN2}, {'4', CHAIN4}, {'8', CHAIN8}, {0, 0}};
	uint32_t flag=0;
	int input;
	int output;
	int fsize=0;
	int data_len=0;
	int amount_of_bytes=0;
	char *fptr;

	check_input(argc, argv, opt);

	set_flag(&flag, argv, argc, opt);
	
	check_flags(flag, opt, &amount_of_bytes);

	set_path(argc, argv, paths);
	

	open_files(&input, &output, paths, &fsize);
	char *buffer = malloc(fsize);
	map_file(input,&fptr, fsize);

	
	reading(fptr, buffer, fsize, &data_len);
	
	if((flag&ASCII)==ASCII){
		buffer = convert_to_ascii(buffer, data_len);
	}
	else if((flag&HEX)==HEX){
		buffer = convert_to_hex_chains(buffer, data_len, amount_of_bytes);
	}
	else if((flag&INTEGER)==INTEGER){
		buffer = convert_to_int(buffer, data_len);
	}	
	
	printing(output, buffer);

	munmap(fptr, fsize);
	free(buffer);
}

