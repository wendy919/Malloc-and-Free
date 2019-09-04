//10152130207_yangwendi_Lab3 Malloc and Free

#include <stdio.h>
#include <stdlib.h>
#include "mem.h"
#include <unistd.h>
#include <sys/mman.h>

int m_error;
int flag=0;

// Header
typedef struct __header
{
    int size;  //size of the allocated region分配区域的大小，contain additional pointers to speed up deallocation包含额外指针来加速释放
    int magic; //provide additional integrity checking and other information提供额外的完整性检查和其他信息
} header;

//Description of a node of the freelist
//记录空闲块的大小，指向下一块空闲块的位置；定义空闲列表中的节点
typedef struct __node
{
    int size;
    struct __node* next;
} node;

node*head;

    int mem_init(int size_of_region)
    {
        if(flag) //flag=0所以不执行此步骤，为了检测进程是否调用超过1次；
        {
            m_error=E_BAD_ARGS;
            return -1;
        }
        flag=1;
        int pagesize=getpagesize(); //以页面大小位单位请求内存
        int p_count=1; //页数
        while(p_count*pagesize<size_of_region) //管理的内存块应满足用户请求所需要的内存块的大小
            ++p_count;
        //初始化堆，并将空闲列表的第一个元素放入该空间中；
        //这个堆的建立在通过系统调用mmap()所获得的空闲空间中；
        /*构建一个堆的方法还有sbrk()：系统找到空间的物理页面，将它们映射到请求进程的地址空间，然后返回新堆结束的值；但是这里我们使用mmap()；*/
        head=(node*)mmap(NULL,p_count*pagesize,PROT_READ | PROT_WRITE,MAP_SHARED|MAP_ANONYMOUS, -1, 0); /*PROT_WRITE映射区域可被写入；MAP_SHARED对映射区域的写入数据会复制回文件内，而且允许其他映射该文件的进程共享；*/
        //若映射成功则返回映射区的内存起始地址，否则返回MAP_FAILED（-1）
        if(head==MAP_FAILED) //mmap调用失败
        {
            m_error=E_BAD_ARGS;
            return -1;
        }
        //头指针包含这个堆的起始地址
        node*p=head;
        p->size=p_count*pagesize-sizeof(node); // 堆中实际的可用空间大小，为减掉头指针后的剩余空间
        p->next=NULL;
        return 0;
    }

    void * mem_alloc(int size, int style)
     //style: choose which strategy for managing free space
    {
        int n=1;
        while(n*8<size) //保证8字节对齐的内存块
            ++n;
        n=n*8+8;//the space need to be allocated actually，include the header
        node*p=head,*before=head,*q=head,*tbe; //p用于指向所选的内存块
        header*h;

        //3种方式确定如何查看可用空间的列表
        //Best Fit
        if(style==M_BESTFIT)
        {
            while(p!=NULL&&p->size<n) 
            {
                before=p;
                p=p->next;
            }
            
            if(p==NULL)
            {   m_error=E_NO_SPACE; //mem_init分配的内存中没有足够的连续可用空间满足此请求
                return NULL;
            }
            
            q=p;
            //search through the free list
            while(q->next!=NULL)
            {
                tbe=q;
                q=q->next;
                /*find chunks of free memory that are as big or bigger than the requested size; then return the one that is the smallest in that group of candidates*/
                if(q->size>=n&&q->size<=p->size)
                {
                    p=q;
                    before=tbe;
                }
            }
        }

        // Worst Fit
        else if(style==M_WORSTFIT)
        {
            while(q->next!=NULL)
            {
                tbe=q;
                q=q->next;
                //find the largest chunk and reutrn the requested amount
                if(q->size>=p->size)
                {
                    p=q;
                    before=tbe;
                }
            }

            if(p->size<n)
            {
                m_error=E_NO_SPACE;
                return NULL;
            }
        }
        
        //First Fit
        else if(style==M_FIRSTFIT)
        {
            while(p!=NULL&&p->size<n)
            {
                before=p;
                p=p->next;
            }

            if(p==NULL)
            {
                m_error=E_NO_SPACE;
                return NULL;
            }
        }

        
        if(p==head) //该堆为空
        {
            if(head->size==n) //n为所需要的内存字节数，用完所有剩余的空闲内存块
            {
                head=p->next;
            }
            else
            {
                head=(void*)head+n; 
                head->size=p->size-n-sizeof(node); //减去(allocated memory+一个node所占用的空间)，即为剩余空闲块的大小
                head->next=p->next;
            }
        else
        {
            if(p->size==n) //该内存块的大小正好为满足所需的内存块
            {
                before->next=p->next;
            }
            else
            {
                before->next=(void*)p+n;
                before->next->next=p->next; //将前一个空闲内存块的的next指向被分配的内存块的写一块空闲内存块
                before->next->size=p->size-n-sizeof(node); 
                
            }
        }

        //设置header
        h=(header*)p;
        h->size=n-8; //实际allocated memory的大小
        h->magic=1234567;
        return (void*)p+8;     //use type cast to void
                               //返回一个指向该对象的指针
    }

    int mem_free(void * ptr) //free teh memory object that ptr points to 
    {
        if(ptr==NULL)   //ptr=NULL，不进行任何操作
            return 0;

        if(*(int*)(ptr-4)!=1234567) //whether the magic number matches the expected value as a snaity check
            return -1;

        node*ptrn=(node*)(ptr-8);
        node*p=head,*before=head;
        int b_size=(ptrn->size)+sizeof(head);
        ptrn->size=b_size-sizeof(node);

        //找到临近的空闲块
        while(p!=NULL&&p<ptrn) //p指向所需要释放的内存块
        {
            before=p;
            p=p->next; //p为下一块空闲空间
        }

        if(p==head) //仅有一块内存块在堆上
        {
            head=ptrn;
            head->size=ptrn->size;
            head->next=p;
        }
        else 
        {
            before->next=ptrn;
            before->next->next=p; //ptrn->next指向下一块的空闲空间
            before->next->size=ptrn->size;
        }
     
        //coalescing
        if(ptrn==head) //释放块位于堆栈顶部
        {
            if((void*)ptrn+ptrn->size+sizeof(node)==ptrn->next) //跟下一块空闲块合并
            {
                ptrn->size=ptrn->size+ptrn->next->size+sizeof(node);
                ptrn->next=ptrn->next->next; 
            }
        }
        else if(ptrn->next==NULL) //释放块位于堆栈底部
        {
            if((void*)before+before->size+sizeof(node)==ptrn) //跟上一块空闲块合并
            {
                before->size=before->size+ptrn->size+sizeof(node);
                before->next=ptrn->next;
            }
        }
        //位于堆的中间区域
        else if((void*)before+before->size+sizeof(node)==ptrn&&(void*)ptrn+ptrn->size+sizeof(node)==ptrn->next) //上下都有内存块，跟上下两块合并
        {
            before->size=before->size+ptrn->size+ptrn->next->size+2*sizeof(node);
            before->next=ptrn->next->next;
        }
        else if((void*)before+before->size+sizeof(node)==ptrn) //跟上一块合并
        {
            before->size=before->size+ptrn->size+sizeof(node);
            before->next=ptrn->next;
        }
        else if((void*)ptrn+ptrn->size+sizeof(node)==ptrn->next) //跟下一块合并
        {
            ptrn->size=ptrn->size+ptrn->next->size+sizeof(node);
            ptrn->next=ptrn->next->next;
        }
        //ptrn->size=ptrn->size+sizeof(node) ！！！;
	return 0;
    }

    //将可用的内存的区域输出到屏幕上
    void mem_dump()
    {
        node*p=head;
        while(p!=NULL)
        {
            printf("%p size:%d\n",p,p->size); //依次输出下一个空闲内存块
            p=p->next;
        }
    }

