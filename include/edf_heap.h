#pragma once
#include <os.h>
void OS_EdfHeapInit(void);
void OS_EdfHeapInsert(OS_TCB*, OS_ERR*);
void OS_EdfHeapSiftDown(int);
void OS_EdfHeapSiftUp(int);
void OS_EdfHeapRemove(OS_TCB*);
OS_TCB* OS_EdfHeapPeek();
void OS_EdfPrintHeap();
void OS_EdfHeapSift();
