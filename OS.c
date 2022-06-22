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
//  ����ʱ��
    int running_time;
//   ����ʱ��
    int enter_time;
//    ���ȼ�
    int priority;
    //���ʱ��
    int done_time;
    int copyRunning_time;  //����ʱ��Ƭ��ת
//  ���̿�ʼ���е�ʱ��
    int start_time;
    
    struct PCB* next;
} PCB;

typedef struct PCBQueue{
    PCB* firstProg;
    PCB* LastProg;
    int size;
} PCBQueue;


typedef struct FileNode{
    char name[NAMEMAXN]; //�ļ��������ļ�����
    char path[PATHMAXN]; //�ļ����У�·��
    int flag; //�����ļ������ļ��еı�ʶ->0: �ļ��У�1: �ļ�
    char datetime[20]; //�ļ���������
    int filenumber; //�ļ��а������ļ����У�����
    struct FileNode** files; //�ļ��а������ļ����У��б�
    struct FileNode* pNode; //�ϼ�Ŀ¼
}FileNode, *FileTree;

//���ں��������ڼ�¼�ļ���������
//���������Բ��ÿ��������һЩ�����Ƚ���Ҫ
char* getDateTime(){
    char cur_time[20];
    time_t t;
    struct tm * lt;
    time(&t);
    lt = localtime(&t); //תΪʱ��ṹ
    sprintf(cur_time, "%04d/%02d/%02d %02d:%02d:%02d",lt->tm_year+1900, lt->tm_mon, lt->tm_mday, lt->tm_hour, lt->tm_min, lt->tm_sec);
    return cur_time;
}
//��ʼ���ļ�ϵͳ��������һ����Ŀ¼ROOT�ڵ�
FileTree init(){
    FileTree root;
    root = (FileNode *)malloc(sizeof(FileNode)); //����ռ�
    strcpy(root->name, "ROOT"); //�ַ�����������
    strcpy(root->path, "ROOT");
    root->flag = 0; //ROOTΪĿ¼�ڵ㣬��ʶΪ0
    root->filenumber = 0; //��ʼʱ�������ļ����У�����Ϊ0
    root->files = NULL;
    root->pNode = NULL;
    return root;
}
//md:�½��ļ���
void makeDirectory(FileNode *pNode, char dirname[]){
    if(pNode->files == NULL){ //�״�ʹ��Ҫ����ռ�
        pNode->files = (FileNode**)malloc(FILEMAXN * sizeof(FileNode*));
    }
    pNode->files[pNode->filenumber] = (FileNode *)malloc(sizeof(FileNode));
    FileNode *dir = pNode->files[pNode->filenumber++];
    dir->flag = 0; //Ŀ¼�ڵ㣬��ʶΪ0
    strcpy(dir->name, dirname);
    dir->filenumber = 0;
    dir->files = NULL;
    dir->pNode = pNode;
    //���´������ɵ�ǰ·��
    char tmp[PATHMAXN];
    strcpy(tmp, pNode->path);
    strcat(tmp, "\\");
    strcat(tmp, dirname);
    strcpy(dir->path, tmp);

}
//mf:�½��ļ�
void makeFile(FileNode *pNode, char filename[]){
    if(pNode->files == NULL){
        pNode->files = (FileNode**)malloc(FILEMAXN * sizeof(FileNode*));
    }
    pNode->files[pNode->filenumber] = (FileNode *)malloc(sizeof(FileNode));
    FileNode *dir = pNode->files[pNode->filenumber++];
    dir->flag = 1; //�ļ��ڵ㣬��ʶΪ1
    strcpy(dir->name, filename);
    dir->filenumber = 0;
    dir->files = NULL;
    dir->pNode = pNode;
}
//dir:�г���ǰĿ¼��ȫ���ļ����У�
void showDir(FileNode *pNode){
    int i;
    //������ӡ�ļ���
    for(i = 0; i < pNode->filenumber; i++){
        printf("%s ", pNode->files[i]->name);
    }
    printf("\n");
}
//cd:�л�����ǰ·���µ�ĳ���ļ���
FileNode* enterSubDir(FileNode *pNode, char dirname[]){
    FileNode* curNode = NULL;
    int i;
    for(i = 0; i < pNode->filenumber; i++){
        if(strcmp(pNode->files[i]->name, dirname) == 0
           && pNode->files[i]->flag == 0){
            //���д��ļ��У��򽫵�ǰ�ڵ��л�Ϊ���ļ���
            curNode = pNode->files[i];
        }
    }
    return curNode;
}
//del:ɾ���ļ����У�
int delFile(FileNode *pNode, char dirname[]){
   int i;
   for(i = 0; i < pNode->filenumber; i++){
        if(strcmp(pNode->files[i]->name, dirname) == 0){
            free(pNode->files[i]); //�ͷſռ�
            int j = i;
            for(; j < pNode->filenumber - 1; j++){
                pNode->files[j] = pNode->files[j + 1]; //����ڵ�ǰ��
            }
            pNode->filenumber --; //���ļ����У�������һ
        }
    }
}
void wenjianshowInfo()
{
	printf("  ******************************************************\n\n");
	printf("  *                ��ӭʹ���ļ�ģ��ϵͳ                 *\n \n");
	printf("  ******************************************************\n\n");
	printf("  ��ϵͳĿǰ֧�����¹��ܣ�\n");
	printf("  1.�л�����ǰ·���µ�ĳ�ļ���: cd + �ļ���\n");
	printf("  2.�л�����ǰ·�����ϼ��ļ���: cd ..\n");
	printf("  3.������·�����л��ظ�·��ROOT: cd \\ \n");
	printf("  4.�г���ǰ·���µ�ȫ���ļ��к��ļ�: dir\n");
	printf("  5.�ڵ�ǰ·�����½��ļ���: md + �ļ�����\n");
	printf("  6.�ڵ�ǰ·�����½��ļ�: mf + �ļ���\n");
	printf("  7.ɾ����ǰ·���µ�ĳ�ļ�(��)��: del�ļ�(��)��\n");
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



void EnterQueue(PCBQueue* queue,PCB* pro){   //������̶���
    queue->LastProg->next = (PCB*)malloc(sizeof(PCB));
    queue->LastProg = queue->LastProg->next;
    queue->LastProg->enter_time = pro->enter_time;
//    ��name���Ƹ� LastProg
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
        printf("�������%d�����̵����֣�����ʱ�� ������ʱ�䣬���ȼ�\n",i+1);
        scanf("%s",prog.name);
        scanf("%d",&prog.enter_time) ;
        scanf("%d",&prog.running_time);
        prog.copyRunning_time = prog.running_time;
        scanf("%d",&prog.priority);
        pro[i]=prog;
    }
}

