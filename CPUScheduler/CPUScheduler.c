#include <stdio.h>
#include <stdlib.h>

//HRRN

// Structs =======================================================

typedef struct priority_queue* queue_pointer;
struct priority_queue {

	queue_pointer left_link;
	queue_pointer right_link;
	int pid;
	int priority;
	int computing_time;
	int response_ratio; //응답률
	int waiting_time;	//대기시간

};

typedef struct queue_head* head_pointer;
struct queue_head {
	queue_pointer left_link;
	queue_pointer right_link;
};


// Methods =======================================================
void initialize_queue(void);
int insert_queue(int priority, int computing_time);
int delete_queue(int priority);
void print_queue(void);

void insert_after(queue_pointer new, queue_pointer des);
queue_pointer find_node_for_insert(int priority);
void free_all_node(void);

void add_waiting_time(int wt);


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

	int fmode, fpriority, fctime;
	while (!feof(datafile))
	{
		fmode = fpriority = fctime = 0;

		if (!fscanf_s(datafile, "%d %d %d\n", &fmode, &fpriority, &fctime))
			printf("파일 읽기 실패\n");

		switch (fmode)
		{
		case 0: // 스케쥴링
			insert_queue(fpriority, fctime);
			//print_queue();
			break;
		case 1: // 프로세스 추가
			delete_queue(fpriority);
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
int insert_queue(int priority, int computing_time) // 완료
{
	if (priority > 30)
	{
		printf("우선순위가 범위를 초과했습니다. 삽입실패\n");
		return -1;
	}
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
	temp->computing_time = computing_time;
	temp->priority = priority; // 삽입할 노드 생성



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
void print_queue(void) // 완료
{
	printf("Queue ID \tpriority \tcomputing_time\n");
	for (int i = 0; i < 3; i++)
	{
		if (queueArr[i]->right_link == NULL)
			continue;

		queue_pointer temp = queueArr[i]->right_link;
		while (temp != NULL)
		{
			printf("%d\t\t %d\t\t %d\t\t\n", i + 1, temp->priority, temp->computing_time);
			temp = temp->right_link;
		}
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
	head_pointer head = queueArr[(priority - 1) / 10];
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
	for (int i = 0; i < 3; i++)
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

}