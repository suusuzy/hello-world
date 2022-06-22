#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define true 1
#define false 0
#define processNum 5
#define resourceNum 3
#define MAXINT 9999

#define FILEMAXN 100
#define OPMAXN 100
#define NAMEMAXN 100
#define PATHMAXN 1000


typedef int bool;

int available[resourceNum]={3,3,2};
int maxRequest[processNum][resourceNum]={7,5,3,3,2,2,9,0,2,2,2,2,4,3,3};
int allocation[processNum][resourceNum]={0,1,0,2,0,0,3,0,2,2,1,1,0,0,2};
int need[processNum][resourceNum]={7,4,3,1,2,2,6,0,0,0,1,1,4,3,1};
bool Finish[processNum];
int safeSeries[processNum]={MAXINT,MAXINT,MAXINT,MAXINT,MAXINT};
int request[resourceNum];

typedef struct PCB{
    char name[20];
//  运行时间
    int running_time;
//   到达时间
    int enter_time;
//    优先级
    int priority;
    //完成时间
    int done_time;
    int copyRunning_time;  //用于时间片轮转
//  进程开始运行的时间
    int start_time;
    
    struct PCB* next;
} PCB;

typedef struct PCBQueue{
    PCB* firstProg;
    PCB* LastProg;
    int size;
} PCBQueue;


typedef struct FileNode{
    char name[NAMEMAXN]; //文件名或者文件夹名
    char path[PATHMAXN]; //文件（夹）路径
    int flag; //区分文件或者文件夹的标识->0: 文件夹，1: 文件
    char datetime[20]; //文件创建日期
    int filenumber; //文件夹包含的文件（夹）数量
    struct FileNode** files; //文件夹包含的文件（夹）列表
    struct FileNode* pNode; //上级目录
}FileNode, *FileTree;

//日期函数，用于记录文件创建日期
//本函数可以不用看，下面的一些函数比较重要
char* getDateTime(){
    char cur_time[20];
    time_t t;
    struct tm * lt;
    time(&t);
    lt = localtime(&t); //转为时间结构
    sprintf(cur_time, "%04d/%02d/%02d %02d:%02d:%02d",lt->tm_year+1900, lt->tm_mon, lt->tm_mday, lt->tm_hour, lt->tm_min, lt->tm_sec);
    return cur_time;
}
//初始化文件系统，即建立一个根目录ROOT节点
FileTree init(){
    FileTree root;
    root = (FileNode *)malloc(sizeof(FileNode)); //分配空间
    strcpy(root->name, "ROOT"); //字符串拷贝函数
    strcpy(root->path, "ROOT");
    root->flag = 0; //ROOT为目录节点，标识为0
    root->filenumber = 0; //初始时所含子文件（夹）数量为0
    root->files = NULL;
    root->pNode = NULL;
    return root;
}
//md:新建文件夹
void makeDirectory(FileNode *pNode, char dirname[]){
    if(pNode->files == NULL){ //首次使用要分配空间
        pNode->files = (FileNode**)malloc(FILEMAXN * sizeof(FileNode*));
    }
    pNode->files[pNode->filenumber] = (FileNode *)malloc(sizeof(FileNode));
    FileNode *dir = pNode->files[pNode->filenumber++];
    dir->flag = 0; //目录节点，标识为0
    strcpy(dir->name, dirname);
    dir->filenumber = 0;
    dir->files = NULL;
    dir->pNode = pNode;
    //以下代码生成当前路径
    char tmp[PATHMAXN];
    strcpy(tmp, pNode->path);
    strcat(tmp, "\\");
    strcat(tmp, dirname);
    strcpy(dir->path, tmp);

}
//mf:新建文件
void makeFile(FileNode *pNode, char filename[]){
    if(pNode->files == NULL){
        pNode->files = (FileNode**)malloc(FILEMAXN * sizeof(FileNode*));
    }
    pNode->files[pNode->filenumber] = (FileNode *)malloc(sizeof(FileNode));
    FileNode *dir = pNode->files[pNode->filenumber++];
    dir->flag = 1; //文件节点，标识为1
    strcpy(dir->name, filename);
    dir->filenumber = 0;
    dir->files = NULL;
    dir->pNode = pNode;
}
//dir:列出当前目录下全部文件（夹）
void showDir(FileNode *pNode){
    int i;
    //遍历打印文件名
    for(i = 0; i < pNode->filenumber; i++){
        printf("%s ", pNode->files[i]->name);
    }
    printf("\n");
}
//cd:切换到当前路径下的某个文件夹
FileNode* enterSubDir(FileNode *pNode, char dirname[]){
    FileNode* curNode = NULL;
    int i;
    for(i = 0; i < pNode->filenumber; i++){
        if(strcmp(pNode->files[i]->name, dirname) == 0
           && pNode->files[i]->flag == 0){
            //若有此文件夹，则将当前节点切换为此文件夹
            curNode = pNode->files[i];
        }
    }
    return curNode;
}
//del:删除文件（夹）
int delFile(FileNode *pNode, char dirname[]){
   int i;
   for(i = 0; i < pNode->filenumber; i++){
        if(strcmp(pNode->files[i]->name, dirname) == 0){
            free(pNode->files[i]); //释放空间
            int j = i;
            for(; j < pNode->filenumber - 1; j++){
                pNode->files[j] = pNode->files[j + 1]; //后面节点前移
            }
            pNode->filenumber --; //子文件（夹）数量减一
        }
    }
}
void wenjianshowInfo()
{
	printf("  ******************************************************\n\n");
	printf("  *                欢迎使用文件模拟系统                 *\n \n");
	printf("  ******************************************************\n\n");
	printf("  该系统目前支持如下功能：\n");
	printf("  1.切换到当前路径下的某文件夹: cd + 文件夹\n");
	printf("  2.切换到当前路径的上级文件夹: cd ..\n");
	printf("  3.在任意路径下切换回根路径ROOT: cd \\ \n");
	printf("  4.列出当前路径下的全部文件夹和文件: dir\n");
	printf("  5.在当前路径下新建文件夹: md + 文件夹名\n");
	printf("  6.在当前路径下新建文件: mf + 文件名\n");
	printf("  7.删除当前路径下的某文件(夹)名: del文件(夹)名\n");
	printf("  ******************************************************\n");
}

