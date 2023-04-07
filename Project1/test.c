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
    pa = A->next;  // pa 指向 A 的首元结点
    pb = B->next;
    qa = pa, qb = pb;
    C = A;        // 因为 C 中第一个元素是 A 中的元素，所以只需要 C 指向 A就行了
    while (pa && pb)
    {
        qa = pa;
        qb = pb;
        pa = pa->next;
        pb = pb->next;
        qb->next = qa->next;
        qa->next = qb;
    }
    if (!pa)  // 如果 A 链表的长度小于 B 链表的长度
        qb->next = pb; // 将 B 的后续节点连接到新链表的尾端
    pb = B;  // 准备删除 B 链表
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


