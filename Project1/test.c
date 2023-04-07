#define  _CRT_SECURE_NO_WARNINGS 1

/**
* Created with CLion.
* User:Lenovo
* Date:2023-03-29
* Time:16:57
* Description:
*/

#include <stdio.h>
#include <stdlib.h>
#define ElemType int

int Min(int a, int b)
{
    return a < b ? a : b;
}

struct _lnklist {
    ElemType data;
    struct _lnklist* next;
};
typedef struct _lnklist Node;
typedef struct _lnklist* LinkList;

void insert(LinkList L, ElemType num)
{
    while (L->next != NULL)
    {
        L = L->next;
    }
    LinkList newNode = (LinkList)malloc(sizeof(Node));
    newNode->data = num;
    newNode->next = L->next;
    L->next = newNode;
}

void lnk_merge(LinkList A, LinkList B, LinkList C)
{
    LinkList pa, pb, qa, qb;
    pa = A->next;  // pa ָ�� A ����Ԫ���
    pb = B->next;
    qa = pa, qb = pb;
    C = A;        // ��Ϊ C �е�һ��Ԫ���� A �е�Ԫ�أ�����ֻ��Ҫ C ָ�� A������
    while (pa && pb)
    {
        qa = pa;
        qb = pb;
        pa = pa->next;
        pb = pb->next;
        qb->next = qa->next;
        qa->next = qb;
    }
    if (!pa)  // ��� A ����ĳ���С�� B ����ĳ���
        qb->next = pb; // �� B �ĺ����ڵ����ӵ��������β��
    pb = B;  // ׼��ɾ�� B ����
    free(pb);
}

int main()
{
    LinkList HeadA = (LinkList)malloc(sizeof(Node));
    LinkList HeadB = (LinkList)malloc(sizeof(Node));
    LinkList HeadC = (LinkList)malloc(sizeof(Node));
    HeadA->next = NULL;
    HeadB->next = NULL;
    HeadC->next = NULL;

    for (int i = 0; i < 10; i++)
    {
        int tmpNum = 0;
        scanf("%d", &tmpNum);
        insert(HeadA, tmpNum);
    }
    for (int i = 0; i < 10; i++)
    {
        int tmpNum = 0;
        scanf("%d", &tmpNum);
        insert(HeadB, tmpNum);
    }
    lnk_merge(HeadA, HeadB, HeadC);

    while (HeadC->next != NULL)
    {
        printf("%d ", HeadC->next->data);
        HeadC = HeadC->next;
    }

    return 0;
}


