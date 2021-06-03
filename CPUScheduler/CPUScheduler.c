#include <stdio.h>
#include <stdlib.h>

//HRRN

#define MAX_QUEUE 5

#define HRR(wt, st) ((float)wt+(float)st)/(float)st  // 응답률 매크로함수

enum Priority_Set {
	Realtime = 0,
	Q1 = 1,
	Q2 = 2,
	Q3 = 3,
	Q4 = 4,
};

// Structs =======================================================

typedef struct priority_queue* queue_pointer;
struct priority_queue {

	queue_pointer left_link;
	queue_pointer right_link;
	int pid;
	int priority;			// 우선순위 입력은 받지만 사용 x
	int computing_time;		// 연산시간
	int service_time;		// 원래는 서비스타임을 예상해서 정해줘야 하지만 일단 computing_time으로 세팅

	float response_ratio;	//응답률
	int waiting_time;		//대기시간

};

typedef struct queue_head* head_pointer;
struct queue_head {
	queue_pointer left_link;
	queue_pointer right_link;
};


// Methods =======================================================
void initialize_queue(void);								// 모든 헤드큐 초기화
int insert_queue(int pid, int priority, int computing_time);			// 큐 삽입
int delete_queue(int priority);								// 큐 삭제
void print_queue(void);										// 모든 노드 출력

void insert_after(queue_pointer new, queue_pointer des);	// des의 뒤에 new 추가
queue_pointer find_node_for_insert(int priority);			// 우선순위로 어느 큐에 넣을지 정함 (이후에 응답률로 변경)
void free_all_node(void);									// 모든 노드 메모리 해제

void add_waiting_time(int wt);								// 대기시간 증가
void complete_process(queue_pointer node);					// 프로세스 작업 완료 후 출력시켜주기
void schedule_process();									// 프로세스들 스케쥴링
void print_node(queue_pointer p_node);						// 하나의 노드정보 출력
int get_queue_id(queue_pointer p_node);						// 해당 노드가 속한 큐 반환
void update_node(queue_pointer p_node);						// 노드의 hrr 업데이트
float hrr(int wt, int st);


// Variables =======================================================
head_pointer queueArr[5]; // 헤드 노드 realtime, q1 q2 q3 q4
int t = 20;

int main()
{
	initialize_queue();
	FILE* datafile;
	fopen_s(&datafile, "input_data.txt", "r");
	if (datafile == NULL)
	{
		printf("파일 열기 실패\n");
		return 0;
	}

	int fmode, fpid, fpriority, fctime;
	while (!feof(datafile))
	{
		fmode = fpid = fpriority = fctime = 0;

		if (!fscanf_s(datafile, "%d %d %d %d\n", &fmode, &fpid, &fpriority, &fctime))
			printf("파일 읽기 실패\n");

		
		switch (fmode)
		{
		case 0: // 스케쥴링
			insert_queue(fpid, fpriority, fctime);
			print_queue();
			break;
		case 1: // 프로세스 추가
			
			//print_queue();
			break;
		case -1:
			printf("종료\n");
			return 0;
			break;
		default:
			printf("해당하는 명령이 없습니다 입력 : %d\n", fmode);
			break;
		}
		add_waiting_time(1);
	}

	print_queue();
	fclose(datafile);
	free_all_node();
	return 0;
}

void initialize_queue(void) // 완료
{
	memset(queueArr, NULL, sizeof(queue_pointer) * 5);
	head_pointer head;
	for (int i = 0; i < 5; i++) // 큐의 헤드 초기화;
	{
		queueArr[i] = malloc(sizeof(struct queue_head));
		if (queueArr[i] == NULL)
			return;

		queueArr[i]->left_link = NULL;
		queueArr[i]->right_link = NULL;

	}
}
int insert_queue(int pid, int priority, int computing_time) // 완료
{
	head_pointer head = NULL; // 임시 헤드 노드

	queue_pointer temp = NULL;
	temp = malloc(sizeof(struct priority_queue));
	if (temp == NULL)
	{
		printf("노드 생성 실패\n");
		return 0;
	}

	temp->left_link = NULL;
	temp->right_link = NULL;
	temp->pid = pid;
	temp->computing_time = computing_time;

	if(priority > 30)
		temp->priority = 31;
	else
		temp->priority = priority;

	temp->waiting_time = 0;
	temp->service_time = computing_time;
	temp->response_ratio = HRR(temp->waiting_time, temp->service_time);

	// 3개의 큐중 어느 큐에 넣을지 정함
	insert_after(temp, find_node_for_insert(temp->priority));

	return 0;
}
int delete_queue(int priority) // 완료
{
	head_pointer head = queueArr[(priority - 1) / 10]; // 우선순위가 해당하는 큐의 헤드

	if (head->right_link == NULL)
	{
		printf("해당 큐에 노드가 없습니다.\n");
		return -1;
	}

	queue_pointer temp = head->right_link;

	if (temp->priority <= priority)
	{
		printf("노드 삭제 || 입력 노드 : %d ,실제 삭제 노드 : %d\n", priority, temp->priority);

		temp->left_link->right_link = temp->right_link;
		temp->right_link->left_link = temp->left_link;
		free(temp);

		return 0;
	}
	else
	{
		printf("해당 우선순위보다 높거나 같은 노드가 없습니다.\n");
		return -1;
	}

}