void Queueinit(PCBQueue* queue){
    if(queue==NULL){
        return;
    }
    queue->size = 0;
    queue->LastProg = (PCB*)malloc(sizeof(PCB));
    queue->firstProg = queue->LastProg;
}



void EnterQueue(PCBQueue* queue,PCB* pro){   //加入进程队列
    queue->LastProg->next = (PCB*)malloc(sizeof(PCB));
    queue->LastProg = queue->LastProg->next;
    queue->LastProg->enter_time = pro->enter_time;
//    将name复制给 LastProg
    memcpy(queue->LastProg->name,pro->name,sizeof(pro->name));
    queue->LastProg->priority = pro->priority;
    queue->LastProg->running_time = pro->running_time;
    queue->LastProg->copyRunning_time = pro->copyRunning_time;
    queue->LastProg->start_time = pro->start_time;
    queue->size++;
}
PCB* poll(PCBQueue* queue){
    PCB* temp = queue->firstProg->next;
    if(temp == queue->LastProg){
        queue->LastProg=queue->firstProg;
        queue->size--;
        return temp;
    }
    queue->firstProg->next = queue->firstProg->next->next;
    queue->size--;
    return temp;
}

void inputPCB(PCB pro[],int num){
    for(int i=0;i<num;i++){
        PCB prog ;
        printf("请输入第%d个进程的名字，到达时间 ，服务时间，优先级\n",i+1);
        scanf("%s",prog.name);
        scanf("%d",&prog.enter_time) ;
        scanf("%d",&prog.running_time);
        prog.copyRunning_time = prog.running_time;
        scanf("%d",&prog.priority);
        pro[i]=prog;
    }
}

//冒泡排序算法（每次找到最大的放在末尾）
void sortWithEnterTime(PCB pro[],int num){
    for(int i=1;i<num;i++){
        for(int j= 0;j<num-i;j++){
            if(pro[j].enter_time>pro[j+1].enter_time){
                PCB temp = pro[j];
                pro[j] = pro[j+1];
                pro[j+1] = temp;
            }
        }
    }
}

