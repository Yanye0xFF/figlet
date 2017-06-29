/********************************/
//包含一些程序中需要用到的ansi c标准库
#include <malloc.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
/*******************************/
/******************************/
//定义一些常量和宏

#define DEFAULTCOLUMNS 80	//默认显示的列数 80字符
#define MAXLEN 255     /*读取字体文件的单行最大长度*/
#define SMO_NO 0     /* 无控制台输入的字符混合模式 */
#define SMO_YES 1    /*使用由控制台输入的混合模式 忽略字体文件的混合模式*/
#define SMO_FORCE 2  /* logically OR command-line and font smushmodes */
//oldLayout的一些参数
#define SM_SMUSH 128
#define SM_KERN 64
#define SM_EQUAL 1
#define SM_LOWLINE 2
#define SM_HIERARCHY 4
#define SM_PAIR 8
#define SM_BIGX 16
#define SM_HARDBLANK 32
//程序运行模式 测试模式下0单次执行
#define crycle 1 //循环运行 
/******************************/
/******************************/
//全局变量和结构体声明

long *longline; //整个行指针		//字符输出行数组的定位变量
int longlinelen,longlinelenlimit;	//longlinelenlimit行最大长度限制 等于输入宽度的4倍加100字符
long deutsch[7] = {196, 214, 220, 228, 246, 252, 223};	//字体合并模式代码 参考文档
char file_name[16]="rounded.flf";		//文件名称(默认长度16字符)
char file_path[32]="fonts_txt/";		//文件名称(默认长度32字符)
/*
在c语言中使用typedef struct 和struct区别不大
若 struct FC{}这样来定义结构体的话,在申请FC 的变量时,需要这样写 struct node n
若用typedef struct FC{},在申请变量时就可以这样写 FC fc;
区别就在于使用时，是否可以省去struct这个关键字
*/
//保存读取到的字体信息的结构体
typedef struct fc {	
  long ord;			//表示当前字符
  char **thechar;  //表示这个字符图形的二重指针
  struct fc *next;	//链接到下一个字符指针
  } fcharnode;
fcharnode *fcharlist;
char **currchar;
int currcharwidth;
int previouscharwidth;
char **outputline;  //用于保存输出字体集合的二维数组指针
int outlinelen;
int align_type,right2left;	
int smushmode;		//每个字符中间的合并模式
int smushoverride;	//覆盖合并模式 
int outputwidth;	//输出宽度
int outlinelenlimit;	//行输出宽度限制
char hardblank;		//空格标记符	
int charheight;		//字符高度
/******************************/


/******************************/
//程序中用到的函数体声明

//获取控制台参数 vc6
void processArgs(int argc,char *argv[]);
//读取字体文件
void readfont();
//读取字符图形到结构体
void readfontchar(FILE *file,long theord);
//去除文件前4个字符flf2
void readhead(FILE *fp,char *magic);
//用于跳过字体文本信息中作者文本行
void skiptoeol(FILE *fp);
//根据字符高度和输入的宽度分配空间
void linealloc();
//分割行 用于单词分断
void splitline();
//获取字符对应图形的宽度并且缓存上一次字符行宽
void getletter(long c);
//两字符边界模糊规则
char smushem(char lch,char rch);
//两字符位置偏移量
int smushamt();
//向输出字符集中加入字符
int addchar(long c);
//行输出
void printline();
//输出字符到控制台界面
void putstring(char *string);
//打印使用方法
void printusage(FILE *out);
//输出数组集第一列初始化函数
void clearline();
//初始化数组
void initArrayParams(char *ptr,int length);
/******************************/



/***********主函数实现************/

