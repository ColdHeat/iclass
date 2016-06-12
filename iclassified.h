/*
    iClass Key Diversification (iClass Demo)
    Copyright (C) 2011 Radboud University Nijmegen

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _ICLASSIFIED_H_
#define _ICLASSIFIED_H_

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include "stdbyte.h"

#define BUF_SIZE 0xff

size_t SCardCLICCTransmit(SCARDHANDLE hCard, byte_t* pbtSnd, size_t szSnd, byte_t* pbtRcv, size_t* pszRcv);
size_t SCardCLICCTransmitS(SCARDHANDLE hCard, byte_t* pbtSnd, size_t szSnd, byte_t* pbtRcv, size_t* pszRcv);
bool SendApdu(SCARDHANDLE hCard, byte_t* pbtApdu, size_t szApduLen, byte_t* pbtRecv, size_t* pszRxLen);

#endif // _ICLASSIFIED_H_