//ð�������㷨��ÿ���ҵ����ķ���ĩβ��
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
    printf("���� ����ʱ��  ����ʱ�� ��ʼʱ�� ���ʱ�� ��תʱ�� ��Ȩ��תʱ��\n");
    sortWithEnterTime(pro,num);    //���ս���˳������
    PCBQueue* queue = (PCBQueue*)malloc(sizeof(PCBQueue));
    Queueinit(queue);
    EnterQueue(queue,&pro[0]);
    int time = pro[0].enter_time;
    int pronum=1;    //��¼��ǰ�Ľ���
    //ƽ����תʱ��
    float sum_T_time = 0;
//    ��Ȩƽ����תʱ��
    float sum_QT_time = 0 ;

    while(queue->size>0){
        PCB* curpro = poll(queue);   //�ӽ��̶�����ȡ������
        if(time < curpro->enter_time)
            time =  curpro->enter_time;
        //���ʱ��
        int done_time = time+curpro->running_time;
        //��תʱ�䣨��ҵ��ɵ�ʱ��-��ҵ�����ʱ�䣩
        int T_time = done_time - curpro->enter_time;
        sum_T_time += T_time;
        // ��Ȩ��תʱ�䣨����ҵ��ɵ�ʱ��-��ҵ�����ʱ�䣩/ ��ҵ����ʱ�䣩
        float QT_time = T_time / (curpro->running_time+0.0) ;
        sum_QT_time += QT_time;
        for(int tt = time;tt<=done_time && pronum<num;tt++){    //ģ����̵�ִ�й���
            if(tt >= pro[pronum].enter_time){
                EnterQueue(queue,&pro[pronum]);
                pronum++;
            }
        }
        printf("%s\t%d\t%d\t%d\t%d\t%d\t%.2f\n",curpro->name,curpro->enter_time,curpro->running_time,time,done_time
               ,T_time,QT_time);
        time += curpro->running_time;
        if(queue->size==0 && pronum < num){   //��ֹ����ǰһ������ִ���굽��һ�����̵���֮���޽��̽���
            EnterQueue(queue,&pro[pronum]);
            pronum++;
        }
    }
    printf("ƽ����תʱ��Ϊ%.2f\tƽ����Ȩ��תʱ��Ϊ%.2f\n",sum_T_time/(num+0.0),sum_QT_time/(num+0.0));
}

