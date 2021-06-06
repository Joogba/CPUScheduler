#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>



//HRRN

#define MAX_QUEUE 2

#define HRR(wt, st) ((float)wt+(float)st)/(float)st  // 응답률 매크로함수
#define ABS(X) ((X) < 0 ? -(X) : (X))					// 절댓값
#define DIV(sum, cnt) (float)sum/(float)cnt		// 나눗셈

#define EPSILON 0.000001  // float 비교 오차

enum Priority_Set {
	Realtime = 0,
	Q1 = 1
};

// Structs =======================================================

typedef struct priority_queue* queue_pointer;
struct priority_queue {
	queue_pointer left_link;
	queue_pointer right_link;
	int pid;				// Process id
	int priority;			// 우선순위 입력은 받지만 사용 x
	int computing_time;		// 연산시간
	int service_time;		// 원래는 서비스타임(또는 Burst Time)을 예상해서 정해줘야 하지만 일단 computing_time으로 세팅
	int input_time;			// 큐에 들어간 시간


	float response_ratio;	//응답률 
	int waiting_time;		//대기시간
};

typedef struct queue_head* head_pointer;
struct queue_head {
	queue_pointer left_link;
	queue_pointer right_link;
};

// Variables =======================================================

head_pointer queueArr[MAX_QUEUE];	// 헤드 노드 realtime, q1
int time = 0;						// 흐른 시간
int sum_turn_around_time = 0;		// 평균 반환시간을 구하기 위한 모든 반환시간의 합
float sum_normal_turn_around_time = 0;
int scheduling_count = 0;			// 스케쥴링된 프로세스의 갯수




// Methods =======================================================

//  기본 큐 관련 메서드 ==-=-=-=-=-=-=

void initialize_queue(void);								// 모든 헤드큐 초기화
int insert_queue(int pid, int priority, int computing_time);// 큐 삽입 명령 처리
void delete_node(queue_pointer p_node);						// 큐에서 노드 삭제
void print_queue(void);										// 모든 노드 출력
void insert_after(queue_pointer new, queue_pointer des);	// des 노드의 뒤에 new 추가
void free_all_node(void);									// 모든 노드 메모리 해제
void print_node(queue_pointer p_node);						// 하나의 노드정보 출력
void print_node_tat(queue_pointer p_node, int return_time); // 턴어라운드 타임을 포함한 노드정보 출력
int get_queue_id(queue_pointer p_node);						// 해당 노드가 속한 큐 반환

queue_pointer find_node_for_insert(int priority, float hrr);// 우선순위로 어느 큐에 넣을지 정함(realtime process고려)
															// rtp 가아닌것은 hrr을 기준으로 삽입위치 정함

//  hrr 관련 메서드 ==-=-=-=-=-=-=

void add_waiting_time(int wt);								// 대기시간 증가
void update_node(queue_pointer p_node);						// 노드의 hrr 필드 업데이트
void schedule_process();									// 프로세스 스케쥴링
void schedule_process_while();								// 처리할 프로세스가 없을때까지 스케쥴링



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

	printf("Process ID \tQueue ID \tcomputing_time \tResponce_Ratio \tWaiting_time \tturn_around_time\n\n");

	int fmode, fpid, fpriority, fctime;
	while (!feof(datafile))
	{
		fmode = fpid = fpriority = fctime = 0;

		if (!fscanf_s(datafile, "%d %d %d %d\n", &fmode, &fpid, &fpriority, &fctime))
			printf("파일 읽기 실패\n");

		
		switch (fmode)
		{
		case 0: // 프로세스 추가
			insert_queue(fpid, fpriority, fctime);	
			
			break;
		case 1: // 스케쥴링
			schedule_process();
			
			break;
		case -1:
			
			break;
		default:
			printf("해당하는 명령이 없습니다 입력 : %d\n", fmode);
			break;
		}
		
	}	
	schedule_process_while(); // 큐에있는 나머지 프로세스 스케쥴링
	
	printf("\n\n종료\n");

	float avg_tat = DIV(sum_turn_around_time, scheduling_count);
	float avg_nm_tat = DIV(sum_normal_turn_around_time , scheduling_count);

	printf("%d %d %d", sum_turn_around_time, time, scheduling_count);
	printf("HRRN scheduling 평균 반환 시간 : %.3f \n", avg_tat);
	printf("HRRN scheduling 정규화된 평균 반환 시간 : %.3f \n", avg_nm_tat);

	fclose(datafile);
	free_all_node();
	return 0;
}