void FCFS(PCB pro[],int num){
    printf("进程 到达时间  服务时间 开始时间 完成时间 周转时间 带权周转时间\n");
    sortWithEnterTime(pro,num);    //按照进入顺序排序
    PCBQueue* queue = (PCBQueue*)malloc(sizeof(PCBQueue));
    Queueinit(queue);
    EnterQueue(queue,&pro[0]);
    int time = pro[0].enter_time;
    int pronum=1;    //记录当前的进程
    //平均周转时间
    float sum_T_time = 0;
//    带权平均周转时间
    float sum_QT_time = 0 ;

    while(queue->size>0){
        PCB* curpro = poll(queue);   //从进程队列中取出进程
        if(time < curpro->enter_time)
            time =  curpro->enter_time;
        //完成时间
        int done_time = time+curpro->running_time;
        //周转时间（作业完成的时间-作业到达的时间）
        int T_time = done_time - curpro->enter_time;
        sum_T_time += T_time;
        // 带权周转时间（（作业完成的时间-作业到达的时间）/ 作业运行时间）
        float QT_time = T_time / (curpro->running_time+0.0) ;
        sum_QT_time += QT_time;
        for(int tt = time;tt<=done_time && pronum<num;tt++){    //模拟进程的执行过程
            if(tt >= pro[pronum].enter_time){
                EnterQueue(queue,&pro[pronum]);
                pronum++;
            }
        }
        printf("%s\t%d\t%d\t%d\t%d\t%d\t%.2f\n",curpro->name,curpro->enter_time,curpro->running_time,time,done_time
               ,T_time,QT_time);
        time += curpro->running_time;
        if(queue->size==0 && pronum < num){   //防止出现前一个进程执行完到下一个进程到达之间无进程进入
            EnterQueue(queue,&pro[pronum]);
            pronum++;
        }
    }
    printf("平均周转时间为%.2f\t平均带权周转时间为%.2f\n",sum_T_time/(num+0.0),sum_QT_time/(num+0.0));
}

//按照运行时间排序
void sortWithLongth(PCB pro[],int start,int end){
    int len = end - start;
    if(len == 1)
        return;
    for(int i=1;i<len;i++){
        for(int j= start;j<end-i;j++){
            if(pro[j].running_time>pro[j+1].running_time){
                PCB temp = pro[j];
                pro[j] = pro[j+1];
                pro[j+1] = temp;
            }
        }
    }
}

