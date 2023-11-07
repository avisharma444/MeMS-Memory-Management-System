/*
All the main functions with respect to the MeMS are inplemented here
read the function discription for more details

NOTE: DO NOT CHANGE THE NAME OR SIGNATURE OF FUNCTIONS ALREADY PROVIDED
you are only allowed to implement the functions 
you can also make additional helper functions a you wish

REFER DOCUMENTATION FOR MORE DETAILS ON FUNSTIONS AND THEIR FUNCTIONALITY
*/
// add other headers as required
#include<math.h>

#include<stdio.h>
#include<stdlib.h>
#include <sys/mman.h>
#include<stdint.h>

/*
Use this macro where ever you need PAGE_SIZE.
As PAGESIZE can differ system to system we should have flexibility to modify this 
macro to make the output of all system same and conduct a fair evaluation. 
*/
#define PAGE_SIZE 4096


/*
Initializes all the required parameters for the MeMS system. The main parameters to be initialized are:
1. the head of the free list i.e. the pointer that points to the head of the free list
2. the starting MeMS virtual address from which the heap in our MeMS virtual address space will start.
3. any other global variable that you want for the MeMS implementation can be initialized here.
Input Parameter: Nothing
Returns: Nothing
*/
int count =0;
struct MainNode{
    size_t size;
    struct MainNode* next;
    struct MainNode* prev;
    int used_flag;
    struct SubChainNode* sub;
    int vas;
    int vae;
};

typedef struct SubChainNode{
    size_t size;
    struct SubChainNode* prev;
    struct SubChainNode* next;     
    struct MainNode* main;
    int flag;  //0 - hole , 1 - process
    void* m_start;
    int vas;
    int vae;
}SubChainNode;

struct  MainNode* main_head; //head of the structs page
void* structs_curr;
void* mem_curr;
void* mem_ptr;
struct SubChainNode* sub_head;
int starting_virual_addr;
struct MainNode* main_ptr;
int total_pages;
void mems_init(){
    main_ptr = NULL;
    structs_curr = NULL;
    mem_curr = NULL;
    main_head = NULL;
    mem_ptr = NULL;
    sub_head = NULL;
    total_pages = 0;
    starting_virual_addr = 1000;
}
/*
This function will be called at the end of the MeMS system and its main job is to unmap the 
allocated memory using the munmap system call.
Input Parameter: Nothing
Returns: Nothing
*/

void mems_finish(){
    struct MainNode* main_node=main_head;
    while(main_node!=NULL){
        munmap(main_node->sub->m_start,(main_node->vae-main_node->vas+1));

        if(main_node->next==NULL){
            break;
        }
        main_node=main_node->next;
    }
}



