/********************************/
//����һЩ��������Ҫ�õ���ansi c��׼��

#include <malloc.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
/*******************************/


/******************************/
//����һЩ�����ͺ�

#define DEFAULTCOLUMNS 80	//Ĭ����ʾ������ 80�ַ�
#define MAXLEN 255     /*��ȡ�����ļ��ĵ�����󳤶�*/
#define SMO_NO 0     /* �޿���̨������ַ����ģʽ */
#define SMO_YES 1    /*ʹ���ɿ���̨����Ļ��ģʽ ���������ļ��Ļ��ģʽ*/
#define SMO_FORCE 2  /* logically OR command-line and font smushmodes */
//oldLayout��һЩ����
#define SM_SMUSH 128
#define SM_KERN 64
#define SM_EQUAL 1
#define SM_LOWLINE 2
#define SM_HIERARCHY 4
#define SM_PAIR 8
#define SM_BIGX 16
#define SM_HARDBLANK 32
//��������ģʽ ����ģʽ��0����ִ��
#define crycle 1 //ѭ������ 
/******************************/


/******************************/
//ȫ�ֱ����ͽṹ������

long *longline; //������ָ��		//�ַ����������Ķ�λ����
int longlinelen,longlinelenlimit;	//longlinelenlimit����󳤶����� ���������ȵ�4����100�ַ�
long deutsch[7] = {196, 214, 220, 228, 246, 252, 223};	//����ϲ�ģʽ���� �ο��ĵ�
char file_name[16]="rounded.flf";		//�ļ�����(Ĭ�ϳ���16�ַ�)
char file_path[32]="fonts_txt/";		//�ļ�����(Ĭ�ϳ���32�ַ�)
/*
��c������ʹ��typedef struct ��struct���𲻴�
�� struct FC{}����������ṹ��Ļ�,������FC �ı���ʱ,��Ҫ����д struct node n
����typedef struct FC{},���������ʱ�Ϳ�������д FC fc;
���������ʹ��ʱ���Ƿ����ʡȥstruct����ؼ���
*/
//�����ȡ����������Ϣ�Ľṹ��
typedef struct fc {	
  long ord;			//��ʾ��ǰ�ַ�
  char **thechar;  //��ʾ����ַ�ͼ�εĶ���ָ��
  struct fc *next;	//���ӵ���һ���ַ�ָ��
  } fcharnode;
fcharnode *fcharlist;
char **currchar;
int currcharwidth;
int previouscharwidth;
char **outputline;  //���ڱ���������弯�ϵĶ�ά����ָ��
int outlinelen;
int align_type,right2left;	
int smushmode;		//ÿ���ַ��м�ĺϲ�ģʽ
int smushoverride;	//���Ǻϲ�ģʽ 
int outputwidth;	//������
int outlinelenlimit;	//������������
char hardblank;		//�ո��Ƿ�	
int charheight;		//�ַ��߶�
/******************************/


/******************************/
//�������õ��ĺ���������

//��ȡ����̨���� vc6
void processArgs(int argc,char *argv[]);
//��ȡ�����ļ�
void readfont();
//��ȡ�ַ�ͼ�ε��ṹ��
void readfontchar(FILE *file,long theord);
//ȥ���ļ�ǰ4���ַ�flf2
void readhead(FILE *fp,char *magic);
//�������������ı���Ϣ�������ı���
void skiptoeol(FILE *fp);
//�����ַ��߶Ⱥ�����Ŀ�ȷ���ռ�
void linealloc();
//�ָ��� ���ڵ��ʷֶ�
void splitline();
//��ȡ�ַ���Ӧͼ�εĿ�Ȳ��һ�����һ���ַ��п�
void getletter(long c);
//���ַ��߽�ģ������
char smushem(char lch,char rch);
//���ַ�λ��ƫ����
int smushamt();
//������ַ����м����ַ�
int addchar(long c);
//�����
void printline();
//����ַ�������̨����
void putstring(char *string);
//��ӡʹ�÷���
void printusage(FILE *out);
//������鼯��һ�г�ʼ������
void clearline();
//��ʼ������
void initArrayParams(char *ptr,int length);
/******************************/



