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
 *	File:       rdata.h																													*
 *																																		*
 ****************************************************************************************************************************************/

#ifndef _EMPI_RDATA_H_
#define _EMPI_RDATA_H_

/****************************************************************************************************************************************
*
*	'EMPI_DAlltoallv'
*
****************************************************************************************************************************************/
void EMPI_DAlltoallv (void *sendbuf, int sendcnt, int sdispl, MPI_Datatype sendtype, void *recvbuf, int recvcnt, int rdispl, MPI_Datatype recvtype, MPI_Comm comm);

/****************************************************************************************************************************************
*
*	'EMPI_SAlltoallv'
*
****************************************************************************************************************************************/
void EMPI_SAlltoallv (int *sendrow, int *sendcol, void *sendval, int sendcnt, int sdispl, int **recvrow, int **recvcol, void **recvval, int recvcnt, int rdispl, MPI_Datatype datatype, MPI_Comm comm);

/****************************************************************************************************************************************
*
*	'EMPI_Rdata'
*
****************************************************************************************************************************************/
void EMPI_Rdata (int scount, int sdispl, int rcount, int rdispl);

/****************************************************************************************************************************************
*
*	'EMPI_Rdata_spawn_cost'
*
****************************************************************************************************************************************/
void EMPI_Rdata_spawn_cost (int size, int nprocs, double *cost);

/****************************************************************************************************************************************
*
*	'EMPI_Rdata_remove_cost'
*
****************************************************************************************************************************************/
void EMPI_Rdata_remove_cost (int size, int nprocs, double *cost);

/****************************************************************************************************************************************
*
*	'EMPI_Register_size'
*
****************************************************************************************************************************************/
void EMPI_Register_size (int size);

/****************************************************************************************************************************************
*
*	'EMPI_Register_sparse'
*
****************************************************************************************************************************************/
void EMPI_Register_sparse (char *id, void *addr_row, void *addr_col, void *addr_val, MPI_Datatype datatype, int size, int nnz);

/****************************************************************************************************************************************
*
*	'EMPI_Register_vector'
*
****************************************************************************************************************************************/
void EMPI_Register_vector (char *id, void *addr, MPI_Datatype datatype, int size, int mapping);

/****************************************************************************************************************************************
*
*	'EMPI_Register_dense'
*
****************************************************************************************************************************************/
void EMPI_Register_dense (char *id, void *addr, MPI_Datatype datatype, int size, int mapping);

/****************************************************************************************************************************************
*
*	'EMPI_Deregister_shared'
*
****************************************************************************************************************************************/
void EMPI_Deregister_shared (char *id);

/****************************************************************************************************************************************
*
*	'EMPI_Get_addr'
*
****************************************************************************************************************************************/
void *EMPI_Get_addr (char *id);

/****************************************************************************************************************************************
*
*	'EMPI_Get_addr_sparse'
*
****************************************************************************************************************************************/
void EMPI_Get_addr_sparse (char *id, void **addr_row, void **addr_col, void **addr_val);

/****************************************************************************************************************************************
*
*	'EMPI_Get_shared'
*
****************************************************************************************************************************************/
void EMPI_Get_shared (int *it);


/****************************************************************************************************************************************
*
*	'EMPI_alloc'
*
****************************************************************************************************************************************/

void EMPI_alloc();

/****************************************************************************************************************************************
*
*	'EMPI_status_alloc'
*
****************************************************************************************************************************************/

int EMPI_status_alloc();

#endif
