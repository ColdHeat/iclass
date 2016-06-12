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

#include <stdio.h>
#include <openssl/des.h>
#include <windows.h>
#include <winscard.h>
#include "iclassified.h"
#include "stdbyte.h"

#define PCSC_MAX_DEVICES 16
byte_t pbtRx[BUF_SIZE];
size_t szRxLen;

typedef struct struct_iclass_mem {
  byte_t app_limit;
  byte_t otp[2];
  byte_t write_lock;
  byte_t chip_conf;
  byte_t mem_conf;
  byte_t eas;
  byte_t fuses;
} iclass_mem;

void print_bytes(const byte_t* bt, size_t len) {
  size_t count;
  for (count=0; count<len; count++) {
    printf("%02x",bt[count]);
  }
  printf("\n");
}

void read_card(SCARDHANDLE hCard){
  int i;
  byte_t pbtApduRead[] = { 0x84,0xB0,0x00,0x00,0x00,0x08 };
  for (i = 0; i <= 0x1F; ++i)
  {
    if (!SendApdu(hCard,pbtApduRead,sizeof(pbtApduRead),pbtRx,&szRxLen)) {
      printf("Error: Could not read block %02x\n", i);
    }
    print_bytes(pbtRx,szRxLen);
    pbtApduRead[3]++;
  }
}

void read_data(SCARDHANDLE hCard, char* block_str){
  int block = strtoul(block_str, NULL, 0);
  byte_t read_block[] = { 0x84,0xB0,0x00,block,0x00,0x08 };

  if (!SendApdu(hCard,read_block,sizeof(read_block),pbtRx,&szRxLen)) {
    printf("Error: Could not read block %d\n", block);
    return;
  }

  print_bytes(pbtRx,szRxLen);
  return;
}

void write_data(SCARDHANDLE hCard, char* block_str, char* data){
  int block = strtoul(block_str, NULL, 0);

  byte_t val[8];
  char* pos = data;
  size_t count = 0;

  // Convert hexstring to byte array
  for(count = 0; count < sizeof(val)/sizeof(val[0]); count++) {
      sscanf(pos, "%2hhx", &val[count]);
      pos += 2;
  }

  byte_t pbtApduWrite[]   = { 0x84,0xD6,0x00,block,0x08,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};

  count = 0;
  for(count = 0; count < 8; count++){
    pbtApduWrite[5+count] = val[count];
  }

  if (!SendApdu(hCard, pbtApduWrite, sizeof(pbtApduWrite), pbtRx, &szRxLen)) {
    printf("Error: Write failed\n");
    print_bytes(pbtRx,szRxLen);
    return;
  }

  printf("SUCCESS\n");
  return;
}

void print_help(){
  printf("iclass\n");
  printf("\n");
  printf("Usage:\n");
  printf("  iclass read <block>\n");
  printf("  iclass write <block> <data>\n");
  printf("\n");
  printf("Examples:\n");
  printf("  To read the entire card:\n");
  printf("    iclass read\n");
  printf("\n");
  printf("  To read a specific block:\n");
  printf("    iclass read 6\n");
  printf("\n");
  printf("  To write to a specific block:\n");
  printf("    iclass write 6 4141414141414141\n");
  return;
}