/***********������ʵ��************/

int main(int argc,char *argv[]){
	long c=0;		//������ַ�
	int i=0,len=0;
	int count=0;
	int wordbreakmode=0;
	int char_not_added=0;		//�ַ�δ�������������
	char in_buffer[64]={0}; //��������
	processArgs(argc,argv);	//vc6.0��ȡ����̨����
	readfont();		//��ȡ�����ļ��������ַ�ͼ�ε��ṹ��
	linealloc();		//���ͼ�μ����ڴ����
	wordbreakmode = 0;	//�ִ�ģʽĬ��0
	//��ʼ�����뻺������
	for(count=0;count<64;count++){
			in_buffer[count]=0;
		}
	//��ѭ��
	while(crycle){
		gets(in_buffer);	//��ȡ����̨����
		len=strlen(in_buffer);	
		if(strcmp(in_buffer,"quit")==0){	//quit�˳�
			exit(0);
		}else if(in_buffer[0]==0){		//ctrl+z�˳�
			exit(0);
		}
		for(count=0;count<len;count++){	//ѭ�������ַ����ȴ�����ÿ���ִ���
			c=in_buffer[count];			//ȡ�����뻺�������Ӧѭ������ֵ���ַ�
			if(isascii(c)&&isspace(c)){	//���c��ascii�ַ����еĿո�
				c = (c=='\t'||c==' ') ? ' ' : '\n';		//�̶��ж��Ƕ�λ��\t����asciiֵ32Ϊ�淵�ؿո�
			}
			//���c���ڽ�������С��32��c���ǻ��л���c��delɾ����
			//���䲻�ڳ�����ķ�Χ,������Щ���Ʒ�
			if((c>'\0' && c<' ' && c!='\n') || c==127) continue;

			do{
				char_not_added = 0;	//�ַ�����
				//�ַ��ֶϴ����ж�
				if (wordbreakmode== -1) {
					if (c==' '){
						break;
					}else if(c=='\n'){
						wordbreakmode = 0;
						break;
					}
				wordbreakmode = 0;
				}
			if (c=='\n') {
				printline();
				wordbreakmode = 0;
			}else if(addchar(c)){
				if (c!=' '){
					wordbreakmode = (wordbreakmode>=2)?3:1;
				}else{
					wordbreakmode = (wordbreakmode>0)?2:0;
				}
			}else if(outlinelen==0){
				for(i=0;i<charheight;i++){
					if(right2left && outputwidth>1){
						putstring(currchar[i]+strlen(currchar[i])-outlinelenlimit);
					}else{
						putstring(currchar[i]);
					}
				}
				wordbreakmode = -1;
			}else if (c==' '){	//�����ո��ҷֶ�ģʽ2 ���� �������õ��ʽ�����ӡ
				if (wordbreakmode==2){
					splitline();
				}else{
					printline();
				}
				wordbreakmode = -1;
			}else{
				if(wordbreakmode>=2){	//����ֶ�ģʽ���ڵ���2ִ��
					splitline();
				}else{
					printline();
			  }
				wordbreakmode = (wordbreakmode==3)?1:0;
				char_not_added = 1;	//�ַ�δ����
			}
		  }while (char_not_added);
		}
		//��ʼ����������
		for(count=0;count<64;count++){
			in_buffer[count]=0;
		}
		if(outlinelen!=0){
			printline();
		}
	}
	return 0;
}

/***************��������ʵ��***********************/


//��ʼ������
//��������ָ�� ���鳤�� ��������ÿ��Ԫ��
void initArrayParams(char *ptr,int length){
	int i=0;
	for(i=0;i<length;i++){
		*(ptr+i)=0;
	}
}

//��ӡʹ�÷���
void printusage(FILE *out){
	fprintf(out,"Usage: [ -d ���������ļ��� ]\n");
	fprintf(out,
		"       [ -f ���������ļ� ] [ -w ���������� ] [-c,-l,-r ���ö��뷽ʽ]\n");
	fprintf(out,
		"       [ -i ��ʾ��Ϣ ]\n");
}

