#include "coders.h"

// Allocation et initialisation du tas
t_heap create_heap(int capacity)
{
    t_heap heap;

    heap.capacity = capacity;
    heap.size = 0;
    heap.elements = malloc(sizeof(t_request) * capacity);
    if (!heap.elements)
        heap.capacity = 0;
    return (heap);
}

// Libération de la mémoire du tas
void free_heap(t_heap *heap)
{
    if (heap->elements)
        free(heap->elements);
    heap->elements = NULL;
    heap->size = 0;
    heap->capacity = 0;
}

// Compare la priorité de deux requêtes selon le scheduler actif
static bool has_higher_priority(t_request req1, t_request req2, t_scheduler scheduler)
{
    if (scheduler == E_SCHED_FIFO)
    {
        if (req1.request_time == req2.request_time)
            return (req1.deadline < req2.deadline);
        return (req1.request_time < req2.request_time);
    }
    else // E_SCHED_EDF
    {
        if (req1.deadline == req2.deadline)
            return (req1.request_time < req2.request_time);
        return (req1.deadline < req2.deadline);
    }
}

// Insère un élément et applique la remontée (Heapify Up)
void heap_push(t_heap *heap, t_request req, t_scheduler scheduler)
{
    int       i;
    int       parent;
    t_request tmp;

    if (!heap->elements || heap->size >= heap->capacity)
        return ;
    i = heap->size;
    heap->elements[i] = req;
    heap->size++;

    while (i > 0)
    {
        parent = (i - 1) / 2;
        if (has_higher_priority(heap->elements[i], heap->elements[parent], scheduler))
        {
            tmp = heap->elements[i];
            heap->elements[i] = heap->elements[parent];
            heap->elements[parent] = tmp;
            i = parent;
        }
        else
            break ;
    }
}

// Extrait la requête la plus prioritaire et réorganise (Heapify Down)
t_request heap_pop(t_heap *heap, t_scheduler scheduler)
{
    t_request top;
    t_request tmp;
    int       i;
    int       left;
    int       right;
    int       smallest;

    if (heap->size <= 0)
    {
        top.coder = NULL;
        return (top);
    }
    top = heap->elements[0];
    heap->elements[0] = heap->elements[heap->size - 1];
    heap->size--;

    i = 0;
    while (2 * i + 1 < heap->size)
    {
        left = 2 * i + 1;
        right = 2 * i + 2;
        smallest = left;

        if (right < heap->size && 
            has_higher_priority(heap->elements[right], heap->elements[left], scheduler))
            smallest = right;

        if (has_higher_priority(heap->elements[smallest], heap->elements[i], scheduler))
        {
            tmp = heap->elements[i];
            heap->elements[i] = heap->elements[smallest];
            heap->elements[smallest] = tmp;
            i = smallest;
        }
        else
            break ;
    }
    return (top);
}