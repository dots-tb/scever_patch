#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "elf.h"

int main(int argc, char **argv){
	
	if(argc < 3){
		printf ("\nusage:  scever_patch module<.elf/.prx> <500>\n");
		return -1;
	}
	uint16_t fw_num = strtol (argv[2],NULL,16); 
	printf("Setting to fw to: %x\n", fw_num);
	printf("Opening: %s\n", argv[1]);
	FILE *fp = fopen(argv[1],"r+b");
	if(!fp)
		return 0;
	Elf64_Ehdr ehdr;
	fread(&ehdr,sizeof(ehdr),1,fp);
	
	printf("Checking if ELF...\n");
	if(memcmp(&ehdr.e_ident, (char[8]){0x7F, 0x45, 0x4C, 0x46, 0x02, 0x01, 0x01, 0x09},8))
		return 0;
	
	Elf64_Phdr *phdrs = malloc(ehdr.e_phentsize * ehdr.e_phnum);
	fseek(fp, ehdr.e_phoff, SEEK_SET);
	fread(phdrs,ehdr.e_phentsize * ehdr.e_phnum,1,fp);
	for(int i = 0; i < ehdr.e_phnum; ++i) {
		if(phdrs[i].p_type == 0x61000002||phdrs[i].p_type == 0x61000001) {
			printf("sce_process_param patch\n");
			char *sce_process_param = malloc(phdrs[i].p_filesz);
			fseek(fp, phdrs[i].p_offset, SEEK_SET);
			fread(sce_process_param,phdrs[i].p_filesz,1,fp);
			uint16_t *req_ver = sce_process_param + 0x12;
			printf("Original version: %x\n", *req_ver); 
			*req_ver = fw_num;
			printf("Writing version: %x\n", *req_ver);
			fseek(fp, phdrs[i].p_offset, SEEK_SET);
			fwrite(sce_process_param,phdrs[i].p_filesz,1,fp);
			free(sce_process_param);			
		}
		if(phdrs[i].p_type == 0x6fffff01) {
			printf("sce_version patch\n");
			char *sce_version = malloc(phdrs[i].p_filesz);
			fseek(fp, phdrs[i].p_offset, SEEK_SET);
			fread(sce_version, phdrs[i].p_filesz,1,fp);
			
			size_t offset = 0;
			while(offset < phdrs[i].p_filesz) {
				int sz = *(sce_version + offset);
				char name[128] ={};
				memcpy(name, sce_version + offset + 1, sz - 0x4);
				printf("Library name: %s\n", name, sz);
				uint16_t *req_ver = sce_version + offset + 1 + sz - 0x4;
				printf("Original version: %x\n", __builtin_bswap16(*req_ver));
				*req_ver = __builtin_bswap16(fw_num);
				printf("Setting version: %x\n", __builtin_bswap16(*req_ver)); 
				offset += sz + 1;
			}
			
					
			printf("Writing sce_version...\n");
			fseek(fp, phdrs[i].p_offset, SEEK_SET);
			fwrite(sce_version,phdrs[i].p_filesz,1,fp);
			free(sce_version);			
		}
		
	}

	free(phdrs);
	fclose(fp);
	
	return 0;
}