int main(int argc,char *argv[]){
	long c=0;		//处理的字符
	int i=0,len=0;
	int count=0;
	int wordbreakmode=0;
	int char_not_added=0;		//字符未加入输出数组标记
	char in_buffer[64]={0}; //输入内容
	processArgs(argc,argv);	//vc6.0获取控制台参数
	readfont();		//读取字体文件并加载字符图形到结构体
	linealloc();		//输出图形集合内存分配
	wordbreakmode = 0;	//分词模式默认0
	//初始化输入缓存数组
	for(count=0;count<64;count++){
			in_buffer[count]=0;
		}
	//主循环
	while(crycle){
		gets(in_buffer);	//获取控制台输入
		len=strlen(in_buffer);	
		if(strcmp(in_buffer,"quit")==0){	//quit退出
			exit(0);
		}else if(in_buffer[0]==0){		//ctrl+z退出
			exit(0);
		}
		for(count=0;count<len;count++){	//循环输入字符长度次数对每个字处理
			c=in_buffer[count];			//取出输入缓存数组对应循环次数值的字符
			if(isascii(c)&&isspace(c)){	//如果c是ascii字符集中的空格
				c = (c=='\t'||c==' ') ? ' ' : '\n';		//继而判断是定位符\t或者ascii值32为真返回空格
			}
			//如果c大于结束符且小于32且c不是换行或者c是del删除符
			//因其不在程序处理的范围,跳过这些控制符
			if((c>'\0' && c<' ' && c!='\n') || c==127) continue;

			do{
				char_not_added = 0;	//字符加入
				//字符分断处理判断
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
			}else if (c==' '){	//遇到空格且分断模式2 则处理 否则正好单词结束打印
				if (wordbreakmode==2){
					splitline();
				}else{
					printline();
				}
				wordbreakmode = -1;
			}else{
				if(wordbreakmode>=2){	//如果分段模式大于等于2执行
					splitline();
				}else{
					printline();
			  }
				wordbreakmode = (wordbreakmode==3)?1:0;
				char_not_added = 1;	//字符未加入
			}
		  }while (char_not_added);
		}
		//初始化输入数组
		for(count=0;count<64;count++){
			in_buffer[count]=0;
		}
		if(outlinelen!=0){
			printline();
		}
	}
	return 0;
}

/***************其他函数实现***********************/


//初始化数组
//传入数组指针 数组长度 置零数组每个元素
void initArrayParams(char *ptr,int length){
	int i=0;
	for(i=0;i<length;i++){
		*(ptr+i)=0;
	}
}

//打印使用方法
void printusage(FILE *out){
	fprintf(out,"Usage: [ -d 设置字体文件夹 ]\n");
	fprintf(out,
		"       [ -f 设置字体文件 ] [ -w 设置输出宽度 ] [-c,-l,-r 设置对齐方式]\n");
	fprintf(out,
		"       [ -i 显示信息 ]\n");
}

void processArgs(int argc,char *argv[]){
	int i=0,len=0;
	int columns=0;	//列宽 
	smushoverride = SMO_NO;
	align_type = -1;
	right2left = -1;
	outputwidth = DEFAULTCOLUMNS;	//输出宽度默认值
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
					printf("      *欢迎使用e-mail签名软件*\n");
					printf("        *请阅读下方使用方式*\n");
					printf("***********************************\n\n");
					printusage(stdout);
					exit(1);
					break;
			}
		}
	}
	outlinelenlimit = outputwidth-1;
}


//用于跳过字体文本信息中的作者文本行 调用一次跳过单行
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

//去除文件前4个字符flf2
void readhead(FILE *fp,char *magic){
	int i;
	for (i=0;i<4;i++) {
		magic[i] = fgetc(fp);
    }
	magic[4] = 0;
}