void SJF(PCB pro[],int num) {
    printf("进程 到达时间  服务时间 开始时间 完成时间 周转时间 带权周转时间\n");
    sortWithEnterTime(pro,num);
    PCBQueue* queue = (PCBQueue*)malloc(sizeof(PCBQueue));;
    Queueinit(queue);
    EnterQueue(queue,&pro[0]);
    int time = pro[0].enter_time;
    int pronum=1;    //记录当前的进程
    float sum_T_time = 0,sum_QT_time = 0;
    while(queue->size>0){
        PCB* curpro = poll(queue);   //从进程队列中取出进程
        if(time <  curpro->enter_time)
            time =  curpro->enter_time;
        int done_time = time+curpro->running_time;
        int T_time = done_time - curpro->enter_time;
        float QT_time = T_time / (curpro->running_time+0.0) ;
        sum_T_time += T_time;
        sum_QT_time += QT_time;
        int pre = pronum;
        for(int tt = time;tt<=done_time&&pronum<num;tt++){    //模拟进程的执行过程
            if(tt>=pro[pronum].enter_time){ // 统计从此任务开始到结束之间有几个进程到达
                pronum++;
            }
        }
        sortWithLongth(pro,pre,pronum);//将到达的进程按照服务时间排序
        for(int i=pre;i<pronum;i++){    //将进程链入队列
            EnterQueue(queue,&pro[i]);
        }
        pre = pronum;
        printf("%s\t%d\t%d\t%d\t%d\t%d\t%.2f\n",curpro->name,curpro->enter_time,curpro->running_time,time,done_time
               ,T_time,QT_time);
        time +=  curpro->running_time;
        if(queue->size==0&&pronum<num){   //防止出现前一个进程执行完到下一个进程到达之间无进程进入
            EnterQueue(queue,&pro[pronum]);
            pronum++;
        }
    }
    printf("平均周转时间为%.2f\t平均带权周转时间为%.2f\n",sum_T_time/(num+0.0),sum_QT_time/num);
}
//按照响应比排序（倒序）
void sortWithResponse(PCB pro[],int start,int end){
    int len = end - start;
    if(len == 1)
        return;
    for(int i=1;i<len;i++){
        for(int j= start;j<end-i;j++){
            //计算响应比
            float m = (pro[j].start_time-pro[j].enter_time+pro[j].running_time)/(pro[j].running_time+0.0);
            float n = (pro[j+1].start_time-pro[j+1].enter_time+pro[j+1].running_time)/(pro[j+1].running_time+0.0);
            if(m < n){
                PCB temp = pro[j];
                pro[j] = pro[j+1];
                pro[j+1] = temp;
            }
        }
    }
}
//高响应比优先
void HRRN(PCB pro[],int num) {
    printf("进程 到达时间  服务时间 开始时间 完成时间 周转时间 带权周转时间\n");
    sortWithEnterTime(pro,num);
    PCBQueue* queue = (PCBQueue*)malloc(sizeof(PCBQueue));;
    Queueinit(queue);
    EnterQueue(queue,&pro[0]);
    int time = pro[0].enter_time;
    int pronum=1;    //记录当前的进程
    float sum_T_time = 0,sum_QT_time = 0;
    while(queue->size>0){
        PCB* curpro = poll(queue);   //从进程队列中取出进程
        if(time <  curpro->enter_time)
            time =  curpro->enter_time;
        int done_time = time+curpro->running_time;
        int T_time = done_time - curpro->enter_time;
        float QT_time = T_time / (curpro->running_time+0.0) ;
        sum_T_time += T_time;
        sum_QT_time += QT_time;
        int pre = pronum;
        for(int tt = time;tt<=done_time&&pronum<num;tt++){    //模拟进程的执行过程
            if(tt>=pro[pronum].enter_time){ // 统计从此任务开始到结束之间有几个进程到达
                pronum++;
            }
        }
        sortWithResponse(pro,pre,pronum);//将到达的进程按照响应时间排序
        for(int i=pre;i<pronum;i++){    //将进程链入队列
            EnterQueue(queue,&pro[i]);
        }
        pre = pronum;
        printf("%s\t%d\t%d\t%d\t%d\t%d\t%.2f\n",curpro->name,curpro->enter_time,curpro->running_time,time,done_time
               ,T_time,QT_time);
        time +=  curpro->running_time;
        if(queue->size==0&&pronum<num){   //防止出现前一个进程执行完到下一个进程到达之间无进程进入
            EnterQueue(queue,&pro[pronum]);
            pronum++;
        }
    }
    printf("平均周转时间为%.2f\t平均带权周转时间为%.2f\n",sum_T_time/(num+0.0),sum_QT_time/num);
}

