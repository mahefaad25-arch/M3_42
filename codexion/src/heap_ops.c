/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heap_ops.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bramahef < bramahef@student.42antananar    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/16 18:27:00 by bramahef          #+#    #+#             */
/*   Updated: 2026/09/16 18:27:01 by bramahef         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "codexion.h"

/*
** Moves the element at index i up until its parent comes before it.
** In a binary heap stored in an array, the parent of index i is
** at index (i - 1) / 2. Cost: O(log n).
*/
void	heap_sift_up(t_heap *heap, int i)
{
	int	parent;

	while (i > 0)
	{
		parent = (i - 1) / 2;
		if (req_is_before(heap, &heap->data[parent], &heap->data[i]))
			break ;
		heap_swap(&heap->data[parent], &heap->data[i]);
		i = parent;
	}
}

/*
** Moves the element at index i down until both its children come
** after it. Children of index i are at 2i + 1 and 2i + 2.
** Cost: O(log n).
*/
void	heap_sift_down(t_heap *heap, int i)
{
	int	best;
	int	left;

	while (1)
	{
		best = i;
		left = 2 * i + 1;
		if (left < heap->size
			&& req_is_before(heap, &heap->data[left], &heap->data[best]))
			best = left;
		if (left + 1 < heap->size
			&& req_is_before(heap, &heap->data[left + 1], &heap->data[best]))
			best = left + 1;
		if (best == i)
			break ;
		heap_swap(&heap->data[i], &heap->data[best]);
		i = best;
	}
}

/*
** Inserts a new request at the end of the array, then sifts it up
** to restore the heap property. The caller must hold the dongle's
** mutex.
*/
void	heap_push(t_heap *heap, int coder_id, long arrival, long deadline)
{
	int	i;

	if (heap->size >= heap->capacity)
		return ;
	i = heap->size;
	heap->data[i].coder_id = coder_id;
	heap->data[i].arrival_ms = arrival;
	heap->data[i].deadline_ms = deadline;
	heap->size = i + 1;
	heap_sift_up(heap, i);
}

/*
** Removes the request of a given coder wherever it is in the heap
** (not only the root): the last element takes its place, then we
** restore the heap property in both directions.
*/
void	heap_remove(t_heap *heap, int coder_id)
{
	int	i;

	i = 0;
	while (i < heap->size && heap->data[i].coder_id != coder_id)
		i++;
	if (i == heap->size)
		return ;
	heap->size--;
	if (i == heap->size)
		return ;
	heap->data[i] = heap->data[heap->size];
	heap_sift_up(heap, i);
	heap_sift_down(heap, i);
}

/*
** Returns the coder number sitting at the root of the heap, i.e.
** the one the dongle must serve next according to the scheduler.
** Returns -1 if nobody is waiting.
*/
int	heap_top_id(t_heap *heap)
{
	if (heap->size == 0)
		return (-1);
	return (heap->data[0].coder_id);
}
