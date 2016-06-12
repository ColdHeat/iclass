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

#include <string.h>
#include <stdio.h>
#include <openssl/des.h>
#include <windows.h>
#include <winscard.h>
#include "iclassified.h"

// The active 3DES (tripple des) original or session key and reader data header
static byte_t key[16];
static byte_t rdh[4] = { 0x00,0x00,0x00,0x00 }; 

size_t SCardCLICCTransmit(SCARDHANDLE hCard, byte_t* pbtSnd, size_t szSnd, byte_t* pbtRcv, size_t* pszRcv)
{
  byte_t ctrlbuf[519] = { 0xf7 };
  ctrlbuf[1] = (byte_t)szSnd;
  memcpy(ctrlbuf+5,pbtSnd,szSnd);
  memset(ctrlbuf+5+szSnd,0x00,sizeof(ctrlbuf)-5-szSnd);
  return SCardControl((SCARDHANDLE)hCard,0x00313084,ctrlbuf,sizeof(ctrlbuf),pbtRcv,(DWORD)*pszRcv,(DWORD*)pszRcv);
}

size_t SCardCLICCTransmitS(SCARDHANDLE hCard, byte_t* pbtSnd, size_t szSnd, byte_t* pbtRcv, size_t* pszRcv)
{
  DES_key_schedule ks_a;
  DES_key_schedule ks_b;
  DES_cblock IV = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
  byte_t pbtMAC[BUF_SIZE];
  byte_t pbtSndPlain[BUF_SIZE];
  byte_t pbtSndEnc[BUF_SIZE];
  byte_t pbtRcvPlain[BUF_SIZE];
  byte_t pbtRcvEnc[BUF_SIZE];
  size_t szDataLen;
  size_t szPayloadLen;
  size_t szRcvEncLen;
  size_t szLe;
  size_t result;

  // Make sure this is APDU for the secured mode
  if (pbtSnd[0] != 0x84) {
    return 1;
  }

  // Use the original key only on a "manage session" message (often the first msg only)
  if (pbtSnd[1] == 0x72 && pbtSnd[2] == 0x00) {
  // Test if we used the read or write key
    if (pbtSnd[3] == 0x00) {
      memcpy(key,"\xA5\x96\x15\x4A\xB7\x3C\xD8\x94\x1F\x56\x78\xF5\xE3\xF1\xDF\x73",16);
    } else {
      memcpy(key,"\xB4\x97\xCA\x72\x8A\x80\x66\xDA\xC3\x2E\x5B\x85\x8C\x2C\x70\x28",16);
    }
  }

  DES_set_key((const_DES_cblock*)&key[0],&ks_a);
  DES_set_key((const_DES_cblock*)&key[8],&ks_b);

  // Make a copy of the plapacket
//  memcpy(pbtPlainOut,pucSendData,szPayloadLen);

  // Copy the instruction APDU (excluding the data-length byte)
//  memcpy(pbtSendDataPlain,pucSendData,4);

  // Copy the payload (plaAPDU data bytes)
  szPayloadLen = pbtSnd[4];
  memcpy(pbtSndPlain+10,pbtSnd+5,szPayloadLen);
  
  // Add the "random" data-header
  memcpy(pbtSndPlain+5,rdh,4);
  pbtSndPlain[9] = (byte_t)szPayloadLen;
  
  // Add the padding bytes
  pbtSndPlain[10+szPayloadLen] = 0x80;
  szDataLen = 5+szPayloadLen+1;
  while ((szDataLen%8) != 0) {
    pbtSndPlain[5+szDataLen] = 0x00;
    szDataLen++;
  }

  // Add the MAC using ONLY 8 bytes single DES key
  memset(IV,0x00,sizeof(IV));
  DES_cbc_encrypt(pbtSndPlain+5,pbtMAC,(long)szDataLen,&ks_a,&IV,DES_ENCRYPT);
  
  // Append last encrypted CBC block as MAC to the message data
  memcpy(pbtSndPlain+5+szDataLen,pbtMAC+(szDataLen-8),8);
  szDataLen += 8;

  // Copy the instruction APDU and encrypted data length
  memcpy(pbtSndEnc,pbtSnd,4);
  pbtSndEnc[4] = (byte_t)szDataLen;

  // Encrypt the data
  memset(IV,0x00,sizeof(IV));
  DES_ede2_cbc_encrypt(pbtSndPlain+5,pbtSndEnc+5,(long)szDataLen,&ks_a,&ks_b,&IV,DES_ENCRYPT);

  if (5+szPayloadLen == szSnd-1) {
    pbtSndEnc[szDataLen+5] = pbtSnd[5+szPayloadLen];
    szLe = 1;
  } else {
    szLe = 0;
  }
  
  szRcvEncLen = BUF_SIZE;
  result = SCardCLICCTransmit(hCard,pbtSndEnc,szDataLen+5+szLe,pbtRcvEnc,&szRcvEncLen);

  // After a succesful response, update the key
  if (pbtSnd[1] == 0x72 && pbtSnd[2] == 0x00) {
    memcpy(key,pbtSndPlain+10,8);
    memcpy(key+8,pbtSndPlain+21,8);
    DES_set_key((const_DES_cblock*)&key[0],&ks_a);
    DES_set_key((const_DES_cblock*)&key[8],&ks_b);
  }

  memset(IV,0x00,sizeof(IV));
  DES_ede2_cbc_encrypt(pbtRcvEnc,pbtRcvPlain,(long)szRcvEncLen-2,&ks_a,&ks_b,&IV,DES_DECRYPT);

  // Update response for the reader data header
  rdh[2] = pbtRcvPlain[2] ^ 0xff;
  rdh[3] = pbtRcvPlain[3] ^ 0xff;

  // Determreceived datalength
  szDataLen = pbtRcvPlain[4];

  if (*pszRcv < szDataLen+2) {
    return 1;
  }

  // Copy decrypted response and status bytes
  memcpy(pbtRcv,pbtRcvPlain+5,szDataLen);
  memcpy(pbtRcv+szDataLen,pbtRcvEnc+szRcvEncLen-2,2);

  // Set the length of the received message
  *pszRcv = szDataLen+2;

  return result;
}

bool SendApdu(SCARDHANDLE hCard, byte_t* pbtApdu, size_t szApduLen, byte_t* pbtRcv, size_t* pszRcv) {
  
  *pszRcv = BUF_SIZE;

  if (pbtApdu[0] == 0x80)  {
    SCardCLICCTransmit(hCard,pbtApdu,szApduLen,pbtRcv,pszRcv);
  } else {
    SCardCLICCTransmitS(hCard,pbtApdu,szApduLen,pbtRcv,pszRcv);
  }
  
  if ((pbtRcv[*pszRcv-2] == 0x90) && (pbtRcv[*pszRcv-1] == 0x00)) {
    *pszRcv -= 2;
    return true;
  } else {
    return false;
  }
}