void insert_after(queue_pointer new, queue_pointer des) // 완료
{
	if (des->right_link == NULL)	// 마지막노드일때
	{
		des->right_link = new;
		new->left_link = des;
	}
	else							// 중간일때
	{
		des->right_link->left_link = new;
		new->right_link = des->right_link;
		new->left_link = des;
		des->right_link = new;
	}
	printf("노드 삽입 || 우선순위 : %d\n", new->priority);
}

queue_pointer find_node_for_insert(int priority) // 완료
{
	// 몇번째 큐에 들어갈지 정해준다
	int idx = 0;
	if (priority < 0)
		idx = Realtime;
	else if (priority > 0 && priority <= 30)
	{
		idx = ((priority - 1) / 10) + 1; // Q1, Q2, Q3
	}
	else if (priority > 30)
		idx = Q4;

	head_pointer head = queueArr[idx];
	queue_pointer temp = head->right_link;
	if (temp == NULL)
	{
		return (queue_pointer)head;
	}

	while (temp != NULL)
	{

		if (temp->priority > priority)
			return temp->left_link;
		else if (temp->priority <= priority && temp->right_link == NULL)
			return temp;
		else if (temp->priority <= priority && temp->right_link->priority >= priority)
			return temp;

		temp = temp->right_link;
	}
	return temp; //없으면 꼬리 마지막 반환
}
void free_all_node(void)
{
	printf("노드 메모리 해제\n");

	for (int i = 0; i < MAX_QUEUE; i++)
	{
		queue_pointer temp = NULL;
		if (queueArr[i]->right_link == NULL)
		{
			printf("%d 번큐 비어있음(free all node)\n", i + 1);
			continue;
		}
		queue_pointer head = queueArr[i]->right_link;

		while (head)
		{
			temp = head;
			head = head->right_link;
			free(temp);
		}
		queueArr[i] = NULL;
	}
}

void add_waiting_time(int wt)
{
	for (int i = 0; i < MAX_QUEUE; i++)
	{
		if (queueArr[i]->right_link == NULL)
			continue;

		queue_pointer temp = queueArr[i]->right_link;
		while (temp != NULL)
		{
			temp->waiting_time += wt;
			update_node(temp);
			temp = temp->right_link;
		}
	}
}

void complete_process(queue_pointer node)
{

}// 프로세스 작업 완료 후 출력시켜주기
void schedule_process()
{

}

void print_node(queue_pointer p_node)
{
	if (p_node == NULL)
		return;

	char* temp = NULL;
	int temp_qid = 0;

	temp_qid = get_queue_id(p_node);

	switch (temp_qid)
	{
	case Realtime:
		temp = "real_time";
		break;

	case Q1:
		temp = "Q1";
		break;

	case Q2:
		temp = "Q2";
		break;

	case Q3:
		temp = "Q3";
		break;

	case Q4:
		temp = "Q4";
		break;
	}

	printf("%d\t\t %s\t\t %d\t\t\ %d\t\t %.1f\t\t %d\t\t\n", p_node->pid, temp ,p_node->priority, p_node->computing_time, p_node->response_ratio, p_node->waiting_time);
}

void print_queue(void) // 완료
{
	printf("Process ID \tQueue ID \tpriority \tcomputing_time \tResponce_Ratio \tWaiting_time\n");
	for (int i = 0; i < 5; i++)
	{
		if (queueArr[i]->right_link == NULL)
			continue;

		queue_pointer temp = queueArr[i]->right_link;
		while (temp != NULL)
		{
			print_node(temp);
			temp = temp->right_link;
		}
	}
}

int get_queue_id(queue_pointer p_node)
{
	queue_pointer temp = p_node;
	
	// head로 이동
	while (temp->left_link != NULL)
	{
		temp = temp->left_link;
	}

	//queueArr에서 해당 head와 같으면 인덱스 반환
	for (int i = 0; i < MAX_QUEUE; i++)
	{
		if (temp == queueArr[i])
		{
			return i;
		}
	}
}

void update_node(queue_pointer p_node)
{
	if (p_node == NULL)
		return;
	p_node->response_ratio = HRR(p_node->waiting_time, p_node->service_time);
}

float hrr(int wt, int st)
{
	return ((float)wt + (float)st) / (float)st;
}