//按权重排序
void sortWithPriority(PCB pro[],int start,int end){
    int len = end - start;
    if(len == 1) return ;
    for(int i=1;i<len;i++){
        for(int j= start;j<end-i;j++){
            if(pro[j].priority>pro[j+1].priority){
                PCB temp = pro[j];
                pro[j] = pro[j+1];
                pro[j+1] = temp;
            }
        }
    }
}
//优先级调度算法
void HPF(PCB pro[],int num){
    printf("进程 到达时间  服务时间 开始时间 完成时间 周转时间 带权周转时间\n");
    sortWithEnterTime(pro,num);
    PCBQueue* queue = (PCBQueue*)malloc(sizeof(PCBQueue));;
    Queueinit(queue);
    EnterQueue(queue,&pro[0]);
    int time = pro[0].enter_time;
    int pronum=1;    //记录当前的进程
    float sum_T_time = 0,sum_QT_time = 0;
    while(queue->size>0){
        PCB* curpro = poll(queue);   //从进程队列中取出进程
        if(time<curpro->enter_time)
            time =  curpro->enter_time;
        int done_time = time+curpro->running_time;
        int T_time = done_time - curpro->enter_time;
        float QT_time = T_time / (curpro->running_time+0.0) ;
        sum_T_time += T_time;
        sum_QT_time += QT_time;
        int pre = pronum;
        for(int tt = time;tt<=done_time&&pronum<num;tt++){    //模拟进程的执行过程
            if(tt>=pro[pronum].enter_time){ // 统计从此任务开始到结束之间有几个进程到达
                pronum++;
            }
        }
        sortWithPriority(pro,pre,pronum);//将到达的进程按照服务时间排序
        for(int i=pre;i<pronum;i++){    //将进程链入队列
            EnterQueue(queue,&pro[i]);
        }
        pre = pronum;
        printf("%s\t%d\t%d\t%d\t%d\t%d\t%.2f\n",curpro->name,curpro->enter_time,curpro->running_time,time,done_time
               ,T_time,QT_time);
        time +=  curpro->running_time;
        if(queue->size==0&&pronum<num){   //防止出现前一个进程执行完到下一个进程到达之间无进程进入
            EnterQueue(queue,&pro[pronum]);
            pronum++;
        }
    }
    printf("平均周转时间为%.2f\t平均带权周转时间为%.2f\n",sum_T_time/(num+0.0),sum_QT_time/(num+0.0));
}
//时间片轮转调度
void RR(PCB pro[],int num){
    printf("请输入时间片大小");
    int timeslice;
    scanf("%d",&timeslice);
    printf("进程 到达时间 服务时间 进入时间 完成时间 周转时间 带权周转时间\n");
    sortWithEnterTime(pro,num);
    PCBQueue* queue = (PCBQueue*)malloc(sizeof(PCBQueue));;
    Queueinit(queue);
    //第一个进程开始运行的时间就是到达时间
    pro[0].start_time = pro[0].enter_time;
    EnterQueue(queue,&pro[0]);
    int time = 0;
    int pronum = 1;
    float sum_T_time = 0,sum_QT_time = 0;
    while(queue->size>0){
        PCB* curpro = poll(queue);    // 从队列中取出头节点
        if(time<curpro->enter_time)
            time = curpro->enter_time;
        if(timeslice >= curpro->running_time){   // 如果剩余时间小于时间片  则此任务完成
            for(int tt = time;tt<=time+curpro->running_time&&pronum<num;tt++){    // 模拟进程的执行过程
                if(tt>=pro[pronum].enter_time){ // 统计从此任务开始到结束之间有几个进程到达
                    pro[pronum].start_time = tt;
                    EnterQueue(queue,&pro[pronum]);
                    pronum++;
                }
            }
            time += curpro->running_time;
            curpro->running_time = 0;
            curpro->done_time = time;
            int T_time = curpro->done_time-curpro->start_time;
            float QT_time = T_time / (curpro->copyRunning_time+0.0);
            sum_T_time += T_time;
            sum_QT_time += QT_time;
            printf("%s\t%d\t%d\t  %d\t   %d\t %d\t  %.2f\n",curpro->name,curpro->enter_time,curpro->copyRunning_time,
                   curpro->start_time,curpro->done_time,T_time,QT_time);
            if(queue->size==0&&pronum<num){   //防止出现前一个进程执行完到下一个进程到达之间无进程进入
                pro[pronum].start_time = pro[pronum].enter_time;
                EnterQueue(queue,&pro[pronum]);
                pronum++;
            }
            continue;
        }
        // 运行时间大于时间片
        for(int tt = time;tt<=time+timeslice&&pronum<num;tt++){    //模拟进程的执行过程
            if(tt>=pro[pronum].enter_time){ // 统计从此任务开始到结束之间有几个进程到达
                pro[pronum].start_time = tt;
                EnterQueue(queue,&pro[pronum]);
                pronum++;
            }
        }
        time += timeslice;
        curpro->running_time -= timeslice;
        //当前程序未完成  继续添加到队列中
        EnterQueue(queue,curpro);
        if(queue->size==0&&pronum<num){   //防止出现前一个进程执行完到下一个进程到达之间无进程进入
            pro[pronum].start_time = pro[pronum].enter_time;
            EnterQueue(queue,&pro[pronum]);
            pronum++;
        }
    }
    printf("平均周转时间为%.2f\t平均带权周转时间为%.2f\n\n",sum_T_time/(num+0.0),sum_QT_time/(num+0.0));
}
void choiceMenu(){
    printf("请选择进程调度算法：\n\n");
    printf("1.先来先服务算法\n2.短进程优先算法\n3.高优先级优先\n4.时间片轮转算法\n5.高响应比优先算法\n6.退出\n");
}
void menu(){
    int proNum;
    printf("请输入进程的个数：");
    scanf("%d",&proNum);
    PCB pro[proNum];
    inputPCB(pro,proNum);
    choiceMenu();
    int choice;
    while(1){
        scanf("%d",&choice);
        switch(choice){
            case 1:FCFS(pro,proNum);choiceMenu();break;
            case 2:SJF(pro,proNum);choiceMenu();break;
            case 3:HPF(pro,proNum);choiceMenu();break;
            case 4:RR(pro,proNum);choiceMenu();break;
            case 5:HRRN(pro,proNum);choiceMenu();break;
            case 6:return;
        }
    }
}


