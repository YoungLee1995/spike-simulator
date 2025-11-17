#ifndef _sm_dev_queue_h
#define _sm_dev_queue_h

#include "sm_common.h"
#include <iostream>
#include <queue>
#include <stdexcept>
#include <tuple>
#include <variant>

using TSTuple = tuple<dsa_ops_t, TMM_INFO, MUL_Desc, CONV_Desc, PROC_Desc, ELW_Desc>;
using DMATuple = tuple<dsa_ops_t, DMA_INFO, DMA_LS_Desc, DMA_CP_Desc>;
using LLMTuple = tuple<dsa_ops_t, LLM_INFO, LLM_Desc>;
using TSElwType = std::variant<DDEP_Rls_Info, DDEP_Use_Info, TSTuple>;
using DMAElwType = std::variant<DDEP_Rls_Info, DDEP_Use_Info, DMATuple>;
using LLMElwType = std::variant<DDEP_Rls_Info, DDEP_Use_Info, LLMTuple>;

template <typename T>
void enqueue(std::queue<T> &q, const T &value)
{
    q.push(value);
}
template <typename T>
bool empty(const std::queue<T> &q) { return q.empty(); }
template <typename T>
void assert_empty(const std::queue<T> &q)
{
    if (empty(q))
        throw out_of_range("Queue is empty");
}

template <typename T>
T dequeue(std::queue<T> &q)
{
    assert_empty(q);
    T value = move(q.front());
    q.pop();
    return value;
}

template <typename T>
T &front(std::queue<T> &q)
{
    assert_empty(q);
    return q.front();
}
template <typename T>
void pop(std::queue<T> &q)
{
    assert_empty(q);
    q.pop();
}


#endif