void processArgs(int argc,char *argv[]){
	int i=0,len=0;
	int columns=0;	//�п� 
	smushoverride = SMO_NO;
	align_type = -1;
	right2left = -1;
	outputwidth = DEFAULTCOLUMNS;	//������Ĭ��ֵ
	for(i=1;i<argc;i++){
		if(argv[i][0]=='-'){
			switch(argv[i][1]){
				//align_type
				case 'l':
					align_type = 0;
					break;
				case 'r':
					align_type = 2;
					break;
				case 'c':
					align_type = 1;
					break;
				if (smushmode == 0) smushmode = SM_KERN;
				else if (smushmode == -1) smushmode = 0;
				else smushmode = (smushmode & 63) | SM_SMUSH;
				smushoverride = SMO_YES;
					break;
				//display_width
				case 'w':
					columns=atoi(argv[i+1]);
					if(columns>0){
						outputwidth = columns;
					}
					break;
				//file_name
				case 'f':
					initArrayParams(file_name,16);
					strcpy(file_name,argv[i+1]);
					len=strlen(file_name);
					file_name[len]='.';
					file_name[len+1]='f';
					file_name[len+2]='l';
					file_name[len+3]='f';
					file_name[len+4]='\0';
					break;
				case 'd':
					initArrayParams(file_path,32);
					strcpy(file_path,argv[i+1]);
					len=strlen(file_path);
					file_path[len]='/';
					file_path[len+1]='\0';
					break;
				case 'h':
					printusage(stdout);
					exit(1);
					break;
				default :
					printf("***********************************\n");
					printf("      *��ӭʹ��e-mailǩ�����*\n");
					printf("        *���Ķ��·�ʹ�÷�ʽ*\n");
					printf("***********************************\n\n");
					printusage(stdout);
					exit(1);
					break;
			}
		}
	}
	outlinelenlimit = outputwidth-1;
}


//�������������ı���Ϣ�е������ı��� ����һ����������
void skiptoeol(FILE *fp){
	int ch;
	while(ch=fgetc(fp),ch!=EOF){
		if(ch == '\n') return;
		if(ch == '\r'){
			ch = fgetc(fp);
			if (ch != EOF && ch != '\n') ungetc(ch,fp);
			return;
		}
	}
}

//ȥ���ļ�ǰ4���ַ�flf2
void readhead(FILE *fp,char *magic){
	int i;
	for (i=0;i<4;i++) {
		magic[i] = fgetc(fp);
    }
	magic[4] = 0;
}

