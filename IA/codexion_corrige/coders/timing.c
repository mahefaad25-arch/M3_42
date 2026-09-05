/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::  */
/*   timing.c                                          :+:      :+:    :+:  */
/*                                                    +:+ +:+         +:+    */
/*   By: student <student@student.42.fr>            +#+  +:+       +#+      */
/*                                                +#+#+#+#+#+   +#+         */
/*   Created: 2026/09/05 00:00:00 by student           #+#    #+#          */
/*   Updated: 2026/09/05 00:00:00 by student          ###   ########.fr    */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

/*
** Split out of utils.c: with ft_usleep_ms included, utils.c had 6
** functions, one over the Norm's 5-functions-per-file limit.
*/

/*
** Small sleep helper working in milliseconds, sleeping in short slices
** (1ms) so a coder can react quickly if the simulation stops while it
** is sleeping (debug/refactor phases).
*/
void	ft_usleep_ms(long ms)
{
	long	slept;

	slept = 0;
	while (slept < ms)
	{
		usleep(1000);
		slept++;
	}
}
