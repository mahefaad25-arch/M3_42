/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heap.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bramahef < bramahef@student.42antananar    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/16 18:27:05 by bramahef          #+#    #+#             */
/*   Updated: 2026/09/16 18:27:06 by bramahef         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

/*
** Allocates the array backing the heap. "capacity" is the maximum
** number of coders that can wait for one dongle at the same time
** (at most number_of_coders). Returns 0 on success, -1 on malloc
** failure.
*/
int	heap_init(t_heap *heap, int capacity, t_sched policy)
{
	heap->data = malloc(sizeof(t_req) * capacity);
	if (heap->data == NULL)
		return (-1);
	heap->size = 0;
	heap->capacity = capacity;
	heap->policy = policy;
	return (0);
}

/*
** Frees the heap's array.
*/
void	heap_destroy(t_heap *heap)
{
	free(heap->data);
	heap->data = NULL;
	heap->size = 0;
	heap->capacity = 0;
}

/*
** Strict ordering used by the heap: returns 1 if request "a" must
** be served before request "b".
**
** fifo -> the request that arrived first wins.
** edf  -> the request with the earliest burnout deadline wins.
**         Deadlines can be equal (timestamps are in milliseconds),
**         so the subject requires a tie-breaker to keep the policy
**         fully deterministic: we then compare arrival time, and
**         finally the coder number, which is unique. Two requests
**         can therefore never be "equal", and the heap always makes
**         the same choice for the same situation.
*/
int	req_is_before(t_heap *heap, t_req *a, t_req *b)
{
	if (heap->policy == CX_EDF && a->deadline_ms != b->deadline_ms)
		return (a->deadline_ms < b->deadline_ms);
	if (a->arrival_ms != b->arrival_ms)
		return (a->arrival_ms < b->arrival_ms);
	return (a->coder_id < b->coder_id);
}

/*
** Swaps two requests inside the heap array.
*/
void	heap_swap(t_req *a, t_req *b)
{
	t_req	tmp;

	tmp = *a;
	*a = *b;
	*b = tmp;
}
