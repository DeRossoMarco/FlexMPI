/**
* @version		FlexMPI v3.1
* @copyright	Copyright (C) 2018 Universidad Carlos III de Madrid. All rights reserved.
* @license		GNU/GPL, see LICENSE.txt
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You have received a copy of the GNU General Public License in LICENSE.txt
* also available in <http://www.gnu.org/licenses/gpl.html>.
*
* See COPYRIGHT.txt for copyright notices and details.
*/

/****************************************************************************************************************************************
 *																																		*
 *	FLEX-MPI																															*
 *																																		*
 *	File:       memalloc.h																												*
 *																																		*
 ****************************************************************************************************************************************/

#ifndef _EMPI_MEMALLOC_H_
#define _EMPI_MEMALLOC_H_

/****************************************************************************************************************************************
*
*	'EMPI_malloc'
*
****************************************************************************************************************************************/
void *EMPI_malloc (size_t size);

/****************************************************************************************************************************************
*
*	'EMPI_calloc'
*
****************************************************************************************************************************************/
void *EMPI_calloc (size_t nmemb, size_t size);

/****************************************************************************************************************************************
*
*	'EMPI_free'
*
****************************************************************************************************************************************/
void EMPI_free (void *addr, char *id);

#endif