//��������鼯��һ�г�ʼ��
void clearline(){
	int i;
	for(i=0;i<charheight;i++){
		outputline[i][0] = '\0';
    }
	outlinelen = 0;
	longlinelen = 0;
}
//��ȡ�ַ�ͼ�ε��ṹ��
void readfontchar(FILE *file,long theord){
	int row,k;
	char templine[MAXLEN+1];
	char endchar, outline[MAXLEN+1];
	fcharnode *fclsave;
	fclsave = fcharlist;
	fcharlist = (fcharnode*)malloc(sizeof(fcharnode));
	fcharlist->ord = theord;
	fcharlist->thechar = (char**)malloc(sizeof(char*)*charheight);
	fcharlist->next = fclsave;
	outline[0] = 0;

	for(row=0;row<charheight;row++){
    if(fgets(templine,MAXLEN,file)==NULL){
		templine[0] = '\0';
      }
    strcpy(outline,templine);
    k = strlen(outline)-1;
    while(k>=0 && isspace(outline[k])){  /* remove trailing spaces */
		k--;
      }
    if(k>=0){
		endchar = outline[k];  /* remove endmarks */
		while(k>=0 && outline[k]==endchar){
			k--;
        }
      }
    outline[k+1] = '\0';
    fcharlist->thechar[row] = (char*)malloc(sizeof(char)*(strlen(outline)+1));
    strcpy(fcharlist->thechar[row],outline);
    }
}
//��ȡ�����ļ�
void readfont(){
	int i,row,numsread;
	long theord;
	int maxlen,cmtlines,ffright2left;
	int smush,smush2;
	char fileline[MAXLEN+1],magicnum[5];	//���ڱ���flf2����ȥ��
	char fname[32]={0};
	FILE *fontfile;
	strcat(file_path,file_name);	//ƴ���ļ����͸����ļ����γ�·��
	fontfile = fopen(file_path,"r");	//ֻ����ʽ���ļ�
	if(fontfile==NULL){
		fprintf(stderr,"%s: Unable to open font file,check filepath or filename\n",file_path);
		exit(1);
    }
	readhead(fontfile,magicnum);	//ȥ�������ļ���flf2
	if(fgets(fileline,MAXLEN,fontfile)==NULL){
		fileline[0] = '\0';
    }
	//����ϴζ�ȡ��fileline���ȴ�����fileline�Ի��н���ִ������ע����
	if(strlen(fileline)>0 ? fileline[strlen(fileline)-1]!='\n' : 0){
		skiptoeol(fontfile);	
    }
	//��ȡ�ļ�ͷ��Ϣ
	numsread = sscanf(fileline,"%*c%c %d %*d %d %d %d %d %d",&hardblank,&charheight,&maxlen,&smush,&cmtlines,&ffright2left,&smush2);
	
	if(maxlen > MAXLEN){
		fprintf(stderr,"%s: line character is too wide\n",file_path);
		exit(1);
    }
	//����cmtlines��ע����
	for(i=1;i<=cmtlines;i++){
		skiptoeol(fontfile);	//ѭ����������������Ϣ��
    }	
	//�����ȡ��������С��6 ��Ϊ�ϰ汾�������ļ� ����ֻ��5�� ��ô���������ӡΪ0
	if(numsread<6){
		ffright2left = 0;
    }

	if(numsread<7){ /* ���û���µĹ������ �ͽ�oldlayoutת�����¹������*/
		if (smush == 0) smush2 = SM_KERN;
		else if (smush < 0) smush2 = 0;
		else smush2 = (smush & 31) | SM_SMUSH;
    }
	if(charheight<1){	//charheight maxlen ��СֵΪ1
		charheight = 1;
    }

	if(maxlen<1){
		maxlen = 1;
    }
	maxlen += 100; /* maxlen������ؿ��԰������ */
	if(smushoverride == SMO_NO)
		smushmode = smush2;
	else if (smushoverride == SMO_FORCE)
		smushmode |= smush2;
	if(right2left<0){	
		right2left = ffright2left;
    }
	if(align_type<0){
		align_type = 2*right2left;
    }

	fcharlist = (fcharnode*)malloc(sizeof(fcharnode));
	fcharlist->ord = 0;
	fcharlist->thechar = (char**)malloc(sizeof(char*)*charheight);
	fcharlist->next = NULL;
	for(row=0;row<charheight;row++){
		fcharlist->thechar[row] = (char*)malloc(sizeof(char));
		fcharlist->thechar[row][0] = '\0';
    }
	for(theord=' ';theord<='~';theord++){
		readfontchar(fontfile,theord);
    }
	for(theord=0;theord<=6;theord++){
		readfontchar(fontfile,deutsch[theord]);
    }
	while(fgets(fileline,maxlen+1,fontfile)==NULL?0:
    sscanf(fileline,"%li",&theord)==1){
    readfontchar(fontfile,theord);
    }
	fclose(fontfile);
}