void initialize_queue(void) // 완료
{
	memset(queueArr, NULL, sizeof(queue_pointer) * 5);
	head_pointer head;
	for (int i = 0; i < MAX_QUEUE; i++) // 큐의 헤드 초기화;
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
	temp->input_time = time;

	if(priority > 30)
		temp->priority = 31;
	else
		temp->priority = priority;

	temp->waiting_time = 0;
	temp->service_time = computing_time;
	temp->response_ratio = 
		
		(temp->waiting_time, temp->service_time);

	add_waiting_time(1);
	
	insert_after(temp, find_node_for_insert(temp->priority, temp->response_ratio));

	return 0;
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
	
}

queue_pointer find_node_for_insert(int priority, float hrr) // 완료
{
	// 몇번째 큐에 들어갈지 정해준다
	int idx = 0;
	if (priority < 0)
		idx = Realtime;
	else
		idx = Q1;

	head_pointer head = queueArr[idx];
	queue_pointer temp = head->right_link;
	if (temp == NULL)
	{
		return (queue_pointer)head;
	}

	while (temp != NULL)
	{
		if (temp->response_ratio < hrr)
			return temp->right_link;
		else if (temp->response_ratio >= hrr && temp->right_link == NULL)
			return temp;
		else if (temp->response_ratio >= hrr && temp->right_link->response_ratio <= hrr)
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

	default:
		temp = "default";

	}

	printf("%d\t\t%-10s\t\t%d\t\t%.4f\t\t%d\t\t\n", p_node->pid, temp , p_node->computing_time, p_node->response_ratio, p_node->waiting_time);
}

void print_node_tat(queue_pointer p_node, int return_time)
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

	default:
		temp = "in nowhere";
		break;
	}

	printf("%d\t\t%-10s\t\t%d\t\t%.4f\t\t%d\t\t%d\t\t\n", p_node->pid, temp, p_node->computing_time, p_node->response_ratio, p_node->waiting_time, return_time);
}

void print_queue(void) // 완료
{
	printf("Process ID \tQueue ID \tcomputing_time \tResponce_Ratio \tWaiting_time \tturn_around_time\n\n");
	for (int i = 0; i < MAX_QUEUE; i++)
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

queue_pointer get_max_hrr(int idx)
{
	float max_hrr = 0;
	queue_pointer temp = queueArr[idx]->right_link;
	queue_pointer max_hrr_node = NULL;

	while (temp != NULL)
	{
		if (max_hrr < temp->response_ratio) // 크면
		{
			max_hrr = temp->response_ratio;
			max_hrr_node = temp;
		}
		else if (ABS(max_hrr - temp->response_ratio) < EPSILON) // 같으면
		{
			if (max_hrr_node->waiting_time <= temp->waiting_time) // 대기시간으로 비교
			{
				max_hrr = temp->response_ratio;
				max_hrr_node = temp;
			}

		}
		temp = temp->right_link;
	}

	

	return max_hrr_node;
}

void node_to_top(queue_pointer p_node, int idx) 
{
	// 큐에서 뺀다
	if (p_node == NULL)
		return;

	if (idx > 4 || idx < 0)
		return;

	delete_node(p_node);

	// 큐의 맨앞으로
	insert_after(p_node,queueArr[idx] );
	
}

void schedule_process()
{
	
	int turn_arround_time = 0;
	int idx = 0;
	queue_pointer temp = NULL;

	if (queueArr[Realtime]->right_link != NULL)
	{
		//실시간 프로세스가 있으면
		idx = Realtime;
		temp = queueArr[idx]->right_link;
	}
	else if(queueArr[Q1]->right_link != NULL)
	{
		//실시간 프로세스가 없으면 일반큐
		idx = Q1;
		node_to_top(get_max_hrr(Q1), Q1);
		temp = queueArr[idx]->right_link;
	}

	if (temp == NULL)
		return;

	time += temp->computing_time;
	turn_arround_time = time - temp->input_time;
	sum_turn_around_time += turn_arround_time;
	sum_normal_turn_around_time += (float)turn_arround_time / (float)temp->computing_time;
	scheduling_count++;
	
	
	print_node_tat(temp,turn_arround_time);
	

	delete_node(temp);
	add_waiting_time(temp->computing_time);

	free(temp);

	return;
}

void delete_node(queue_pointer p_node)
{
	if (p_node->right_link != NULL)	//마지막노드 아닐때
	{
		p_node->left_link->right_link = p_node->right_link;
		p_node->right_link->left_link = p_node->left_link;
		
	}
	else // 마지막 노드일때
	{
		p_node->left_link->right_link = NULL;
	}

	p_node->left_link = NULL;
	p_node->right_link = NULL;

	return ;
}

void schedule_process_while()
{
	bool is_remain = false;
	while (1)
	{
		is_remain = false;
		for (int i = 0; i < MAX_QUEUE; i++)
		{
			if (queueArr[i]->right_link != NULL)
				is_remain = true;
		}

		if (is_remain == false)
			break;

		schedule_process();
		
	}
}