void sisuoInit()
{
	int i,j;
	printf("输入进程数量、资源数量\n");
	//scanf("%d %d",&processNum,&resourceNum);
	printf("输入当前资源可用数目\n");
	for(i=0;i<resourceNum;i++){
		scanf("%d",&available[i]);
	}
	printf("输入最大需求矩阵\n");
	for(i=0;i<processNum;i++){
		for(j=0;j<resourceNum;j++){
			scanf("%d",&maxRequest[i][j]);
		}
	}
	
	printf("输入分配矩阵\n");
	for(i=0;i<processNum;i++){
		for(j=0;j<resourceNum;j++){
			scanf("%d",&allocation[i][j]);
		}
	}
		
	printf("输入当前最大需求矩阵\n");
	for(i=0;i<processNum;i++){
		for(j=0;j<resourceNum;j++){
			scanf("%d",&allocation[i][j]);
		}
	}
}


void showInfo()
{
	int i,j;
	printf("当前剩余资源：");
	for(j=0;j<resourceNum;j++){
		printf("%d ",available[j]);
	}
	printf("\n");
	printf("PID\t MAX\t\tAllocation\tNeed\n");
	for(i=0;i<processNum;i++)
	{
		printf("p%d\t",i);
		for(j=0;j<resourceNum;j++){
			printf("%d ",maxRequest[i][j]);}
		printf("\t\t");
		for(j=0;j<resourceNum;j++){
			printf("%d ",allocation[i][j]);}
		printf("\t\t");
		for(j=0;j<resourceNum;j++){
			printf("%d ",need[i][j]);}
		printf("\n");	
	}
}
void SafeInfo(int *work,int i)
{
	int j;
	printf(" P%d\t",i);
	for(j=0;j<resourceNum;j++){
		printf("%d ",work[j]);
	}
	printf("\t\t");
	
	for(j=0;j<resourceNum;j++){
		printf("%d ",need[i][j]);
	}
	printf("\t");
	
	for(j=0;j<resourceNum;j++){
		printf("%d ",allocation[i][j]);
	}
	printf("\t\t");
	
	for(j=0;j<resourceNum;j++){
		printf("%d ",allocation[i][j]+work[j]);
	}
	printf("\n");
}

bool isSafe()
{
	int i,j,k;
	int trueFinished=0;
	int work[resourceNum];
	for(i=0;i<resourceNum;i++){
		work[i]=available[i];
	}
	
	for(i=0;i<processNum;i++){
		Finish[i]=false;
	}
	i=0;
	int temp=0;
	while(trueFinished !=processNum)
	{
		int j=0;
		if(Finish[i]!=true){
			for(j=0;j<resourceNum;j++){
				if(need[i][j]>work[j])
				{break;}
			}
		}
		
		if(j==resourceNum){
			Finish[i]=true;
			SafeInfo(work,i);
			for(k=0;k<resourceNum;k++){
				work[k]+=allocation[i][k];
			}
			int k2;
			safeSeries[trueFinished++]=i;
		}
		
		i++;
		
		if(i>=processNum)
		{
			i=i%processNum;
			if(temp==trueFinished)
				break;
		}
		
		temp=trueFinished;
	
	}
	
	if(trueFinished==processNum){
		printf("\n系统安全！\n\n安全序列为：");
		for(i=0;i<processNum;i++){
			printf("%d",safeSeries[i]);
		}
		return true;
	}
	printf("************系统不安全！**************\n");
	return false;
}




