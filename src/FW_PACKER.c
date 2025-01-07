/*
 ============================================================================
 Name        : STM32_BL_UPDATER.c
 Author      : Ing. Petr Pavlata
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
//#include "stm32f_bl_opcodes.h"
//#include "stm32_mcu.h"
//#include "tty.h"
#include "fw_info.h"

#define VERSION 		"1.0"
#define BLOCK_SIZE		512

void PrintHelp(void)
{
	printf("Usage: \n");
	printf("-help        - Display this message\n");
	printf("-parts       - number of images\n");
	printf("-ifn      	 - input binary file\n");
	printf("-ipln      	 - target padded part length\n");
	printf("-of     	 - output firmware\n");
	printf("-string      - compatible string\n");
	printf("-CRC         - crc algorithm\n");
	printf("-cs          - compatibility string");
	printf("-fwmaj       - major_version");
	printf("-fwmin       - minor_version");

	printf("-pMajn       - Part major");
	printf("-pMinn       - Part minor");
	printf("-pBuildn     - Part build");
	printf("-pIDstrn     - Part Id string");
	printf("-pLoadAddr   - Part Load address");
	printf("-pTypen   	 - Part Image type { MCUFW FPGA CONFIG }");
	printf("-pPlatform   - Part Platform   { STM32 HT32 XILS6 XILZ7 XILZMP XILA7 }");
	printf("-pCRCn   	 - Part CRC alg    { STM32_CRC }");
}


#define IMAGE_MAGIC 	0x1a5e82b5
#define MCUFW 			0x01
#define FPGA			0x02
#define CONFIG 			0x03

#define STM32 			0x01
#define HT32 			0x02
#define XILS6 			0x03
#define XILZ7 			0x04
#define XILZMP			0x05
#define GWN1			0x06
#define XILA7			0x07

#define CRC_STM32		0x01
#define CRC_GWN1	   	0x02

typedef enum {
	Crypt_NONE, Crypt_AES_128, Crypt_AES_256
}Crypt_type_t;

Crypt_type_t CryptToType(char * typestring)
{
	if(0 == strcmp(typestring,"CRYPT_NONE"))     return Crypt_NONE;
	if(0 == strcmp(typestring,"CRYPT_AES128")) 	 return Crypt_AES_128;
	if(0 == strcmp(typestring,"CRYPT_AES256"))   return Crypt_AES_256;
	return Crypt_NONE;
}


uint8_t TypeToNum(char * typestring)
{
	if(0 == strcmp(typestring,"MCUFW"))  return MCUFW;
	if(0 == strcmp(typestring,"FPGA")) 	 return FPGA;
	if(0 == strcmp(typestring,"CONFIG")) return CONFIG;
	return 0;
}

uint8_t PlatformToNum(char * typestring)
{
	if(0 == strcmp(typestring,"STM32"))  	return STM32;
	if(0 == strcmp(typestring,"HT32")) 	 	return HT32;
	if(0 == strcmp(typestring,"XILS6")) 	return XILS6;
	if(0 == strcmp(typestring,"XILZ7")) 	return XILZ7;
	if(0 == strcmp(typestring,"XILZMP")) 	return XILZMP;
	if(0 == strcmp(typestring,"GWN1")) 	    return GWN1;
	if(0 == strcmp(typestring,"XILA7")) 	return XILA7;
	return 0;
}

uint8_t CRCTypeToNum(char * typestring)
{
	if(0 == strcmp(typestring,"CRC_STM32"))  	return CRC_STM32;
	if(0 == strcmp(typestring,"CRC_GWN1"))  	return CRC_GWN1;
	return 0;
}

char * GetParemeterByName(int argc, char *argv[], const char * name, char ** value)
{
	uint16_t i;
	for(i = 0; i< argc; i++)
	{
		if(0 == strcmp(argv[i],name))
		{
			if(argc > i && value != NULL)  *value = argv[i+1];
			return argv[i];
		}
	}
	return NULL;
}
#define MAX_PARTS 8

typedef struct __attribute__((packed))
{
	uint32_t Header_magic;
	uint32_t Platform;
	char 	 ID_String[32];
	uint8_t	 FW_Ver_Major;
	uint8_t	 FW_Ver_Minor;
	uint16_t HeaderSize;
	uint32_t ImageOffsets[8];
	uint32_t ImageSizes[8];
	uint8_t  ImageTypes[8];
	uint64_t LoadAddreses[8];
	uint32_t FW_Security_Mode;
	uint64_t FW_Security_KeyHash;
	uint32_t CRC;
}Firmware_Image_Header_t;


typedef struct __attribute__((packed))
{
	char 	 ID_String[32];
	uint8_t	 FW_Ver_Major;
	uint8_t	 FW_Ver_Minor;
	uint8_t	 Image_type;
	uint8_t	 Platform;			//36
	uint16_t BuildNumber;
	uint8_t	 resv0[2];
	uint32_t LoadVectorHI;
	uint32_t LoadVectorLO;
	uint8_t	 resv1[12];
	uint32_t CRC;				//64
}Firmware_Trailer_t;

#define PADDING_SIZE sizeof(Firmware_Trailer_t)


uint32_t Crc32Fast(uint32_t Crc, uint32_t Data)
{
  static const uint32_t CrcTable[16] = { // Nibble lookup table for 0x04C11DB7 polynomial
    0x00000000,0x04C11DB7,0x09823B6E,0x0D4326D9,0x130476DC,0x17C56B6B,0x1A864DB2,0x1E475005,
    0x2608EDB8,0x22C9F00F,0x2F8AD6D6,0x2B4BCB61,0x350C9B64,0x31CD86D3,0x3C8EA00A,0x384FBDBD };

  Crc = Crc ^ Data; // Apply all 32-bits

  // Process 32-bits, 4 at a time, or 8 rounds

  Crc = (Crc << 4) ^ CrcTable[Crc >> 28]; // Assumes 32-bit reg, masking index to 4-bits
  Crc = (Crc << 4) ^ CrcTable[Crc >> 28]; //  0x04C11DB7 Polynomial used in STM32
  Crc = (Crc << 4) ^ CrcTable[Crc >> 28];
  Crc = (Crc << 4) ^ CrcTable[Crc >> 28];
  Crc = (Crc << 4) ^ CrcTable[Crc >> 28];
  Crc = (Crc << 4) ^ CrcTable[Crc >> 28];
  Crc = (Crc << 4) ^ CrcTable[Crc >> 28];
  Crc = (Crc << 4) ^ CrcTable[Crc >> 28];

  return(Crc);
}

//****************************************************************************
uint32_t Crc32FastBlock(uint32_t Crc, uint32_t Size, uint32_t *Data)
{

  while(Size--)
    Crc = Crc32Fast(Crc, *Data++); // 32-bit at a time

  return(Crc);
}

uint32_t crc_stm32(void * data, uint32_t length)
{
	return Crc32FastBlock(0xffffffff, length/4, (uint32_t *) data);
}

uint32_t compute_crc(void  * buffer,uint32_t length, uint8_t crc_alg )
{
	switch(crc_alg)
	{
		case STM32:
			printf("#FWPKG: calculatig CRC, alg=STM32, offset %p length: %u\n",buffer,length);
			return crc_stm32((uint8_t *)buffer,  length -4);
			break;
	}
	return 0;
}

#define MAX_VERSION_FILE_SIZE 	(128 * 1024)
#define MAX_VERSION_PARAM_SIZE 	64

int version_file_param_search(char * inputdata, char * param_name, char * output_buffer )
{
	char * startstring;
	char * endstring;
	const char valid[] = "01234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
	const char delim[] = " \"\n\r\0\t";
	//find parameter define
	startstring = strstr(inputdata,param_name);
	if(startstring == NULL)
	{
		printf("#FWPKG: Version parameter: %s not found in file\n",param_name);
		return 0;
	}
	//first white
	startstring += strcspn(startstring,delim);
	//find beginning of parameter value
	startstring += strcspn(startstring,valid);

	endstring  = startstring + strcspn(startstring,delim);
	if( ( (endstring - startstring) > 0) && ((endstring - startstring) < MAX_VERSION_PARAM_SIZE))
	{
		memset(output_buffer,0,MAX_VERSION_PARAM_SIZE );
		memcpy(output_buffer,startstring,(endstring - startstring));
		printf("#FWPKG: Version file parameter %s == %s\n", param_name,output_buffer);
		return (endstring - startstring);
	}
	return 0;
}

int IncrementBuild(char * data, char length )
{
	char * buffer = malloc(MAX_VERSION_FILE_SIZE);
	char * startstring = strstr(data,"BUILDNUMBER");
	char * endstring;
	const char valid[] = "01234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
	const char delim[] = " \"\n\r\0\t";
	char buff[64];
	memset(buff,0,64 );
	if(startstring == NULL)
	{
		printf("#FWPKG: Version parameter BUILDNUMBER not found in file\n");
	}
	//first white
	startstring += strcspn(startstring,delim);
	//find beginning of parameter value
	startstring += strcspn(startstring,valid);
	endstring  = startstring + strcspn(startstring,delim);
	if( ( (endstring - startstring) > 0) && ((endstring - startstring) < MAX_VERSION_PARAM_SIZE))
	{
		memcpy(buff,startstring,(endstring - startstring));
		int build = atoi(buff);
		build++;
		int off = startstring-data;
		printf("#FWPKG: Version file set current build to %u %u\n", build,off);
		memcpy(buffer,data,off);
		int add = sprintf(buffer+off, "%u",build);
		add += sprintf(buffer+off+add,"%s",endstring);
	//	printf("%s",  buffer);

		memcpy(data,buffer,off+ add);
		return (off) + add;
	}
	return 0;
}

int main(int argc, char *argv[]) {
	setvbuf(stdout, NULL, _IONBF, 0);
	char  *  	image_buffer = NULL;
	int			image_length = BLOCK_SIZE;
	char * 		param;
	char * 		infilename[MAX_PARTS];
	int			target_part_lengths[MAX_PARTS];
	int			target_image_lengths[MAX_PARTS];
	char * 		outputfilename = NULL;
	FILE * 		Fi[MAX_PARTS];
	FILE * 		Fo;
	int			num_of_parts = 0,i =0;
	char *		Id_String;
	int	 		Major_ver = 0;
	int			Minor_Ver = 0;
	char *		VersionFile = malloc(MAX_VERSION_FILE_SIZE);
	char dyn_infile[256];
	Firmware_Image_Header_t * header;
	if(argc <= 0) return -1;
	printf("#FWPKG: FIRMWARE PACKER TOOL %s - %s \n",VERSION, __DATE__);

	if(GetParemeterByName	(argc, argv,  	"-help", 	NULL)) 				{ PrintHelp(); return 0; }
	if(GetParemeterByName	(argc, argv,  	"-buildinc",NULL))
	{
		sprintf(dyn_infile,"-pVersionFile");

		if(GetParemeterByName	(argc, argv,  	dyn_infile,	 &param))
		{
			FILE * Version = fopen( param, "rb" );
			if(Version == NULL)
			{
				printf("#FWPKG: Version file cannot be opened!\n");
				return -1;
			}
			char strbuffbuffer[MAX_VERSION_PARAM_SIZE];
			memset(VersionFile ,0,MAX_VERSION_FILE_SIZE);
			int readed = fread(VersionFile ,1,MAX_VERSION_FILE_SIZE,Version);
			fclose(Version);

			int edited = IncrementBuild(VersionFile,readed);
			if(edited > 0){
				Version = fopen( param, "wb" );
				if(Version == NULL)
				{
					printf("#FWPKG: Version file cannot be opened for write!\n");
					return -1;
				}
				fwrite(VersionFile ,1,edited,Version);
				fclose(Version);
			}
		}
		return 0;
	}

	if(GetParemeterByName	(argc, argv,  	"-imgverfile",	 &param)) {
		printf("#FWPKG: Global Version file: %s\n",param);
		printf("#FWPKG: Block size: %u Image padding % u\n",BLOCK_SIZE, PADDING_SIZE);
		FILE * Version = fopen( param, "rb" );
		if(Version == NULL)
		{
			printf("#FWPKG: Version file cannot be opened!\n");
			return -1;
		}
		char strbuffbuffer[MAX_VERSION_PARAM_SIZE];
		memset(VersionFile ,0,MAX_VERSION_FILE_SIZE);
		int readed = fread(VersionFile ,1,MAX_VERSION_FILE_SIZE,Version);
		version_file_param_search(VersionFile,"FW_MAJOR", strbuffbuffer);
		Major_ver = atoi(strbuffbuffer);
		version_file_param_search(VersionFile,"FW_MINOR", strbuffbuffer);
		Minor_Ver = atoi(strbuffbuffer);
		fclose(Version);
	}
	else
	{
		if(GetParemeterByName	(argc, argv,  	"-fwmaj",	 &param)) 			{ Major_ver = atoi(param); }
		if(GetParemeterByName	(argc, argv,  	"-fwmin",   &param)) 			{ Minor_Ver = atoi(param); }
	}



	if(!(GetParemeterByName	(argc, argv,  	"-of",  	&outputfilename)))  { printf("#FWPKG: missing output filename!\n"); return -1; }
	GetParemeterByName		(argc, argv,  	"-cs",  	&Id_String);


	for(i = 0;i < MAX_PARTS; i++)
	{
		sprintf(dyn_infile,"-if%u",i+1);
		if(GetParemeterByName(argc, argv, dyn_infile, &infilename[i]))
		{
			num_of_parts++;
		}
		else break;
		sprintf(dyn_infile,"-ipl%u",i+1);
		target_part_lengths[i] = 0;
		int filesize = 0;
		if(GetParemeterByName(argc, argv, dyn_infile, &param))
		{
			target_part_lengths[i] = atoi(param);
	        //Read File size
			Fi[i] = fopen( infilename[i], "rb" );
			fseek(Fi[i], 0, SEEK_END);
			filesize = ftell((Fi[i]));
			fclose(Fi[i]);

			if(target_part_lengths[i] == 0)
			{
				target_image_lengths[i] = filesize;
				target_part_lengths[i] = filesize;
				if(target_part_lengths[i] % PADDING_SIZE) {target_part_lengths[i]= ((target_part_lengths[i] % PADDING_SIZE) + 1) * PADDING_SIZE;}
				target_part_lengths[i] += PADDING_SIZE; // one block for trailer
				printf("#FWPKG: Using binary file based length: %u B Section: filesize  %u \n",target_part_lengths[i]);
			}
		}

		//PADDING_SIZE
		/*
		if(target_part_lengths[i] % BLOCK_SIZE)
		{
			target_part_lengths[i] = ((target_part_lengths[i] / BLOCK_SIZE) + 1) * BLOCK_SIZE;
			printf("#FWPKG: Padding image to: %u\n",target_part_lengths[i]);
		}
*/
		image_length += target_part_lengths[i];
	}

	printf("#FWPKG: Output filename: %s, Output Image size: %u \n",outputfilename, image_length);
	//prepare buffer
	image_buffer = malloc(image_length);
	memset(image_buffer,0,image_length);
	header = (Firmware_Image_Header_t*)image_buffer;

	//Prehled input casti
	for(i = 0;i < num_of_parts; i++)
	{
		printf("#FWPKG: Part %u of %u  => filename: %s padded size: %u \n",i+1,num_of_parts,infilename[i], target_part_lengths[i] );
		Fi[i] = fopen( infilename[i], "rb" );
		if(Fi[i] ==  NULL)
		{
			printf("#FWPKG: Cannot open part %u == file: %s \n",i,infilename[i]);
			return -1;
		}

	}

	//nulty blok je hlavicka
	uint32_t image_pointer = BLOCK_SIZE;
	//copy data into image
	for(i = 0;i < num_of_parts; i++)
	{
		printf("#FWPKG: Adding binary image file to offset 0x%08x = file: %s\n",image_pointer,infilename[i]);
		int readed = fread(image_buffer+image_pointer,1,target_part_lengths[i],Fi[i]);
		if(readed > (target_part_lengths[i]) - PADDING_SIZE)
		{
			printf("#FWPKG: Padded section is too small for data %u == file: %s , section size: %u, image size %u\n",i,infilename[i],target_part_lengths[i],readed);
			return -1;
		}
		header->ImageOffsets[i] = image_pointer;
		header->ImageSizes[i]	= target_part_lengths[i];

		//
		uint32_t tooff = image_pointer + target_part_lengths[i] - PADDING_SIZE;
		Firmware_Trailer_t * Trailer = (Firmware_Trailer_t *)(image_buffer+tooff );

		//Version size
		sprintf(dyn_infile,"-pVersionFile%u",i+1);
		if(GetParemeterByName	(argc, argv,  	dyn_infile,	 &param))
		{
			printf("#FWPKG: Part: %u Version file: %s\n",i,param);
			FILE * Version = fopen( param, "rb" );
			if(Version == NULL)
			{
				printf("#FWPKG: Version file cannot be opened!\n");
				return -1;
			}
			char strbuffbuffer[MAX_VERSION_PARAM_SIZE];
			memset(VersionFile ,0,MAX_VERSION_FILE_SIZE);
			int readed = fread(VersionFile ,1,MAX_VERSION_FILE_SIZE,Version);

			version_file_param_search(VersionFile,"FW_STRING",strbuffbuffer);
			sprintf(Trailer->ID_String, "%s",strbuffbuffer);
			version_file_param_search(VersionFile,"FW_MAJOR", strbuffbuffer);
			Trailer->FW_Ver_Major = atoi(strbuffbuffer);
			version_file_param_search(VersionFile,"FW_MINOR", strbuffbuffer);
			Trailer->FW_Ver_Minor = atoi(strbuffbuffer);
			version_file_param_search(VersionFile,"BUILDNUMBER", strbuffbuffer);
			Trailer->BuildNumber = atoi(strbuffbuffer);
			fclose(Version);
		}
		else
		{
			sprintf(dyn_infile,"-pMaj%u",i+1);
			if(GetParemeterByName	(argc, argv,  	dyn_infile,	 &param)) 			{ Trailer->FW_Ver_Major = atoi(param); 				}
			sprintf(dyn_infile,"-pMin%u",i+1);
			if(GetParemeterByName	(argc, argv,  	dyn_infile,	 &param)) 			{ Trailer->FW_Ver_Minor = atoi(param); 				}
			sprintf(dyn_infile,"-pBuild%u",i+1);
			if(GetParemeterByName	(argc, argv,  	dyn_infile,	 &param)) 			{ Trailer->BuildNumber  = atoi(param); 				}
			sprintf(dyn_infile,"-pIDstr%u",i+1);
			if(GetParemeterByName	(argc, argv,  	dyn_infile,	 &param)) 			{ sprintf(Trailer->ID_String, "%s", param); 		}
		}
		sprintf(dyn_infile,"-pLoadAddr%u",i+1);
		if(GetParemeterByName	(argc, argv,  	dyn_infile,	 &param)) 			{ header->LoadAddreses[i] 	= strtol(param, NULL, 16); 	}
		Trailer->LoadVectorLO = header->LoadAddreses[i];
		sprintf(dyn_infile,"-pType%u",i+1);
		if(GetParemeterByName	(argc, argv,  	dyn_infile,	 &param)) 			{ Trailer->Image_type 	= TypeToNum(param);	 		}
		sprintf(dyn_infile,"-pPlatform%u",i+1);
		if(GetParemeterByName	(argc, argv,  	dyn_infile,	 &param)) 			{ Trailer->Platform 	= PlatformToNum(param);	 	}



		//part CRC compute
		sprintf(dyn_infile,"-pCRC%u",i+1);
		uint8_t crc_alg = 0;
		if(GetParemeterByName	(argc, argv,  	dyn_infile,	 &param)) 			{ crc_alg 	= CRCTypeToNum(param);	 	}
		if(crc_alg)
		{
			Trailer->CRC 	= compute_crc(image_buffer+image_pointer,target_part_lengths[i], crc_alg );
			printf("#FWPKG: Part CRC: 0x%08x\n",Trailer->CRC);
		}

		printf("#FWPKG: Part offset: 0x%08x Trailer offset: 0x%08x Part End: 0x%08x \n",image_pointer, tooff ,image_pointer + target_part_lengths[i]);
		// Encryption
		sprintf(dyn_infile,"-pCrypt%u",i+1);
		Crypt_type_t Crypttype = Crypt_NONE;

		if(GetParemeterByName	(argc, argv,  	dyn_infile,	 &param)) 			{ Crypttype 	= CryptToType(param);	 		}
		if(Crypttype != Crypt_NONE)
		{
			char * keyfilename;
			sprintf(dyn_infile,"-keyfile%u",i+1);
			if(GetParemeterByName(argc, argv, dyn_infile,&keyfilename))
			{
				num_of_parts++;
			}

			printf("#FWPKG: Required image encryption, keyfile: %s \n",keyfilename);
		}
		//

		image_pointer += target_part_lengths[i];

	}
	//prepare header
	header->Header_magic = IMAGE_MAGIC;
	header->FW_Ver_Major = Major_ver;
	header->FW_Ver_Minor = Minor_Ver;
	sprintf(header->ID_String,"%s",Id_String);
	//save output
	Fo = fopen( outputfilename, "wb" );
	int wr;
	wr = fwrite(image_buffer,1,image_length,Fo);
	fclose(Fo);
	printf("#FWPKG: wrote %u of %u B\n",wr,image_length);

	sprintf(outputfilename,"%s_%u.%u.fwimg",Id_String,Major_ver,Minor_Ver);
	Fo = fopen( outputfilename, "wb" );
	wr = fwrite(image_buffer,1,image_length,Fo);
	fclose(Fo);
	free(image_buffer);
	free(VersionFile);

	return 0;

	return EXIT_SUCCESS;
}
