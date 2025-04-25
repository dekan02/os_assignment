/*
 * Copyright (C) 2025 pdnguyen of HCMC University of Technology VNU-HCM
 */

/* Sierra release
 * Source Code License Grant: The authors hereby grant to Licensee
 * personal permission to use and modify the Licensed Source Code
 * for the sole purpose of studying while attending the course CO2018.
 */

#include "common.h"
#include "syscall.h"
#include "stdio.h"
#include "libmem.h"
#include "queue.h"
int __sys_killall(struct pcb_t *caller, struct sc_regs* regs)
{
    char proc_name[100];
    uint32_t data;
    int i, j;
    int terminated = 0;

    //hardcode for demo only
    uint32_t memrg = regs->a1;
    
    /* Get name of the target proc */
    int i = 0;
    data = 0;
    while(data != -1){
        libread(caller, memrg, i, &data);
        proc_name[i]= data;
        if(data == -1) proc_name[i]='\0';
        i++;
    }
    // To debug
    printf("The procname retrieved from memregionid %d is \"%s\"\n", memrg, proc_name);

    /* Traverse proclist to terminate the proc */
    // Check running list
    for (i = 0; i < caller->running_list->size; i++) {
        if (strcmp(caller->running_list->proc[i]->path, proc_name) == 0) {
            // Found a match in running list
            struct pcb_t *proc = caller->running_list->proc[i];
            
            // Free process memory
            free_pcb_memph(proc);
            
            for (j = i; j < caller->running_list->size - 1; j++) {
                caller->running_list->proc[j] = caller->running_list->proc[j + 1];
            }
            caller->running_list->size--;
            terminated = 1;
            i--;
        }
    }

#ifdef MLQ_SCHED
    // Check MLQ ready queue
    struct queue_t *mlq_ready_queue = caller->mlq_ready_queue;
    if (mlq_ready_queue != NULL) { // Check if the queue exists
        pthread_mutex_lock(&mlq_ready_queue); // Lock the MLQ queue
        for (i = 0; i < mlq_ready_queue->size; i++) {
            if (strcmp(mlq_ready_queue->proc[i]->path, proc_name) == 0) {
                // Found a match in MLQ ready queue
                struct pcb_t *proc = mlq_ready_queue->proc[i];
                
                // Free process memory
                free_pcb_memph(proc);
                
                // Remove from MLQ ready queue
                for (j = i; j < mlq_ready_queue->size - 1; j++) {
                    mlq_ready_queue->proc[j] = mlq_ready_queue->proc[j + 1];
                }
                mlq_ready_queue->size--;
                terminated = 1;
                i--;
            }
        }
        pthread_mutex_unlock(&mlq_ready_queue); // Unlock the MLQ queue
    }
#endif
    // Debug
    if (terminated) {
        printf("Successfully terminated all processes with name \"%s\"\n", proc_name);
    } else {
        printf("No processes found with name \"%s\"\n", proc_name);
    }

    return 0; 
}