//�����ַ��߶Ⱥ�����Ŀ�ȷ���ռ�
void linealloc(){
	int row; //��
	//����charheight�����и�
	outputline = (char**)malloc(sizeof(char*)*charheight);
	for (row=0;row<charheight;row++) {
		//ÿ���и߷����п�
		outputline[row] = (char*)malloc(sizeof(char)*(outlinelenlimit+1));
    }
	//�����ƿ��
	longlinelenlimit = outputwidth*4+100;
	//���������
	longline = (long*)malloc(sizeof(long)*(longlinelenlimit+1));

	clearline();
}
//��ȡ�ַ���Ӧͼ�εĿ�Ȳ��һ�����һ���ַ��п�
void getletter(long c){
	fcharnode *charptr;		//����һ���ַ��ṹ��ָ��ָ��fcharlist��Ԫ��
	//�����ַ�����ordָ������ݲ�Ϊ�����cʱ ����ǰ������� ��ָ��ָ����һ������
	for(charptr=fcharlist;charptr==NULL?0:charptr->ord!=c;charptr=charptr->next) ;
	//�жϷ�֧��ȥ��,���������ѭ��charptr==NULL���������
	if(charptr!=NULL){
		currchar = charptr->thechar;	//ָ��ǰ�ַ�ͼ��
    }else{
		for(charptr=fcharlist;charptr==NULL?0:charptr->ord!=0;charptr=charptr->next) ;
		currchar = charptr->thechar;
    }
	previouscharwidth = currcharwidth;	//����ǰһ���ַ����
	currcharwidth = strlen(currchar[0]);	//ˢ�µ�ǰ�ַ����Ϊcurrchar
}

//���ַ��߽�ģ������
char smushem(char lch,char rch){
	if(lch==' ') return rch;
	if(rch==' ') return lch;
	if(previouscharwidth<2 || currcharwidth<2) return '\0';
	if((smushmode & SM_SMUSH) == 0) return '\0';  /* kerning */
	if((smushmode & 63) == 0){
		if (lch==hardblank) return rch;
		if (rch==hardblank) return lch;
		if (right2left==1) return lch;
		return rch;
    }
	if(smushmode & SM_HARDBLANK){
		if (lch==hardblank && rch==hardblank) return lch;
    }
	if(lch==hardblank || rch==hardblank) return '\0';
	if(smushmode & SM_EQUAL){
		if (lch==rch) return lch;
    }
	if(smushmode & SM_LOWLINE){
		if (lch=='_' && strchr("|/\\[]{}()<>",rch)) return rch;
		if (rch=='_' && strchr("|/\\[]{}()<>",lch)) return lch;
    }
	if(smushmode & SM_HIERARCHY){
		if (lch=='|' && strchr("/\\[]{}()<>",rch)) return rch;
		if (rch=='|' && strchr("/\\[]{}()<>",lch)) return lch;
		if (strchr("/\\",lch) && strchr("[]{}()<>",rch)) return rch;
		if (strchr("/\\",rch) && strchr("[]{}()<>",lch)) return lch;
		if (strchr("[]",lch) && strchr("{}()<>",rch)) return rch;
		if (strchr("[]",rch) && strchr("{}()<>",lch)) return lch;
		if (strchr("{}",lch) && strchr("()<>",rch)) return rch;
		if (strchr("{}",rch) && strchr("()<>",lch )) return lch;
		if (strchr("()",lch) && strchr("<>",rch)) return rch;
		if (strchr("()",rch) && strchr("<>",lch)) return lch;
    }
	if(smushmode & SM_PAIR){
		if (lch=='[' && rch==']') return '|';
		if (rch=='[' && lch==']') return '|';
		if (lch=='{' && rch=='}') return '|';
		if (rch=='{' && lch=='}') return '|';
		if (lch=='(' && rch==')') return '|';
		if (rch=='(' && lch==')') return '|';
    }
	if(smushmode & SM_BIGX){
		if (lch=='/' && rch=='\\') return '|';
		if (rch=='/' && lch=='\\') return 'Y';
		if (lch=='>' && rch=='<') return 'X';
    }
	return '\0';
}