/*
Allocates memory of the specified size by reusing a segment from the free list if 
a sufficiently large segment is available. 

Else, uses the mmap system call to allocate more memory on the heap and updates 
the free list accordingly.

Note that while mapping using mmap do not forget to reuse the unused space from mapping
by adding it to the free list.
Parameter: The size of the memory the user program wants
Returns: MeMS Virtual address (that is created by MeMS)
*/ 
int custom_ceil(double x) {
    int int_part = (int)x;
    if (x > int_part) {
        return int_part + 1;
    }
    return (int)x;
}
int current_virtual_address = 1000;  // Starting virtual address
int initial_value;
void* mems_malloc(size_t size)
{
    count++;
    // printf("count at - %d\n",count);
    float d= (float)size/PAGE_SIZE;
    int req_pages;
    req_pages= custom_ceil(d);
    // printf("\ntotal pages -- %d\n",req_pages);
    int firstflag =0;
    // we intialized a page just for memory here    
    if(mem_ptr == NULL){
        //mmap for storing allocated memory.
        mem_ptr = mmap(NULL, (PAGE_SIZE)*(req_pages), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
        if (mem_ptr == MAP_FAILED) {
            perror("mmap failed");
            exit(1);
        }
        mem_curr = mem_ptr;
        total_pages =req_pages;
    }


    
    


    if(main_head == NULL)
    {
        firstflag =1;
    
        //mmap for storing all the structs.
        structs_curr = mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
        if (structs_curr == MAP_FAILED) {
            perror("mmap failed");
            exit(1);
        }
        // printf("Intially structs curr at - %p\n",structs_curr);
        initial_value = (int)structs_curr;
                                                        // printf("struct curr now - %d\n",initial_value-(int)structs_curr);

        main_head = structs_curr; 
        structs_curr += (sizeof(struct MainNode));
                                                        // printf("struct curr now - %d\n",initial_value-(int)structs_curr);

        main_head->next = NULL;
        main_head->prev = NULL;
        
        main_head->size = req_pages*PAGE_SIZE;
        main_head->vas = starting_virual_addr;
        
        main_head->vae = starting_virual_addr+main_head->size-1;

        //subchain
        sub_head = structs_curr;
        if(structs_curr+sizeof(struct SubChainNode)>PAGE_SIZE){
                                structs_curr = mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
                                    if (structs_curr == MAP_FAILED) {
                                        perror("mmap failed");
                                        exit(1);
                                    }
                                    initial_value = (int)structs_curr;
                            }//------------------------------------------------------------------------------------------------------------------------------------
        structs_curr+= (sizeof(struct SubChainNode));
                                                        // printf("struct curr now - %d\n",initial_value-(int)structs_curr);

        sub_head->main = main_head;
        sub_head->next=NULL;
        sub_head->prev = NULL;
        sub_head->flag = 0;//0 - hole , 1 - filled
        sub_head->size = sub_head->main->size;//hole of whole pagesize
        sub_head->m_start = mem_curr;
        sub_head->vas = sub_head->main->vas;
        sub_head->vae = sub_head->vas+sub_head->size;
        //
        main_head->sub = sub_head;
        //main_ptr tracks which main node is being used currently.
        main_ptr = main_head;
    }



        //we have to allocate = size_t size;
        struct MainNode* ptr = main_head;
        while(ptr->next!=NULL&&ptr->size<=size){
            // printf("ptr size - %d size - %d\n",ptr->size,size);
            // printf("iterating main node size -> %d\n",ptr->size);
            ptr = ptr->next;
        }
        // printf("bahaar -- ptr size - %d size - %d\n",ptr->size,size);
        // printf("milgya %d\n",ptr->vas);
        // printf("main - %d\n",sizeof(struct MainNode));
        //  printf("sub - %d\n",sizeof(struct SubChainNode));

            // printf("2\n");

        if(ptr->size>=size)

        {
            
            // printf("IN here");
            // printf("1\n");
            struct MainNode* main_node = main_head;
            struct SubChainNode* sub_node = main_node->sub;

            while (main_node != NULL) {
                while (sub_node != NULL) {
                    if (sub_node->flag == 0 && sub_node->size >= size) {
                        // printf("#\n");
                        // Found a hole segment that is large enough

                        //this is the size of the total free area
                        int initial_hole = sub_node->size;

                        sub_node->flag = 1; // Mark it as a process segment

                        //assign size to this process node. size=requested size for allocation 
                        sub_node->size = size;



                        sub_node->vae = sub_node->vas + sub_node->size-1;
                        mem_curr += size;

                        // Check if there's space left for a new hole
                        if (initial_hole - size >0) {
                            
                            struct SubChainNode* new_hole = structs_curr;
                            if(structs_curr+sizeof(struct SubChainNode)>PAGE_SIZE){
                                structs_curr = mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
                                    if (structs_curr == MAP_FAILED) {
                                        perror("mmap failed");
                                        exit(1);
                                    }
                                    initial_value = (int)structs_curr;
                            }
                            structs_curr+=(sizeof(struct SubChainNode));
                            // printf("struct curr now - %d\n",initial_value-(int)structs_curr);

                            new_hole->vas = sub_node->vae + 1;
                            new_hole->size = initial_hole - size;
                            new_hole->vae = new_hole->vas + new_hole->size-1;
                            new_hole->flag = 0;
                            new_hole->main = sub_node->main;
                            new_hole->prev = sub_node;
                            new_hole->next = sub_node->next;//could be null too
                            new_hole->m_start = mem_curr;//could  be sub_node->m_start+sub_node->size;

                            if (sub_node->next != NULL) {
                                sub_node->next->prev = new_hole;
                            }

                            sub_node->next = new_hole;
                        }
                                                    sub_node->main->size -= size ;


                        // Update the current virtual address and return the allocated virtual address
                        
                        int* allocated_address = (int*)current_virtual_address;
                        // printf("1.return %d real %d",allocated_address,current_virtual_address);
                        current_virtual_address += size;
                        // printf("va inced to - %d  %u  mem_curr at - %u  size in main_head - %u  ",current_virtual_address,sub_node->m_start,mem_curr,main_head->size );
                        // printf("end - %d\n",total_pages);

                        // printf("struct page counter at(end) - %d\n",initial_value-(int)structs_curr);
                        return allocated_address;
                    }

                    sub_node = sub_node->next;
                }

                main_node = main_node->next;
                if (main_node != NULL) {
                    sub_node = main_node->sub;
                }
            }

        }else
        {
                        // printf("3\n");

            
            void* new_page = NULL;
            new_page = mmap(NULL, (PAGE_SIZE)*req_pages, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
                        // printf("4\n");

   
            current_virtual_address  = (int)(starting_virual_addr+(PAGE_SIZE*(total_pages)));
            total_pages+=req_pages;

            mem_curr = new_page;
            mem_curr+=size;
            struct MainNode* new_main_node = structs_curr;
                                        if(structs_curr+sizeof(struct MainNode)>PAGE_SIZE){
                                structs_curr = mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
                                    if (structs_curr == MAP_FAILED) {
                                        perror("mmap failed");
                                        exit(1);
                                    }
                                    initial_value = (int)structs_curr;
                            }
            structs_curr+=sizeof(struct MainNode);
                                                            // printf("struct curr now - %d\n",initial_value-(int)structs_curr);

            new_main_node->size=req_pages*PAGE_SIZE;
            struct MainNode* ptr2 = main_head;
                        // printf("5\n");

            while(ptr2->next!=NULL){
                ptr2 = ptr2->next;
            }
            ptr2->next = new_main_node;
            new_main_node->prev = ptr2;
            new_main_node->vas = ptr2->vae+1;//-----------------------------------------
        
            new_main_node->vae = new_main_node->vas+new_main_node->size-1;
            new_main_node->next = NULL;
            main_ptr = new_main_node;
                        // printf("6\n");


            struct SubChainNode* new_sub_node = structs_curr;
            if(structs_curr+sizeof(struct SubChainNode)>PAGE_SIZE){
                                structs_curr = mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
                                    if (structs_curr == MAP_FAILED) {
                                        perror("mmap failed");
                                        exit(1);
                                    }
                                    initial_value = (int)structs_curr;
                            }
            
            structs_curr+=sizeof(struct SubChainNode);
                                                            // printf("struct curr now - %d\n",initial_value-(int)structs_curr);

            new_sub_node->main = new_main_node;
            // printf("%d\n",new_sub_node->main->vas);
            new_sub_node->m_start = new_page;
            new_sub_node->flag = 0;
            new_sub_node->prev= NULL;
            new_sub_node->next = NULL;
                        // printf("7\n");
                        //     printf("8\n");

            new_sub_node->size = new_sub_node->main->size;
                        // printf("10\n");
                        //                             printf("8\n");


            
            int x = new_sub_node->main->vas;
            // printf("00\n");
            // printf("va - - %d\n",x);
            new_sub_node->vas = x;
                        // printf("9\n");

            new_sub_node->vae = new_sub_node->vas+new_sub_node->size;
            new_main_node->sub = new_sub_node;

                        // printf("struct page counter at(end) - %d  %d\n",initial_value,initial_value-(int)structs_curr);
                        //             printf("8\n");

            return mems_malloc(size);

        }
    
}


/*
this function print the stats of the MeMS system like
1. How many pages are utilised by using the mems_malloc
2. how much memory is unused i.e. the memory that is in freelist and is not used.
3. It also prints details about each node in the main chain and each segment (PROCESS or HOLE) in the sub-chain.
Parameter: Nothing
Returns: Nothing but should print the necessary information on STDOUT
*/
void mems_print_stats(){
    printf("\n");
    struct MainNode* main_node=main_head;
    int mainchain_length=0;
    size_t unused_space=0;

    while(main_node!=NULL)
    {
        printf("MAIN[%d:%d]-> ",main_node->vas,main_node->vae);
        struct SubChainNode* sub_node=main_node->sub;

        while(sub_node!=NULL){
            if(sub_node->flag==0){
                printf("H[%d:%d] <-> ",sub_node->vas,sub_node->vae);
                unused_space+=(sub_node->vae-sub_node->vas)+1;
            }
            else if(sub_node->flag==1){
                printf("P[%d:%d] <-> ",sub_node->vas,sub_node->vae);
            }
            else{
                printf("Flag neither zero nor 1!!<->");
            }
            sub_node=sub_node->next;
        }
        printf("NULL\n");
        main_node=main_node->next;
        mainchain_length++;
    }
    printf("Pages used:    %d\n",total_pages);
    printf("Space unused:    %ld\n",unused_space);
    printf("Main Chain Length:      %d\n",mainchain_length);
    printf("Sub-chain Length array: [");
    // struct MainNode* main_node=main_head;
    main_node=main_head;
    int sub_nodes;
    while(main_node!=NULL){
        struct SubChainNode* sub_node=main_node->sub;
        sub_nodes=0;
        while(sub_node!=NULL){
            sub_nodes++;
            sub_node=sub_node->next;
        }
        printf("%d, ",sub_nodes);
        main_node=main_node->next;
    }
    printf("]\n");
    printf("\n");
}




/*
Returns the MeMS physical address mapped to ptr ( ptr is MeMS virtual address).
Parameter: MeMS Virtual address (that is created by MeMS)
Returns: MeMS physical address mapped to the passed ptr (MeMS virtual address).
*/
void *mems_get(void*v_ptr){
    struct MainNode* main_node=main_head;
    while(main_node!=NULL){
        if((intptr_t)v_ptr>=main_node->vas && (intptr_t)v_ptr<=main_node->vae){
            struct SubChainNode* sub_node=main_node->sub;
            while(sub_node!=NULL){
                if((intptr_t)v_ptr>=sub_node->vas && (intptr_t)v_ptr<=sub_node->vae){
                    // return (int)(v_ptr - sub_node->vas) + sub_node->m_start;
                    return (void *)((char *)sub_node->m_start + (size_t)(v_ptr - sub_node->vas));
                }
                sub_node=sub_node->next;
            }
        }
        main_node=main_node->next;
    }
    perror("can't find the memory location to perform mems_get()\n");
}
void mems_free(void *v_ptr){
    //v_ptr = memory to be freed
    int target = (int)v_ptr;
    // printf("return %d real *d",target,v_ptr);
    // printf("mem to be freed -%d\n",target);
    struct MainNode* main_temp = main_head;
    while(main_temp!=NULL)
    {
        struct SubChainNode* sub_temp = main_temp->sub;
        while(sub_temp!=NULL)
        {
            if(sub_temp->vas == target){
                if(sub_temp->flag==0){
                    printf("Already a hole node. No need to free its memory.\n");
                    return;
                }
                sub_temp->flag = 0;//mark it a hole
                int mem_size= sub_temp->size;
                if(sub_temp->next!=NULL&&sub_temp->next->flag == 0){
                    //now we will have to merge these holes
                    struct SubChainNode* next_sub = sub_temp->next;
                    sub_temp->vae= next_sub->vae;
                    sub_temp->next = next_sub->next;

                    if(next_sub->next!=NULL){
                        next_sub->next->prev = sub_temp;
                    }
                    next_sub->prev = NULL;
                    next_sub->next = NULL;
                    sub_temp->size+=next_sub->size;
                                        structs_curr-=sizeof(struct SubChainNode);


                } 
                if(sub_temp->prev!=NULL&&sub_temp->prev->flag == 0){
                    //merge node with prev node
                    struct SubChainNode* prev_sub = sub_temp->prev;

                    sub_temp->vas = prev_sub->vas;
                    sub_temp->prev = prev_sub->prev;
                    if(prev_sub->prev!=NULL){
                        prev_sub->prev->next = sub_temp;
                    }
                    prev_sub->next=NULL;
                    prev_sub->prev = NULL;
                    sub_temp->size+=prev_sub->size;
                    structs_curr-=sizeof(struct SubChainNode);
                }
                sub_temp->main->size+=mem_size;
            }
            sub_temp=sub_temp->next;
        }
        main_temp = main_temp->next;
    }   
}