//��������ʱ������
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
    printf("���� ����ʱ��  ����ʱ�� ��ʼʱ�� ���ʱ�� ��תʱ�� ��Ȩ��תʱ��\n");
    sortWithEnterTime(pro,num);
    PCBQueue* queue = (PCBQueue*)malloc(sizeof(PCBQueue));;
    Queueinit(queue);
    EnterQueue(queue,&pro[0]);
    int time = pro[0].enter_time;
    int pronum=1;    //��¼��ǰ�Ľ���
    float sum_T_time = 0,sum_QT_time = 0;
    while(queue->size>0){
        PCB* curpro = poll(queue);   //�ӽ��̶�����ȡ������
        if(time <  curpro->enter_time)
            time =  curpro->enter_time;
        int done_time = time+curpro->running_time;
        int T_time = done_time - curpro->enter_time;
        float QT_time = T_time / (curpro->running_time+0.0) ;
        sum_T_time += T_time;
        sum_QT_time += QT_time;
        int pre = pronum;
        for(int tt = time;tt<=done_time&&pronum<num;tt++){    //ģ����̵�ִ�й���
            if(tt>=pro[pronum].enter_time){ // ͳ�ƴӴ�����ʼ������֮���м������̵���
                pronum++;
            }
        }
        sortWithLongth(pro,pre,pronum);//������Ľ��̰��շ���ʱ������
        for(int i=pre;i<pronum;i++){    //�������������
            EnterQueue(queue,&pro[i]);
        }
        pre = pronum;
        printf("%s\t%d\t%d\t%d\t%d\t%d\t%.2f\n",curpro->name,curpro->enter_time,curpro->running_time,time,done_time
               ,T_time,QT_time);
        time +=  curpro->running_time;
        if(queue->size==0&&pronum<num){   //��ֹ����ǰһ������ִ���굽��һ�����̵���֮���޽��̽���
            EnterQueue(queue,&pro[pronum]);
            pronum++;
        }
    }
    printf("ƽ����תʱ��Ϊ%.2f\tƽ����Ȩ��תʱ��Ϊ%.2f\n",sum_T_time/(num+0.0),sum_QT_time/num);
}
//������Ӧ�����򣨵���
void sortWithResponse(PCB pro[],int start,int end){
    int len = end - start;
    if(len == 1)
        return;
    for(int i=1;i<len;i++){
        for(int j= start;j<end-i;j++){
            //������Ӧ��
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
//����Ӧ������
void HRRN(PCB pro[],int num) {
    printf("���� ����ʱ��  ����ʱ�� ��ʼʱ�� ���ʱ�� ��תʱ�� ��Ȩ��תʱ��\n");
    sortWithEnterTime(pro,num);
    PCBQueue* queue = (PCBQueue*)malloc(sizeof(PCBQueue));;
    Queueinit(queue);
    EnterQueue(queue,&pro[0]);
    int time = pro[0].enter_time;
    int pronum=1;    //��¼��ǰ�Ľ���
    float sum_T_time = 0,sum_QT_time = 0;
    while(queue->size>0){
        PCB* curpro = poll(queue);   //�ӽ��̶�����ȡ������
        if(time <  curpro->enter_time)
            time =  curpro->enter_time;
        int done_time = time+curpro->running_time;
        int T_time = done_time - curpro->enter_time;
        float QT_time = T_time / (curpro->running_time+0.0) ;
        sum_T_time += T_time;
        sum_QT_time += QT_time;
        int pre = pronum;
        for(int tt = time;tt<=done_time&&pronum<num;tt++){    //ģ����̵�ִ�й���
            if(tt>=pro[pronum].enter_time){ // ͳ�ƴӴ�����ʼ������֮���м������̵���
                pronum++;
            }
        }
        sortWithResponse(pro,pre,pronum);//������Ľ��̰�����Ӧʱ������
        for(int i=pre;i<pronum;i++){    //�������������
            EnterQueue(queue,&pro[i]);
        }
        pre = pronum;
        printf("%s\t%d\t%d\t%d\t%d\t%d\t%.2f\n",curpro->name,curpro->enter_time,curpro->running_time,time,done_time
               ,T_time,QT_time);
        time +=  curpro->running_time;
        if(queue->size==0&&pronum<num){   //��ֹ����ǰһ������ִ���굽��һ�����̵���֮���޽��̽���
            EnterQueue(queue,&pro[pronum]);
            pronum++;
        }
    }
    printf("ƽ����תʱ��Ϊ%.2f\tƽ����Ȩ��תʱ��Ϊ%.2f\n",sum_T_time/(num+0.0),sum_QT_time/num);
}

//��Ȩ������
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
//���ȼ������㷨
void HPF(PCB pro[],int num){
    printf("���� ����ʱ��  ����ʱ�� ��ʼʱ�� ���ʱ�� ��תʱ�� ��Ȩ��תʱ��\n");
    sortWithEnterTime(pro,num);
    PCBQueue* queue = (PCBQueue*)malloc(sizeof(PCBQueue));;
    Queueinit(queue);
    EnterQueue(queue,&pro[0]);
    int time = pro[0].enter_time;
    int pronum=1;    //��¼��ǰ�Ľ���
    float sum_T_time = 0,sum_QT_time = 0;
    while(queue->size>0){
        PCB* curpro = poll(queue);   //�ӽ��̶�����ȡ������
        if(time<curpro->enter_time)
            time =  curpro->enter_time;
        int done_time = time+curpro->running_time;
        int T_time = done_time - curpro->enter_time;
        float QT_time = T_time / (curpro->running_time+0.0) ;
        sum_T_time += T_time;
        sum_QT_time += QT_time;
        int pre = pronum;
        for(int tt = time;tt<=done_time&&pronum<num;tt++){    //ģ����̵�ִ�й���
            if(tt>=pro[pronum].enter_time){ // ͳ�ƴӴ�����ʼ������֮���м������̵���
                pronum++;
            }
        }
        sortWithPriority(pro,pre,pronum);//������Ľ��̰��շ���ʱ������
        for(int i=pre;i<pronum;i++){    //�������������
            EnterQueue(queue,&pro[i]);
        }
        pre = pronum;
        printf("%s\t%d\t%d\t%d\t%d\t%d\t%.2f\n",curpro->name,curpro->enter_time,curpro->running_time,time,done_time
               ,T_time,QT_time);
        time +=  curpro->running_time;
        if(queue->size==0&&pronum<num){   //��ֹ����ǰһ������ִ���굽��һ�����̵���֮���޽��̽���
            EnterQueue(queue,&pro[pronum]);
            pronum++;
        }
    }
    printf("ƽ����תʱ��Ϊ%.2f\tƽ����Ȩ��תʱ��Ϊ%.2f\n",sum_T_time/(num+0.0),sum_QT_time/(num+0.0));
}
//ʱ��Ƭ��ת����
void RR(PCB pro[],int num){
    printf("������ʱ��Ƭ��С");
    int timeslice;
    scanf("%d",&timeslice);
    printf("���� ����ʱ�� ����ʱ�� ����ʱ�� ���ʱ�� ��תʱ�� ��Ȩ��תʱ��\n");
    sortWithEnterTime(pro,num);
    PCBQueue* queue = (PCBQueue*)malloc(sizeof(PCBQueue));;
    Queueinit(queue);
    //��һ�����̿�ʼ���е�ʱ����ǵ���ʱ��
    pro[0].start_time = pro[0].enter_time;
    EnterQueue(queue,&pro[0]);
    int time = 0;
    int pronum = 1;
    float sum_T_time = 0,sum_QT_time = 0;
    while(queue->size>0){
        PCB* curpro = poll(queue);    // �Ӷ�����ȡ��ͷ�ڵ�
        if(time<curpro->enter_time)
            time = curpro->enter_time;
        if(timeslice >= curpro->running_time){   // ���ʣ��ʱ��С��ʱ��Ƭ  ����������
            for(int tt = time;tt<=time+curpro->running_time&&pronum<num;tt++){    // ģ����̵�ִ�й���
                if(tt>=pro[pronum].enter_time){ // ͳ�ƴӴ�����ʼ������֮���м������̵���
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
            if(queue->size==0&&pronum<num){   //��ֹ����ǰһ������ִ���굽��һ�����̵���֮���޽��̽���
                pro[pronum].start_time = pro[pronum].enter_time;
                EnterQueue(queue,&pro[pronum]);
                pronum++;
            }
            continue;
        }
        // ����ʱ�����ʱ��Ƭ
        for(int tt = time;tt<=time+timeslice&&pronum<num;tt++){    //ģ����̵�ִ�й���
            if(tt>=pro[pronum].enter_time){ // ͳ�ƴӴ�����ʼ������֮���м������̵���
                pro[pronum].start_time = tt;
                EnterQueue(queue,&pro[pronum]);
                pronum++;
            }
        }
        time += timeslice;
        curpro->running_time -= timeslice;
        //��ǰ����δ���  ������ӵ�������
        EnterQueue(queue,curpro);
        if(queue->size==0&&pronum<num){   //��ֹ����ǰһ������ִ���굽��һ�����̵���֮���޽��̽���
            pro[pronum].start_time = pro[pronum].enter_time;
            EnterQueue(queue,&pro[pronum]);
            pronum++;
        }
    }
    printf("ƽ����תʱ��Ϊ%.2f\tƽ����Ȩ��תʱ��Ϊ%.2f\n\n",sum_T_time/(num+0.0),sum_QT_time/(num+0.0));
}
void choiceMenu(){
    printf("��ѡ����̵����㷨��\n\n");
    printf("1.�����ȷ����㷨\n2.�̽��������㷨\n3.�����ȼ�����\n4.ʱ��Ƭ��ת�㷨\n5.����Ӧ�������㷨\n6.�˳�\n");
}
void menu(){
    int proNum;
    printf("��������̵ĸ�����");
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
	printf("���������������Դ����\n");
	//scanf("%d %d",&processNum,&resourceNum);
	printf("���뵱ǰ��Դ������Ŀ\n");
	for(i=0;i<resourceNum;i++){
		scanf("%d",&available[i]);
	}
	printf("��������������\n");
	for(i=0;i<processNum;i++){
		for(j=0;j<resourceNum;j++){
			scanf("%d",&maxRequest[i][j]);
		}
	}
	
	printf("����������\n");
	for(i=0;i<processNum;i++){
		for(j=0;j<resourceNum;j++){
			scanf("%d",&allocation[i][j]);
		}
	}
		
	printf("���뵱ǰ����������\n");
	for(i=0;i<processNum;i++){
		for(j=0;j<resourceNum;j++){
			scanf("%d",&allocation[i][j]);
		}
	}
}


void showInfo()
{
	int i,j;
	printf("��ǰʣ����Դ��");
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
		printf("\nϵͳ��ȫ��\n\n��ȫ����Ϊ��");
		for(i=0;i<processNum;i++){
			printf("%d",safeSeries[i]);
		}
		return true;
	}
	printf("************ϵͳ����ȫ��**************\n");
	return false;
}




int main()
{	int openkey;
	int openkey3;
	int openkey6;
	while(1)
	{

	printf("***************************����ϵͳ����*************************************\n");
	printf("********************����202��Ȩ��+����202����԰*****************************\n");
	printf("---------------------��ӭʹ�ø�ϵͳ��--------------------------\n");
	printf("-------------------------�˵�-------------------------------\n");
	printf("1.ģ����ִ���������㷨\n");
	printf("2.ģ�������������㷨�����չ��̣����ܶ�̬��ʾ\n");
	printf("3.ģ���������жϺͽ�������ܶ�̬��ʾ\n");
	printf("4.����������㷨�������������\n");
	printf("5.���û���½����\n");
	printf("6.�ļ�����ϵͳ\n");
	printf("7.���̹���\n");
	
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
		printf("�Ƿ�ʹ���������ݣ�0�ǣ�1��");
		scanf("%d",&wheInit);
		if(wheInit)
			sisuoInit();//���Բ�ʹ�ã�ѡ�����õ����ݽ��в���
		printf("-------------------------------------------------------------\n");
		showInfo();
		printf("\nϵͳ��ȫ�������\n");
		printf("PID\t Work\t\tNeed\tAllocation\tWork+Allocation\n");
		isSafe();
		while(true){
			printf("\n--------------------------------------------------------\n");
			printf("\n����Ҫ����Ľ��̣�");
			scanf("%d",&curProcess);
			printf("\n����Ҫ���������P%d����Դ��",curProcess);
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
					else{printf("��Դ���㣬�ȴ��У�\n");break;}	
				}
			if(j==resourceNum){
				for(j=0;j<resourceNum;j++){
					available[j]-=request[j];
					allocation[curProcess][j]+=request[j];
					need[curProcess][j]-=request[j];
				}
				printf("\nϵͳ��ȫ�������\n");
				printf(" PID\t Work\t\tNeed\tAllocation\tWork+Allocation\n");
				if(isSafe()){
					printf("����ɹ���\n");
					showInfo();
				}
				else{
					for(j=0;j<resourceNum;j++){
						available[j]+=request[j];
						allocation[curProcess][j]-=request[j];
						need[curProcess][j]+=request[j];
					}
					printf("����ʧ�ܣ�\n");
					showInfo();
				}
			}
			
		}	
		printf("�Ƿ���Ҫ�ص����˵���0����1����\n");
		scanf("%d",&openkey3);
		if(openkey3==1)
			break;
	}
	}
	
	else if(openkey==6){
		wenjianshowInfo();
    	FileTree rootDir = init(); //��ʼ���ļ�ϵͳ��
    	FileNode *curDir = rootDir; //�л���ǰ�ڵ�Ϊ���ڵ�
    	char curPath[PATHMAXN] = "ROOT"; //�л���ǰ·��Ϊ��·��
    	printf("%s> ", curPath); //��� ROOT>
    	char operation[OPMAXN];
    	while(gets(operation)!= EOF){ //��ȡ����
        	char *op = strtok(operation, " ");
        	char *arg = strtok(NULL, " "); //���Ϸָ����������Ͳ���������cd music��������Ϊcd������Ϊmusic
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
            }else if(strcmp(arg, "\\") == 0){ //cd \ ---�л�Ϊ��Ŀ¼
                curDir = rootDir;
            }else{ //cd music --������Ŀ¼
                FileNode *pNode = curDir;
                curDir = enterSubDir(curDir, arg);
                if(!curDir){ //����δ֪Ŀ¼��������
                    printf("ERROR:\"%s\"Ŀ¼��û��\"%s\", ����������!\n",pNode->name, arg);
                    curDir = pNode;
                    printf("%s> ", curPath);
                    continue;
                }
            }
            strcpy(curPath, curDir->path);
        }else if(strcmp(op, "exit") == 0){ //exit �˳�
            printf("���˳�ϵͳ, ллʹ��!\n", op);
            break;
        }else{ // ����δ֪�������
            printf("ERROR:��֧��\"%s\"����, ����������!\n", op);
        }
        printf("%s> ", curPath);
        
        //printf("�Ƿ���Ҫ�ص����˵���0����1����\n");
		//scanf("%d",&openkey6);
		//if(openkey6==1)
			//break;
    }
		
	}
	
	}
	
}