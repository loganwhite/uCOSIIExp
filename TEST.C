/*
*********************************************************************************************************
*                                                uC/OS-II
*                                          The Real-Time Kernel
*
*                           (c) Copyright 1992-2002, Logan Von
*                                           All Rights Reserved
*
*                                               EXAMPLE #1
*********************************************************************************************************
*/

#include "includes.h"

/*
*********************************************************************************************************
*                                               CONSTANTS
*********************************************************************************************************
*/

#define  TASK_STK_SIZE                 512       /* Size of each task's stacks (# of WORDs)            */
#define  N_TASKS                        10       /* Number of identical tasks                          */

/*
*********************************************************************************************************
*                                               VARIABLES
*********************************************************************************************************
*/

OS_STK        TaskStk[N_TASKS][TASK_STK_SIZE];        /* Tasks stacks                                  */
OS_STK		  ReceiveTaskStk[TASK_STK_SIZE];
OS_STK        TaskStartStk[TASK_STK_SIZE];
char          TaskData[N_TASKS];                      /* Parameters to pass to each task               */
void 		  *MSGQGrp[N_TASKS];					//the message queue array with the number of tasks' elements
OS_EVENT     *RandomSem;
OS_EVENT 	 *MSGQ;

/*
*********************************************************************************************************
*                                           FUNCTION PROTOTYPES
*********************************************************************************************************
*/

        void  Task(void *data);                       /* Function prototypes of tasks                  */
		void  ReceiveTask(void *data);				  /* Function prototypes of receive task		   */
        void  TaskStart(void *data);                  /* Function prototypes of Startup task           */
		void  FileContentInit(void);
		char  Classify(unsigned int value);
		void  PntIntNum(INT16U num, INT8U x, INT8U y);
		void  PntFltNum(float num, INT8U x, INT8U y);
		void  GetDate(char* day);
		void  TaskSuspend(void);
		void  TaskResume(void);
		void  RemoveGraph(void);
		void  DrawGraph(INT16U data, INT8U pos);
static  void  TaskStartCreateTasks(void);
static  void  TaskStartDispInit(void);
static  void  TaskStartDisp(void);


int sum(int* arr, int size);
/*$PAGE*/
/*
*********************************************************************************************************
*                                                MAIN
*********************************************************************************************************
*/

void  main (void) {
    PC_DispClrScr(DISP_FGND_WHITE + DISP_BGND_BLACK);      /* Clear the screen                         */

    OSInit();                                              /* Initialize uC/OS-II                      */

    PC_DOSSaveReturn();                                    /* Save environment to return to DOS        */
    PC_VectSet(uCOS, OSCtxSw);                             /* Install uC/OS-II's context switch vector */

    RandomSem   = OSSemCreate(1);                          /* Random number semaphore                  */
	MSGQ		= OSQCreate(MSGQGrp, N_TASKS);
	OSQFlush(MSGQ);
	
    OSTaskCreate(TaskStart, (void *)0, &TaskStartStk[TASK_STK_SIZE - 1], 0);
    OSStart();                                             /* Start multitasking                       */
}


/*
*********************************************************************************************************
*                                              STARTUP TASK
*********************************************************************************************************
*/
void  TaskStart (void *pdata) {
#if OS_CRITICAL_METHOD == 3                                /* Allocate storage for CPU status register */
    OS_CPU_SR  cpu_sr;
#endif
    char       s[100];
    INT16S     key;

    pdata = pdata;                                         /* Prevent compiler warning                 */

    TaskStartDispInit();                                   /* Initialize the display                   */

    OS_ENTER_CRITICAL();
    PC_VectSet(0x08, OSTickISR);                           /* Install uC/OS-II's clock tick ISR        */
    PC_SetTickRate(OS_TICKS_PER_SEC);                      /* Reprogram tick rate                      */
    OS_EXIT_CRITICAL();

    OSStatInit();                                          /* Initialize uC/OS-II's statistics         */
	FileContentInit();									   /* Initialize fileContent				   */
    TaskStartCreateTasks();                                /* Create all the application tasks         */
	
	OSTaskCreate(ReceiveTask,(void *)0, &ReceiveTaskStk[TASK_STK_SIZE - 1], 11);

    for (;;) {
        TaskStartDisp();                                  /* Update the display                       */


        if (PC_GetKey(&key) == TRUE) {                     /* See if key has been pressed              */
            if (key == 'q')                                /* Yes, see if it's the ESCAPE key          */
                PC_DOSReturn();                            /* Return to DOS                            */
			if (key == 'b')								   /* See if it's b and then stop the tasks	   */
				TaskSuspend();
			if (key == 'r')								   /* See if it's r and then resume the tasks	   */
				TaskResume();
        }

        OSCtxSwCtr = 0;                                    /* Clear context switch counter             */
        OSTimeDlyHMSM(0, 0, 0, 100);                         /* Wait one second                          */
    }
}