int main()
{	int openkey;
	int openkey3;
	int openkey6;
	while(1)
	{

	printf("***************************操作系统课设*************************************\n");
	printf("********************张謇202徐权杰+张謇202闫苏园*****************************\n");
	printf("---------------------欢迎使用该系统！--------------------------\n");
	printf("-------------------------菜单-------------------------------\n");
	printf("1.模拟各种处理机调度算法\n");
	printf("2.模拟各种主存分配算法及回收过程，并能动态演示\n");
	printf("3.模拟死锁的判断和解决，并能动态演示\n");
	printf("4.处理机调度算法与分配主存相结合\n");
	printf("5.多用户登陆管理\n");
	printf("6.文件管理系统\n");
	printf("7.磁盘管理\n");
	
	scanf("%d",&openkey);
	if(openkey==1){
		menu();	
	}
	
	else if(openkey==2){
		break;
	}
	
	
	else if(openkey==3){
		int i,j,curProcess;
		int wheInit=0;
		printf("是否使用内置数据？0是，1否：");
		scanf("%d",&wheInit);
		if(wheInit)
			sisuoInit();//可以不使用，选用内置的数据进行测试
		printf("-------------------------------------------------------------\n");
		showInfo();
		printf("\n系统安全情况分析\n");
		printf("PID\t Work\t\tNeed\tAllocation\tWork+Allocation\n");
		isSafe();
		while(true){
			printf("\n--------------------------------------------------------\n");
			printf("\n输入要分配的进程：");
			scanf("%d",&curProcess);
			printf("\n输入要分配给进程P%d的资源：",curProcess);
			for(j=0;j<resourceNum;j++)
			{
				scanf("%d",&request[j]);
			}
			for(j=0;j<resourceNum;j++){
				if(request[j]<=need[curProcess][j]) continue;
				else{printf("ERROR!\n");break;}
			}
		
			if(j==resourceNum){
				for(j=0;j<resourceNum;j++){
					if(request[j]<=need[curProcess][j]) continue;
					else{printf("资源不足，等待中！\n");break;}	
				}
			if(j==resourceNum){
				for(j=0;j<resourceNum;j++){
					available[j]-=request[j];
					allocation[curProcess][j]+=request[j];
					need[curProcess][j]-=request[j];
				}
				printf("\n系统安全情况分析\n");
				printf(" PID\t Work\t\tNeed\tAllocation\tWork+Allocation\n");
				if(isSafe()){
					printf("分配成功！\n");
					showInfo();
				}
				else{
					for(j=0;j<resourceNum;j++){
						available[j]+=request[j];
						allocation[curProcess][j]-=request[j];
						need[curProcess][j]+=request[j];
					}
					printf("分配失败！\n");
					showInfo();
				}
			}
			
		}	
		printf("是否需要回到主菜单？0：否；1：是\n");
		scanf("%d",&openkey3);
		if(openkey3==1)
			break;
	}
	}
	
	else if(openkey==6){
		wenjianshowInfo();
    	FileTree rootDir = init(); //初始化文件系统，
    	FileNode *curDir = rootDir; //切换当前节点为根节点
    	char curPath[PATHMAXN] = "ROOT"; //切换当前路径为根路径
    	printf("%s> ", curPath); //输出 ROOT>
    	char operation[OPMAXN];
    	while(gets(operation)!= EOF){ //获取输入
        	char *op = strtok(operation, " ");
        	char *arg = strtok(NULL, " "); //以上分割出操作命令和参数，比如cd music，则命令为cd，参数为music
        	if(strcmp(op, "md") == 0){ //md
            	makeDirectory(curDir, arg);
        	}else if(strcmp(op, "mf") == 0){ //mf
            	makeFile(curDir, arg);
        	}else if(strcmp(op, "del") == 0){ //del
            	delFile(curDir, arg);
        	}else if(strcmp(op, "dir") == 0){ //dir
            	showDir(curDir);
        	}else if(strcmp(op, "cd") == 0){ //cd
            	if(strcmp(arg, "..") == 0){ //cd ..
                curDir = curDir->pNode;
            }else if(strcmp(arg, "\\") == 0){ //cd \ ---切换为根目录
                curDir = rootDir;
            }else{ //cd music --进入子目录
                FileNode *pNode = curDir;
                curDir = enterSubDir(curDir, arg);
                if(!curDir){ //出现未知目录名，报错
                    printf("ERROR:\"%s\"目录下没有\"%s\", 请重新输入!\n",pNode->name, arg);
                    curDir = pNode;
                    printf("%s> ", curPath);
                    continue;
                }
            }
            strcpy(curPath, curDir->path);
        }else if(strcmp(op, "exit") == 0){ //exit 退出
            printf("已退出系统, 谢谢使用!\n", op);
            break;
        }else{ // 出现未知命令，报错
            printf("ERROR:不支持\"%s\"命令, 请重新输入!\n", op);
        }
        printf("%s> ", curPath);
        
        //printf("是否需要回到主菜单？0：否；1：是\n");
		//scanf("%d",&openkey6);
		//if(openkey6==1)
			//break;
    }
		
	}
	
	}
	
}