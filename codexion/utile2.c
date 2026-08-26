/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utile2.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bramahef < bramahef@student.42antananar    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/25 23:20:46 by bramahef          #+#    #+#             */
/*   Updated: 2026/08/25 23:21:05 by bramahef         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "coders.h"

static void	heapify_down(t_heap *heap, int index, t_scheduler scheduler)
{
	int			left;
	int			right;
	int			swap_index;
	t_request	tmp;

	while (1)
	{
		left = index * 2 + 1;
		right = left + 1;
		swap_index = index;
		if ((left < heap->size) && (request_has_priority(heap->elements[left],
					heap->elements[swap_index], scheduler)))
		{
			swap_index = left;
		}
		if (right < heap->size && request_has_priority(heap->elements[right],
				heap->elements[swap_index], scheduler))
			swap_index = right;
		if (swap_index == index)
			return ;
		tmp = heap->elements[index];
		heap->elements[index] = heap->elements[swap_index];
		heap->elements[swap_index] = tmp;
		index = swap_index;
	}
}

// Extrait la requête la plus prioritaire et réorganise (Heapify Down)

t_request	heap_pop(t_heap *heap, t_scheduler scheduler)
{
	t_request	top;

	top.coder = NULL;
	top.request_time = 0;
	top.deadline = 0;
	if (heap->size <= 0)
		return (top);
	top = heap->elements[0];
	heap->elements[0] = heap->elements[heap->size - 1];
	heap->size--;
	if (heap->size > 0)
		heapify_down(heap, 0, scheduler);
	return (top);
}