//将输出数组集第一列初始化
void clearline(){
	int i;
	for(i=0;i<charheight;i++){
		outputline[i][0] = '\0';
    }
	outlinelen = 0;
	longlinelen = 0;
}
//读取字符图形到结构体
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
//读取字体文件
void readfont(){
	int i,row,numsread;
	long theord;
	int maxlen,cmtlines,ffright2left;
	int smush,smush2;
	char fileline[MAXLEN+1],magicnum[5];	//用于保存flf2可以去除
	char fname[32]={0};
	FILE *fontfile;
	strcat(file_path,file_name);	//拼接文件名和父级文件夹形成路径
	fontfile = fopen(file_path,"r");
	//只读方式打开文件
	if(fontfile==NULL){
		fprintf(stderr,"%s: Unable to open font file,check filepath or filename\n",file_path);
		exit(1);
    }
	readhead(fontfile,magicnum);	//去除字体文件的flf2
	if(fgets(fileline,MAXLEN,fontfile)==NULL){
		fileline[0] = '\0';
    }
	//如果上次读取的fileline长度存在且fileline以换行结束执行跳过注释行
	if(strlen(fileline)>0 ? fileline[strlen(fileline)-1]!='\n' : 0){
		skiptoeol(fontfile);	
    }
	//读取文件头信息
	numsread = sscanf(fileline,"%*c%c %d %*d %d %d %d %d %d",&hardblank,&charheight,&maxlen,&smush,&cmtlines,&ffright2left,&smush2);
	
	if(maxlen > MAXLEN){
		fprintf(stderr,"%s: line character is too wide\n",file_path);
		exit(1);
    }
	//跳过cmtlines行注释行
	for(i=1;i<=cmtlines;i++){
		skiptoeol(fontfile);	//循环调用跳过作者信息行
    }	
	//如果读取到的数量小于6 即为老版本的字体文件 参数只有5个 那么从右向左打印为0
	if(numsread<6){
		ffright2left = 0;
    }

	if(numsread<7){ /* 如果没有新的规则参数 就将oldlayout转换成新规则参数*/
		if (smush == 0) smush2 = SM_KERN;
		else if (smush < 0) smush2 = 0;
		else smush2 = (smush & 31) | SM_SMUSH;
    }
	if(charheight<1){	//charheight maxlen 最小值为1
		charheight = 1;
    }

	if(maxlen<1){
		maxlen = 1;
    }
	maxlen += 100; /* maxlen留点余地可以按需调整 */
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

//根据字符高度和输入的宽度分配空间
void linealloc(){
	int row; //行
	//根据charheight分配行高
	outputline = (char**)malloc(sizeof(char*)*charheight);
	for (row=0;row<charheight;row++) {
		//每个行高分配列宽
		outputline[row] = (char*)malloc(sizeof(char)*(outlinelenlimit+1));
    }
	//行限制宽度
	longlinelenlimit = outputwidth*4+100;
	//输出的数组
	longline = (long*)malloc(sizeof(long)*(longlinelenlimit+1));

	clearline();
}
//获取字符对应图形的宽度并且缓存上一次字符行宽
void getletter(long c){
	fcharnode *charptr;		//声明一个字符结构体指针指向fcharlist首元素
	//遍历字符链表当ord指向的内容不为传入的c时 跳过前面的内容 即指针指向下一个数据
	for(charptr=fcharlist;charptr==NULL?0:charptr->ord!=c;charptr=charptr->next) ;
	//判断分支可去除,经过上面的循环charptr==NULL的情况很少
	if(charptr!=NULL){
		currchar = charptr->thechar;	//指向当前字符图形
    }else{
		for(charptr=fcharlist;charptr==NULL?0:charptr->ord!=0;charptr=charptr->next) ;
		currchar = charptr->thechar;
    }
	previouscharwidth = currcharwidth;	//保存前一个字符宽度
	currcharwidth = strlen(currchar[0]);	//刷新当前字符宽度为currchar
}

//两字符边界模糊规则
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
	return maxsmush;//4宽
}

//分割行 用于单词分断
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

//向输出字符集中加入字符
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

//行打印
void printline(){
	int i;
	for (i=0;i<charheight;i++) {
		putstring(outputline[i]);
    }
	clearline();
}

//输出字符到控制台界面
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
