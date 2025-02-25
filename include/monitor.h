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
 *	File:       monitor.h																												*
 *																																		*
 ****************************************************************************************************************************************/

#ifndef _EMPI_MONITOR_H_
#define _EMPI_MONITOR_H_

/****************************************************************************************************************************************
*
*	'EMPI_Get_tcomm'
*
****************************************************************************************************************************************/
void EMPI_Get_aggregated_tcomm (double *tcomm);

/****************************************************************************************************************************************
*
*	'EMPI_Get_tcomp'
*
****************************************************************************************************************************************/
void EMPI_Get_aggregated_tcomp (double *tcomp);

/****************************************************************************************************************************************
*
*	'EMPI_Get_exec_time'
*
****************************************************************************************************************************************/
void EMPI_Get_exec_time (double *extime);

/****************************************************************************************************************************************
*
*	'EMPI_Get_minprocs'
*
****************************************************************************************************************************************/
void EMPI_Get_minprocs (int *nprocs);

/****************************************************************************************************************************************
*
*	'EMPI_Set_maxprocs'
*
****************************************************************************************************************************************/
void EMPI_Set_maxprocs (int nprocs);

/****************************************************************************************************************************************
*
*	'EMPI_Set_policy'
*
****************************************************************************************************************************************/
void EMPI_Set_policy (int policy);

/****************************************************************************************************************************************
*
*	'EMPI_Set_niter'
*
****************************************************************************************************************************************/
void EMPI_Set_niter (int niter);

/****************************************************************************************************************************************
*
*	'EMPI_Monitor_init'
*
****************************************************************************************************************************************/
void EMPI_Monitor_init (void);

/****************************************************************************************************************************************
*
*	'EMPI_Monitor_end'
*
****************************************************************************************************************************************/
void EMPI_Monitor_end (int *rank, int *size, int iter, int maxiter, int *count, int *disp, int **vcount, int **vdisp, int *fc, char *argv[], char *bin);

/****************************************************************************************************************************************
*
*	'EMPI_Capture_comms'
*
****************************************************************************************************************************************/
void EMPI_Capture_comms (int mpi_op, int *size, MPI_Datatype datatype);

#endif
