#include "RakBot.h"

#include "main.h"

int dblSpace;
char buffer[131072];

TCHAR GetChar(TCHAR _char)
{
	if (_char < 32)
		return '.';

	return _char;
}

char *DumpMem(unsigned char *pAddr, int len)
{
	memset(buffer, 0, 16384);

	char temp[256];
	uint8_t fChar;
	int i=0;
	long bytesRead=0;
	TCHAR line[17];
	int nSpaces=3*16+2;
	memset(&line, 0, sizeof(line));
	int dataidx=0;

	if (dblSpace)
		strcat(buffer, "\n\n");

	for (;;)
	{
		// print hex address
		sprintf(temp, "%08X  ", i);
		strcat(buffer, temp);
		
		// print first 8 bytes
		for (int j = 0; j < 0x08; j++)
		{
			fChar = pAddr[dataidx];
			if(dataidx >= len) break;
			dataidx++;

			sprintf(temp, "%02X ", fChar);
			strcat(buffer, temp);

			// add to the ASCII text
			line[bytesRead++] = GetChar(fChar);

			// this took three characters
			nSpaces -= 3;
		}

		// print last 8 bytes - change in the "xx " to " xx" provides
		// the double space in between the first 8 and the last 8 bytes.
		for (int j = 0x08; j < 0x10; j++)
		{
			fChar = pAddr[dataidx];
			if(dataidx >= len) break;
			dataidx++;

			sprintf(temp, " %02X", (unsigned char)fChar);
			strcat(buffer, temp);

			// add to the ASCII text
			line[bytesRead++] = GetChar(fChar);

			// this took three characters
			nSpaces -= 3;
		}

		// fill in any leftover spaces.
		for (int j = 0; j <= nSpaces; j++)
		{
			strcat(buffer, " ");
		}

		// print ASCII text
		sprintf(temp, "%s", line);
		strcat(buffer, temp);

		// quit if the file is done
		if(dataidx >= len) break;

		// new line
		strcat(buffer, "\n");

		if (dblSpace)
			strcat(buffer, "\n");

		// reset everything
		bytesRead=0;
		memset(&line, 0, sizeof(line));
		i += 16;
		//dataidx++;
		nSpaces = 3*16+2;
	}

	return buffer;
}

unsigned char encrBuffer[4092];

unsigned char sampEncrTable[256] =
{
	0x27, 0x69, 0xFD, 0x87, 0x60, 0x7D, 0x83, 0x02, 0xF2, 0x3F, 0x71, 0x99, 0xA3, 0x7C, 0x1B, 0x9D,
	0x76, 0x30, 0x23, 0x25, 0xC5, 0x82, 0x9B, 0xEB, 0x1E, 0xFA, 0x46, 0x4F, 0x98, 0xC9, 0x37, 0x88,
	0x18, 0xA2, 0x68, 0xD6, 0xD7, 0x22, 0xD1, 0x74, 0x7A, 0x79, 0x2E, 0xD2, 0x6D, 0x48, 0x0F, 0xB1,
	0x62, 0x97, 0xBC, 0x8B, 0x59, 0x7F, 0x29, 0xB6, 0xB9, 0x61, 0xBE, 0xC8, 0xC1, 0xC6, 0x40, 0xEF,
	0x11, 0x6A, 0xA5, 0xC7, 0x3A, 0xF4, 0x4C, 0x13, 0x6C, 0x2B, 0x1C, 0x54, 0x56, 0x55, 0x53, 0xA8,
	0xDC, 0x9C, 0x9A, 0x16, 0xDD, 0xB0, 0xF5, 0x2D, 0xFF, 0xDE, 0x8A, 0x90, 0xFC, 0x95, 0xEC, 0x31,
	0x85, 0xC2, 0x01, 0x06, 0xDB, 0x28, 0xD8, 0xEA, 0xA0, 0xDA, 0x10, 0x0E, 0xF0, 0x2A, 0x6B, 0x21,
	0xF1, 0x86, 0xFB, 0x65, 0xE1, 0x6F, 0xF6, 0x26, 0x33, 0x39, 0xAE, 0xBF, 0xD4, 0xE4, 0xE9, 0x44,
	0x75, 0x3D, 0x63, 0xBD, 0xC0, 0x7B, 0x9E, 0xA6, 0x5C, 0x1F, 0xB2, 0xA4, 0xC4, 0x8D, 0xB3, 0xFE,
	0x8F, 0x19, 0x8C, 0x4D, 0x5E, 0x34, 0xCC, 0xF9, 0xB5, 0xF3, 0xF8, 0xA1, 0x50, 0x04, 0x93, 0x73,
	0xE0, 0xBA, 0xCB, 0x45, 0x35, 0x1A, 0x49, 0x47, 0x6E, 0x2F, 0x51, 0x12, 0xE2, 0x4A, 0x72, 0x05,
	0x66, 0x70, 0xB8, 0xCD, 0x00, 0xE5, 0xBB, 0x24, 0x58, 0xEE, 0xB4, 0x80, 0x81, 0x36, 0xA9, 0x67,
	0x5A, 0x4B, 0xE8, 0xCA, 0xCF, 0x9F, 0xE3, 0xAC, 0xAA, 0x14, 0x5B, 0x5F, 0x0A, 0x3B, 0x77, 0x92,
	0x09, 0x15, 0x4E, 0x94, 0xAD, 0x17, 0x64, 0x52, 0xD3, 0x38, 0x43, 0x0D, 0x0C, 0x07, 0x3C, 0x1D,
	0xAF, 0xED, 0xE7, 0x08, 0xB7, 0x03, 0xE6, 0x8E, 0xAB, 0x91, 0x89, 0x3E, 0x2C, 0x96, 0x42, 0xD9,
	0x78, 0xDF, 0xD0, 0x57, 0x5D, 0x84, 0x41, 0x7E, 0xCE, 0xF7, 0x32, 0xC3, 0xD5, 0x20, 0x0B, 0xA7
};

void kyretardizeDatagram(unsigned char *buf, int len, int port, int unk)
{
    unsigned char bChecksum = 0;
    for(int i = 0; i < len; i++)
    {
        unsigned char bData = buf[i];
        bChecksum ^= bData & 0xAA;
    }
    encrBuffer[0] = bChecksum;

    unsigned char *buf_nocrc = &encrBuffer[1];
    memcpy(buf_nocrc, buf, len);

    for(int i = 0; i < len; i++)
    {
		buf_nocrc[i] = sampEncrTable[buf_nocrc[i]];
		if ( unk )
			buf_nocrc[i] ^= (unsigned __int8)(port) ^ 0xCC;
		unk ^= 1u;
    }
}