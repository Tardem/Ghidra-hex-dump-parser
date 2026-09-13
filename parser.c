#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BUF_SIZE 2044
#define S 1
#define F (1<<1)
#define A (1<<2)
#define H (1<<3)
/*
flags: 
-s - print at screen
-f - print in the file
-sf print at screen and in the file
-a - convert to ascii
-h - make chain of hex-symbols like "0xaa, 0xbb"
example: ./parser -sf encode.txt decode.txt


*/
void close_all(FILE *in, FILE *out){
	if(in!=NULL){
		fclose(in);
	}
	if(out!=NULL){
		fclose(out);
	}
	return;
}

void flag_setup(char *f, char *opt){
	for(int i=0; opt[i]!='\0'; i++){
		if(opt[i]=='s')*f|=1;
		else if(opt[i]=='f')*f|=(1<<1);
		else if(opt[i]=='a')*f|=(1<<2);
		else if(opt[i]=='h')*f|=(1<<3);
	}
	if((((*f)&(1<<2))==A) && (((*f)&(1<<3))==H)){
		puts("program cant make ascii and hex-code in one time");
		exit(-1);
	}
	return;
}

void convert_to_ascii(char *str){
	int i;
	int j;
	int k;
	for(i=0; str[i]!='\0'; i++){
		if(str[i]>=48 && str[i]<=57)str[i]=str[i]-48;
		else str[i]=str[i]-55;
	}
	//well, hex numbers at ghidra always come in pairs 
	for(j=0, k=0; j<i/2; j++, k+=2){
		str[j]=str[k]*16+str[k+1];
	}
	str[j]='\0';

}

char* convert_to_hex_chain(char *str){
	char *new_str=malloc(BUF_SIZE*4);
	int i;
	int k;
	for(i=0, k=0; str[i]!='\0'; i+=2, k+=6){
		new_str[k]='0';
		new_str[k+1]='x';
		new_str[k+2]=str[i];
		new_str[k+3]=str[i+1];
		new_str[k+4]=',';
		new_str[k+5]=' ';
	}
	new_str[k]='\0';
	return new_str;
}

void print(char f, char *str, FILE *output){
	if((f&1)==S){	
		printf("%s", str);
	}
	if((f&(1<<1))==F){
		fprintf(output, "%s", str);
	}
}

int main(int argc, char *argv[]){
	char str_el;
	char *output_string=malloc(BUF_SIZE);
	char flags[]={'s', 'f', '\0'};
	char flag=0; // 1-th bit = -s, 2-th = -f 3-th =-a 4-th = -h
	int i;
	FILE *input=NULL;
	FILE *output=NULL;	
	if(argc<3){
		puts("please, enter the params");		
		exit(-1);	
	}
	flag_setup(&flag, argv[1]);
	input=fopen(argv[2], "rb");
	if(input==NULL){
		printf("ERROR. Program cant open file %s", argv[2]);
		exit(-1);

	}
	if((flag&(1<<1))==F){
	       	if(argc<4){
	       		puts("enter output file!");
			exit(-1);
	       	}	
		output=fopen(argv[3], "wb");
		if(output==NULL){
			printf("ERROR. Program cant open file %s", argv[3]);
			exit(-1);
		}
	}
	for(i=0; fread(&str_el, sizeof(char), 1, input)!=0; i+=2){
		if(str_el=='h'){
			fseek(input, -3, SEEK_CUR);
			fread((output_string+i), sizeof(char), 2, input);
			fseek(input, 1, SEEK_CUR);
		}
		else i-=2;
	}
	output_string[i]='\0';
	if((flag&(1<<2))==A){
		puts("start converting");
		convert_to_ascii(output_string);
	}
	else if((flag&(1<<3))==H){
		output_string=convert_to_hex_chain(output_string);
	}
	print(flag, output_string, output);
	close_all(input, output);
}
