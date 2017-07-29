// check point
#include "stdafx.h"
#include "MFC_Synthetic.h"
#include "MFC_SyntheticDlg.h"
#include "afxdialogex.h"
#include <io.h>

/************************
ť
************************/


void InitQueue(Queue *queue)
{
	queue->front = queue->rear = NULL; //front�� rear�� NULL�� ����
	queue->count = 0;//���� ������ 0���� ����
}

int IsEmpty(Queue *queue)
{
	return queue->count == 0;    //���� ������ 0�̸� �� ����
}

void Enqueue(Queue *queue, segment data)
{
	//��� ����
	node *now = new node;
	
	//������ ����
	now->segment_data = data;

	//��� ���� ������ �ʱ�ȭ 
	now->next = NULL;

	if (IsEmpty(queue))//ť�� ������� ��
	{
		queue->front = now;//�� ���� now�� ����       
	}
	else//������� ���� ��
	{
		queue->rear->next = now;//�� ���� ������ now�� ����
	}
	queue->rear = now;//�� �ڸ� now�� ����   
	queue->count++;//���� ������ 1 ����

	// �ӽ÷� ������ node �޸� ����
}

node Dequeue(Queue *queue)
{
	segment data;
	node nowAddress, *now;
	now = &nowAddress;
	if (IsEmpty(queue))//ť�� ����� ��
	{
		return *now;
	}
	now = queue->front;//�� ���� ��带 now�� ���
	data = now->segment_data;//��ȯ�� ���� now�� data�� ����
	queue->front = now->next;//�� ���� now�� ���� ���� ����
	queue->count--;//���� ������ 1 ����
	return *now;
}


/************************
prevHumanDetectedVector�� ���
component vector�� ���� ���� ũ�� 5�� ����ť�� ����� ����
************************/

void InitComponentVectorQueue(ComponentVectorQueue *componentVectorQueue) {
	printf("Init Queue\n");
	componentVectorQueue->front = 0;
	componentVectorQueue->rear = 0;
}// �� �ڸ� ��Ÿ���� ������ �ΰ��� ���� �ڸ��� ������ ����ִ� ť ����

bool IsComponentVectorQueueEmpty(ComponentVectorQueue *componentVectorQueue) {
	// �� �� �����Ͱ� ������ true
	if (componentVectorQueue->front == componentVectorQueue->rear)
		return true;
	else
		return false;
}

bool IsComponentVectorQueueFull(ComponentVectorQueue *componentVectorQueue) {
	// ť�� ��/�� �������� ���̰� 1�� �� ���
	if (NEXT(componentVectorQueue->rear) == componentVectorQueue->front)
		return true;
	else
		return false;
}

// ť �߰� ����
void PutComponentVectorQueue(ComponentVectorQueue *componentVectorQueue, vector<component> componentVector) {
	// ť�� ���� �� ���� ��� �Լ� Ż��
	if (IsComponentVectorQueueFull(componentVectorQueue))
		return;

	// ������ �߰�
	componentVectorQueue->buf[componentVectorQueue->rear] = componentVector;

	// ��ⷯ ����(�������� ���� ���� ������ ��� ���� ������ ��ȯ
	componentVectorQueue->rear = NEXT(componentVectorQueue->rear);
}

// ť ���� ���� (������ �����ʹ� ����)
void RemoveComponentVectorQueue(ComponentVectorQueue *componentVectorQueue) {
	// ť�� ����� ��� ���������� �ǹ̰� ���⋚���� �Լ� Ż��
	if (IsComponentVectorQueueEmpty(componentVectorQueue))
		return;

	// ť�� ���� ��ġ ���� �� ��ⷯ ����
	componentVectorQueue->front = NEXT(componentVectorQueue->front);
}

// point��° component �����͸� ��ȯ 
vector<component> GetComponentVectorQueue(ComponentVectorQueue *componentVectorQueue, int point) {
	return componentVectorQueue->buf[point];
}