int main(int argc, char* argv[]) {

  DWORD rc;
  DWORD dwActiveProtocol;
  SCARDCONTEXT hContext;
  SCARDHANDLE hCard;

  size_t szPos = 0;
  char acDeviceNames[256 + 64 * PCSC_MAX_DEVICES];
  size_t szDeviceNamesLen = sizeof (acDeviceNames);
  DWORD dwDeviceNamesLen;
  bool bFoundReader = false;

  bool read = false;
  bool write = false;
  char* block = NULL;
  char* data = NULL;

  // Enough data
  if (argc < 2) {
    print_help();
    exit(1);
  } else if( strcmp(argv[1], "read") != 0 && strcmp(argv[1], "write") != 0){
    print_help();
    exit(1);
  }

  // Which command
  read = strcmp(argv[1], "read") == 0;
  if (!read) {
    write = strcmp(argv[1], "write") == 0;
  }

  // Which block if there is a block
  if (argc > 2){
    block = argv[2];

    if (write && (argc == 4)){
      data = argv[3];
      if (strlen(data) != 16){
        printf("FAIL: Data written to a block must be 8 bytes\n");
        exit(1);
      }
    } else if (write && argc <= 4){
      printf("FAIL: Missing data to write\n");
      exit(1);
    }
  }

  ////////////////////////////////////////////////////////////////////
  // Initialize Card Reader
  ////////////////////////////////////////////////////////////////////

  rc = SCardEstablishContext(SCARD_SCOPE_USER,NULL,NULL,&hContext);
  if(rc != SCARD_S_SUCCESS) {
    printf("FAIL: Could not establish smartcard memory context\n");
    return 1;
  }

  // Retrieve the string array of all available pcsc readers
  dwDeviceNamesLen = (DWORD)szDeviceNamesLen;
  if (SCardListReadersA(hContext, NULL, acDeviceNames, &dwDeviceNamesLen) != SCARD_S_SUCCESS) {
    printf("FAIL: Could not connect retrieve smartcard reader list\n");
    return false;
  }

  while (acDeviceNames[szPos] != '\0')
  {
    rc = SCardConnectA(hContext,acDeviceNames+szPos,SCARD_SHARE_SHARED,SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1, &hCard, &dwActiveProtocol);
    if(rc != SCARD_S_SUCCESS) {
      printf("Failed\n");
    } else {
      byte_t pbtApduSecured[] = { 0x84,0x72,0x00,0x01,0x08,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 };
      if (SendApdu(hCard,pbtApduSecured,sizeof(pbtApduSecured),pbtRx,&szRxLen)){
        bFoundReader = true;
        break;
      }
      printf("Failed\n");
      SCardDisconnect(hCard,SCARD_LEAVE_CARD);
    }
    // Find next device name position
    while (acDeviceNames[szPos++] != '\0');
  }

  if (!bFoundReader) {
    printf("FAIL: Could not find OMNIKEY Reader\n");
    return 1;
  }

  ////////////////////////////////////////////////////////////////////
  // INITIALIZE SECURE MODE
  ////////////////////////////////////////////////////////////////////

  byte_t pbtApduSecured[] = { 0x84,0x72,0x00,0x01,0x08,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 };
  if (!SendApdu(hCard, pbtApduSecured, sizeof(pbtApduSecured), pbtRx, &szRxLen)) {
    printf("FAIL: Could not start the OMNIKEY secure mode\n");
    return 1;
  }

  ////////////////////////////////////////////////////////////////////
  // AUTHENTICATE
  ////////////////////////////////////////////////////////////////////

  byte_t pbtApduAuthKey[] = { 0x84,0x88,0x00,0x21,0x00 };
  if (!SendApdu(hCard,pbtApduAuthKey,sizeof(pbtApduAuthKey),pbtRx,&szRxLen)) {
    printf("Error: Authentication failed\n");
    return 1;
  }

  ////////////////////////////////////////////////////////////////////
  // INITIALIZE SECURE MODE
  ////////////////////////////////////////////////////////////////////

  byte_t pbtApduSecured2[] = { 0x84,0x72,0x00,0x01,0x08,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 };
  if (!SendApdu(hCard, pbtApduSecured2, sizeof(pbtApduSecured2), pbtRx, &szRxLen)) {
    printf("FAIL: Could not start the OMNIKEY secure mode\n");
    return 1;
  }


  if (read && block){
    read_data(hCard, block);
  } else if (read && argc == 2){
    read_card(hCard);
  }  else if (write){
    write_data(hCard, block, data);
  }

  return 0;
}
