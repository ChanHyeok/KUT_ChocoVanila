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

void Enqueue(Queue *queue, int data, int index)
{
	node *now = (node *)malloc(sizeof(node)); //��� ����
	//������ ����
	now->timeTag = data;
	now->indexOfSegmentArray = index;
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
}

node Dequeue(Queue *queue)
{
	int data = 0;
	node nowAddress;
	node *now;
	now = &nowAddress;
	if (IsEmpty(queue))//ť�� ����� ��
	{
		return *now;
	}
	now = queue->front;//�� ���� ��带 now�� ���
	data = now->timeTag;//��ȯ�� ���� now�� data�� ����
	queue->front = now->next;//�� ���� now�� ���� ���� ����
	queue->count--;//���� ������ 1 ����
	return *now;
}