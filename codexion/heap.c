/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heap.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bramahef < bramahef@student.42antananar    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/25 00:00:00 by username          #+#    #+#             */
/*   Updated: 2026/08/25 23:20:23 by bramahef         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "coders.h"

// Allocation et initialisation du tas

t_heap	create_heap(int capacity)
{
	t_heap	heap;

	heap.capacity = capacity;
	heap.size = 0;
	heap.elements = malloc(sizeof(t_request) * (size_t)capacity);
	if (!heap.elements)
		heap.capacity = 0;
	return (heap);
}

// Libération de la mémoire du tas

void	free_heap(t_heap *heap)
{
	if (heap->elements)
		free(heap->elements);
	heap->elements = NULL;
	heap->size = 0;
	heap->capacity = 0;
}

// Compare la priorité de deux requêtes selon le scheduler actif
static bool	request_has_priority(t_request first, t_request second,
		t_scheduler scheduler)
{
	if (scheduler == E_SCHED_FIFO)
	{
		if (first.request_time == second.request_time)
			return (first.deadline < second.deadline);
		return (first.request_time < second.request_time);
	}
	if (first.deadline == second.deadline)
		return (first.request_time < second.request_time);
	return (first.deadline < second.deadline);
}

static void	heapify_up(t_heap *heap, t_scheduler scheduler)
{
	int			index;
	int			parent;
	t_request	tmp;

	index = heap->size - 1;
	while (index > 0)
	{
		parent = (index - 1) / 2;
		if (!request_has_priority(heap->elements[index], heap->elements[parent],
				scheduler))
			break ;
		tmp = heap->elements[index];
		heap->elements[index] = heap->elements[parent];
		heap->elements[parent] = tmp;
		index = parent;
	}
}

// Insère un élément et applique la remontée (Heapify Up)

void	heap_push(t_heap *heap, t_request req, t_scheduler scheduler)
{
	if (!heap->elements || heap->size >= heap->capacity)
		return ;
	heap->elements[heap->size] = req;
	heap->size++;
	heapify_up(heap, scheduler);
}