/*$PAGE*/
/*
*********************************************************************************************************
*                                        INITIALIZE THE DISPLAY
*********************************************************************************************************
*/

static  void  TaskStartDispInit (void) {
/*                                1111111111222222222233333333334444444444555555555566666666667777777777 */
/*                      01234567890123456789012345678901234567890123456789012345678901234567890123456789 */
    PC_DispStr( 0,  0, "                        Feng Wendi NO.2012011686 Class: CS1206                  ", DISP_FGND_WHITE + DISP_BGND_RED + DISP_BLINK);
    PC_DispStr( 0,  1, "Source:(1)   (2)   (3)   (4)   (5)   (6)   (7)   (8)   (9)   (10)   (AVG)       ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0,  2, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0,  3, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0,  4, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
	PC_DispStr( 0,  5, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);    
    PC_DispStr( 0,  6, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0,  7, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0,  8, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0,  9, "--------------------------------------------------------------------------------", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0, 10, "Recei:                                                                          ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0, 11, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0, 12, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0, 13, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0, 14, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0, 15, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0, 16, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0, 17, "Times:                                                            |             ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0, 18, "Max  :                                                            |             ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0, 19, "Min  :                                                            |             ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0, 20, "Avg  :                                                            |             ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0, 21, "                                                                  --------------", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0, 22, "#Tasks          :        CPU Usage:     %                                       ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0, 23, "#Task switch/sec:                                                               ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr( 0, 24, "              <-PRESS 'q' TO QUIT 'b' TO Suspend 'r' TO Resume->                ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY + DISP_BLINK);
/*                                1111111111222222222233333333334444444444555555555566666666667777777777 */
/*                      01234567890123456789012345678901234567890123456789012345678901234567890123456789 */
}

/*$PAGE*/
/*
*********************************************************************************************************
*                                        Title in the file
*********************************************************************************************************
*/
void FileContentInit(void) {
	FILE* fp_source,*fp_receive;
	char filename[25];
	GetDate(filename);
	strcat(filename,"Sd.dat");
	fp_source = fopen(filename,"w");
	fprintf(fp_source,"%11s%5s%5s%5s%5s%5s%5s%5s%5s%5s%6s\n%6s","Source:(1)","(2)","(3)","(4)","(5)","(6)","(7)","(8)","(9)","(10)","(AVG)"," ");
	fclose(fp_source);
	GetDate(filename);
	strcat(filename,"Rv.dat");
	fp_receive = fopen(filename,"w");
	fprintf(fp_receive, "%11s%5s%5s%5s%5s%5s%5s%5s%5s%5s%6s\n%6s","Receive:(1)","(2)","(3)","(4)","(5)","(6)","(7)","(8)","(9)","(10)","(AVG)"," ");
	fclose(fp_receive);
}
/*$PAGE*/
/*
*********************************************************************************************************
*                                           UPDATE THE DISPLAY
*********************************************************************************************************
*/

static  void  TaskStartDisp (void) {
    char   s[80];


    sprintf(s, "%5d", OSTaskCtr);                                  /* Display #tasks running               */
    PC_DispStr(18, 22, s, DISP_FGND_YELLOW + DISP_BGND_BLUE);

#if OS_TASK_STAT_EN > 0
    sprintf(s, "%3d", OSCPUUsage);                                 /* Display CPU usage in %               */
    PC_DispStr(36, 22, s, DISP_FGND_YELLOW + DISP_BGND_BLUE);
#endif

    sprintf(s, "%5d", OSCtxSwCtr);                                 /* Display #context switches per second */
    PC_DispStr(18, 23, s, DISP_FGND_YELLOW + DISP_BGND_BLUE);

    sprintf(s, "V%1d.%02d", OSVersion() / 100, OSVersion() % 100); /* Display uC/OS-II's version number    */
    PC_DispStr(75, 24, s, DISP_FGND_YELLOW + DISP_BGND_BLUE);

    switch (_8087) {                                               /* Display whether FPU present          */
        case 0:
             PC_DispStr(71, 22, " NO  FPU ", DISP_FGND_YELLOW + DISP_BGND_BLUE);
             break;

        case 1:
             PC_DispStr(71, 22, " 8087 FPU", DISP_FGND_YELLOW + DISP_BGND_BLUE);
             break;

        case 2:
             PC_DispStr(71, 22, "80287 FPU", DISP_FGND_YELLOW + DISP_BGND_BLUE);
             break;

        case 3:
             PC_DispStr(71, 22, "80387 FPU", DISP_FGND_YELLOW + DISP_BGND_BLUE);
             break;
    }
}

/*$PAGE*/
/*
*********************************************************************************************************
*                                             CREATE TASKS
*********************************************************************************************************
*/

static  void  TaskStartCreateTasks (void) {
    INT8U  i;


    for (i = 0; i < N_TASKS; i++) {                        /* Create N_TASKS identical tasks           */
        TaskData[i] =  '0'+i;                         /* Each task will display its own letter    */
        OSTaskCreate(Task, (void *)&TaskData[i], &TaskStk[i][TASK_STK_SIZE - 1], i + 1);
    }
}
/*$PAGE*/
/*
*********************************************************************************************************
*                                                  TASKS
*********************************************************************************************************
*/

void  Task (void *pdata) {
    unsigned int  rand_num;
    INT8U  err;
	INT8U  pos_x, pos_y = 0;
	FILE* fp;						//used for storing information to data.dat

	char str_send[10];
	char str_sum[10];
	char filename[25];
	static int value[10];									//values send used for calculate the average


    for (;;) {
		GetDate(filename);
		strcat(filename, "Sd.dat");
		fp = fopen(filename,"a");
        OSSemPend(RandomSem, 0, &err);
        rand_num = random(500);
        OSSemPost(RandomSem);
        sprintf(str_send,"%5d",rand_num);
		OSQPost(MSGQ,str_send);
		pos_x = *(INT8U*)pdata - '0';		
		fprintf(fp,"%s",str_send);
        PC_DispStr(pos_x*6+5, pos_y + 2, str_send, DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
        value[pos_x] = (int)rand_num;
		
		//if this group of value are gotten, sum and average it
		if (pos_x == 9) {
			memset(str_sum,0,sizeof(str_sum));
			sprintf(str_sum,"%6.2f",(float)sum(value,10)/10.0);
			//show sum col
			PC_DispStr(68, 2 + pos_y, str_sum, DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
			fprintf(fp,"%8s\n%6s",str_sum," ");
			memset(value,0,sizeof(value));
		}
		if (pos_y == 7) pos_y = 0;
		OSTimeDlyHMSM(0,0,1,0);
		pos_y ++;		///start next row
		fclose(fp);
    }
	
}
/*$PAGE*/
/*
*********************************************************************************************************
*                                                  RECEIVETASK
*********************************************************************************************************
*/
void ReceiveTask(void *pdata) {
	INT8U err;
	INT8U pos_x = 0, pos_y = 0;	
	INT8U font_color;
	char *str_receiv;
	INT16U recev;
	float result;
	char avg[10];
	char attention[2];
	INT16U i = 0; 										///used for row control
	char str_print[10];
	
	INT16U number = 0;									//used for calculating averages in receiving data.
	
	FILE* fp;	
	char filename[25];
	
	INT16U data_iter = 0;										//data_iter/10 is the row and data_iter%10 is the col
	static INT16U times[10];									//used for count the times received
	static INT16U data[10];									//used for record all the data.
	static INT16U max[10];
	static INT16U min[10];
	static INT16U sumr[10];	
	
    pdata = pdata;
	memset(min,1000,sizeof(min));							//initialize the min array
    for (;;) {
		GetDate(filename);
		strcat(filename,"Rv.dat");
		fp = fopen(filename, "a");
		str_receiv = OSQPend(MSGQ,20,&err);				//request message
		pos_x = i;
		i ++;
		i %= 10;
		if (pos_y/10 == 7) {
			pos_y = 0;
			
			PC_DispClrScr(DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
			TaskStartDispInit();
		}
        PC_DispStr(5+6*pos_x, 10 + pos_y/10, str_receiv, DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
		fprintf(fp, "%s", str_receiv);
		recev = atoi(str_receiv);
		
		
		if (data_iter == 70) {
			data_iter = 0;
		}
		data[data_iter%10] = recev;

		PntIntNum((++ times[data_iter%10]),5+6*pos_x, 17);
		
		if (times[9] == 100) {
			TaskSuspend();
		}
		
		max[data_iter%10] = (max[data_iter%10] < recev) ? recev : max[data_iter%10];
		PntIntNum(max[data_iter%10],5+6*pos_x, 18);
		
		min[data_iter%10] = (min[data_iter%10] > recev) ? recev : min[data_iter%10];
		PntIntNum(min[data_iter%10],5+6*pos_x, 19);
		
		sumr[data_iter%10] += recev;
		PntFltNum((float)sumr[data_iter%10]/(float)times[data_iter%10],5+6*pos_x,20);
		DrawGraph(sumr[data_iter%10]/times[data_iter%10], pos_x);
		
		data_iter ++;
			
		number += recev;
		result = (float)number/10.0;
		sprintf(avg,"%6.2f",result);
		
		attention[0] = Classify((unsigned int)result);
		attention[1] = 0;								//the string ending
		strcat(avg,attention);
		font_color = 0x00 + ((*attention) - 'A');
		
		PC_DispStr(68, 10 + pos_y/10, avg, font_color + DISP_BGND_LIGHT_GRAY);
		if (pos_y%10 == 9) {
			fprintf(fp, "%8s\n%6s",avg," "); 
			memset(avg,0,sizeof(avg));
			number = 0;
			RemoveGraph();
		}
		
        OSTimeDlyHMSM(0, 0, 0, 100);
		pos_y ++;
		fclose(fp);
    }
	
}
/*$PAGE*/
/*
*********************************************************************************************************
*                                                  ARRAYSUM
*********************************************************************************************************
*/
int sum(int* arr, int size) {
	int i, result = 0;
	for (i = 0; i < size; i++)
		result += arr[i];
	return result;
}
/*$PAGE*/
/*
*********************************************************************************************************
*                                                  CLASSIFYAQI
*********************************************************************************************************
*/
char Classify(unsigned int value) {
	if (value <=50) return 'A';
	else if (value > 50 && value <= 100) return 'B';
	else if (value > 100 && value <= 150) return 'C';
	else if (value > 150 && value <= 200) return 'D';
	else if (value > 200 && value <= 300) return 'E';
	else return 'F';
}
/*$PAGE*/
/*
*********************************************************************************************************
*                                                  PRINTINTNUMBER
*********************************************************************************************************
*/
void PntIntNum(INT16U number, INT8U x, INT8U y) {
	char str_p[10];
	sprintf(str_p,"%5d", number);
	PC_DispStr(x, y, str_p, DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
}
/*$PAGE*/
/*
*********************************************************************************************************
*                                                  PRINTFLOATNUMBER
*********************************************************************************************************
*/
void  PntFltNum(float num, INT8U x, INT8U y) {
	char str_p[10];
	sprintf(str_p,"%6.1f", num);
	PC_DispStr(x, y, str_p, DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
}
/*$PAGE*/
/*
*********************************************************************************************************
*                                                  GETDATEINFO
*********************************************************************************************************
*/
void GetDate(char* day) {
	time_t t;
	struct tm* btime;
	t = time(NULL);
	btime = localtime(&t);
	sprintf(day,"%d-%d",btime->tm_mon+1,btime->tm_mday);
}
/*$PAGE*/
/*
*********************************************************************************************************
*                                                  SUSPENDTASKS
*********************************************************************************************************
*/
void  TaskSuspend(void) {
	INT8U i;
	for ( i = 1; i <= N_TASKS+1; i++)
		OSTaskSuspend(i);
}
/*$PAGE*/
/*
*********************************************************************************************************
*                                                  RESUMETASKS
*********************************************************************************************************
*/
void  TaskResume(void) {
	INT8U i;
	for ( i = 1; i <= N_TASKS+1; i++)
		OSTaskResume(i);
}
/*$PAGE*/
/*
*********************************************************************************************************
*                                                  CLEARGRAPHICAREA
*********************************************************************************************************
*/
void  RemoveGraph(void) {
	INT8U i;
	char* white = "             ";
	for (i = 0; i <  4; i++)
		PC_DispStr(67, 17+i, white, DISP_BGND_LIGHT_GRAY);
}
/*$PAGE*/
/*
*********************************************************************************************************
*                                                  PRINTGRAPH
*********************************************************************************************************
*/
void  DrawGraph(INT16U data, INT8U pos) {
	INT8U i;
	data -= 180;
	data /= 25;
	if (data > 4 ) data = 4;
	for (i = 0; i < data; i++)
		PC_DispChar(67+pos, 20-i, 178, DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
}