int smushamt(){
	unsigned int maxsmush,amt;
	int row,linebd,charbd;
	char ch1,ch2;
	if((smushmode & (SM_SMUSH | SM_KERN))==0){
    return 0;
	}
	maxsmush = currcharwidth;
	for (row=0;row<charheight;row++) {
		if (right2left) {
			if (maxsmush>strlen(outputline[row])) {
				maxsmush=strlen(outputline[row]);
			}
			for (charbd=strlen(currchar[row]);
			ch1=currchar[row][charbd],(charbd>0&&(!ch1||ch1==' '));charbd--) ;
			for (linebd=0;ch2=outputline[row][linebd],ch2==' ';linebd++) ;
				amt = linebd+currcharwidth-1-charbd;
		}else {
			for (linebd=strlen(outputline[row]);
				ch1 = outputline[row][linebd],(linebd>0&&(!ch1||ch1==' '));linebd--) ;
			for (charbd=0;ch2=currchar[row][charbd],ch2==' ';charbd++) ;
				amt = charbd+outlinelen-1-linebd;
		}
		if (!ch1||ch1==' ') {
			amt++;
		}else if (ch2) {
			if (smushem(ch1,ch2)!='\0') {
				amt++;
			}
		}
		if (amt<maxsmush) {
			maxsmush = amt;
		}
	}
	return maxsmush;//4��
}

//�ָ��� ���ڵ��ʷֶ�
void splitline(){
	int i,gotspace,lastspace,len1,len2;
	long *part1,*part2; 
	part1 =(long*)malloc(sizeof(long)*(longlinelen+1));
	part2 =(long*)malloc(sizeof(long)*(longlinelen+1));
	gotspace = 0;
	lastspace = longlinelen-1;
	for(i=longlinelen-1;i>=0;i--){
		if (!gotspace && longline[i]==' ') {
			gotspace = 1;
			lastspace = i;
		}
    if(gotspace && longline[i]!=' '){
		break;
      }
    }
	len1 = i+1;
	len2 = longlinelen-lastspace-1;
	for(i=0;i<len1;i++){
		part1[i] = longline[i];
    }
	for(i=0;i<len2;i++){
		part2[i] = longline[lastspace+1+i];
    }
	clearline();
	for(i=0;i<len1;i++){
		addchar(part1[i]);
    }
	printline();
	for(i=0;i<len2;i++){
		addchar(part2[i]);
    }
	free(part1);
	free(part2);
}

//������ַ����м����ַ�
int addchar(long c){
	int smushamount,row,k,column;
	char *templine;
	getletter(c);
	smushamount=smushamt();
	if(outlinelen+currcharwidth-smushamount>outlinelenlimit || longlinelen+1>longlinelenlimit){
		return 0;
    }
	templine = (char*)malloc(sizeof(char)*(outlinelenlimit+1));
	for (row=0;row<charheight;row++){
		if (right2left) {
			strcpy(templine,currchar[row]);
			for (k=0;k<smushamount;k++){
				templine[currcharwidth-smushamount+k] = smushem(templine[currcharwidth-smushamount+k],outputline[row][k]);
			}
			strcat(templine,outputline[row]+smushamount);
			strcpy(outputline[row],templine);
      }else{
		for (k=0;k<smushamount;k++){
			column = outlinelen-smushamount+k;
			if (column < 0) {
				column = 0;
			}
			outputline[row][column] = smushem(outputline[row][column],currchar[row][k]);
		}
		strcat(outputline[row],currchar[row]+smushamount);
      }
    }
	free(templine);
	outlinelen = strlen(outputline[0]);
	longline[longlinelen++] = c;
	return 1;
}

//�д�ӡ
void printline(){
	int i;
	for (i=0;i<charheight;i++) {
		putstring(outputline[i]);
    }
	clearline();
}

//����ַ�������̨����
void putstring(char *string){
	int i,len;
	len = strlen(string);
	if (outputwidth>1) {
		if (len>outputwidth-1) {
			len = outputwidth-1;
		}
    if (align_type>0) {
		for (i=1;(3-align_type)*i+len+align_type-2<outputwidth;i++) {
			putchar(' ');
        }
      }
    }
	for(i=0;i<len;i++){
		putchar(string[i]==hardblank?' ':string[i]);
    }
	putchar('\n');
}