#include "job.h"
#include "policy.h"
#include "timerange.h"
#include "stdlib.h"

struct ListNode {
    void* val;
    struct ListNode* next;
};

typedef struct ListNode listnode_t; 

void free_list(listnode_t* head) {
    listnode_t* temp = head;
    while (head) {
        temp = head->next;
        free(head);
        head = temp;
    }
}

int schedule_jobs(listnode_t* rigid, listnode_t* flexible, uint8_t NUM_THREADS) {
    return 1;
}

int schedule(Job* jobs, uint8_t num_jobs, uint8_t NUM_THREADS, uint64_t GRANULARITY) {
    if (num_jobs == 0) {
        return 1;
    }

    listnode_t* head_rigid = NULL;
    listnode_t* curr_rigid = NULL;

    listnode_t* head_flexible = NULL;
    listnode_t* curr_flexible = NULL;

    for (uint8_t i = 0; i < num_jobs; i++) {
        Job* curr_job = jobs + i;
        listnode_t* node = malloc(sizeof(listnode_t));
        if (!node) {
            perror("malloc");
            free_list(head_rigid);
            free_list(curr_rigid);
            return -1;
        }
        node->val = (void*)curr_job;
        node->next = NULL;

        if (trequals(curr_job->schedulable_timerange, curr_job->default_scheduled_timerange)) {
            if (head_rigid == NULL) {
                head_rigid = node;
                curr_rigid = node;
            } else {
                curr_rigid->next = node;
                curr_rigid = node;
            }
        } else {
            if (head_flexible == NULL) {
                head_flexible = node;
                curr_flexible = node;
            } else {
                curr_flexible->next = node;
                curr_flexible = node;
            }
        }
    }

    schedule_jobs(head_rigid, head_flexible, NUM_THREADS);
}