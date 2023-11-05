#include <stdio.h>
#include <elf.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <signal.h>
#include <setjmp.h>         

int page_faults = 0;
int page_allocations = 0;
int first_successful=1;
size_t internal_fragmentation = 0;
int reqd_segment = 0;
typedef int (*function)();
void load_and_run_elf(char** exe, int i);
void loader_cleanup();
static jmp_buf jump_buffer;

Elf32_Ehdr *ehdr;
Elf32_Phdr *phdr;
int fd;
void segmentation_fault_handler(int signal) {
    printf("Segmentation fault detected. Performing some action...\n");
    page_faults++;
    // You can perform your desired action here
    siglongjmp(jump_buffer, 1);
}
void loader_cleanup() {
    if (phdr) {
        free(phdr);
        phdr = NULL;
    }
    if (ehdr) {        
        free(ehdr);
        ehdr = NULL;
    }
    if (fd != -1) {
        close(fd);
        fd = -1;
    }
}

void load_and_run_elf(char** exe,int i) {
    // printf("Loading and running ELF executable: %s\n", exe[1]); 
    fd = open(exe[1], O_RDONLY);
    // printf("File descriptor: %d\n", fd);
    if (fd == -1) {
        printf("Error opening file");
        loader_cleanup();
        return;
    }

    ehdr = (Elf32_Ehdr*)malloc(sizeof(Elf32_Ehdr));
    if (!ehdr) {
        printf("Memory allocation failed");
        loader_cleanup();
        close(fd);
        return;
    }

    if (read(fd, ehdr, sizeof(Elf32_Ehdr)) != sizeof(Elf32_Ehdr)) {
        printf("Error reading ELF header");
        loader_cleanup();
        return;
    }

    phdr = (Elf32_Phdr*)malloc(sizeof(Elf32_Phdr) * ehdr->e_phnum);
    if (!phdr) {
        printf("Memory allocation failed");
        loader_cleanup();
        return;
    }
    // signal(SIGSEGV, segmentation_fault_handler);

    if (lseek(fd, ehdr->e_phoff, SEEK_SET) == -1 ||
        read(fd, phdr, sizeof(Elf32_Phdr) * ehdr->e_phnum) !=
        sizeof(Elf32_Phdr) * ehdr->e_phnum) {
        printf("Error reading program header table");
        loader_cleanup();
        return;
    }
    void *load_addr = NULL;
    printf("Number of program headers: %d\n", ehdr->e_phnum);
    // for (int i = 0; i < ehdr->e_phnum; i++) {
        // if (phdr[i].p_type == PT_LOAD) {
            // printf("Loading segment %d to memory\n", i);
    size_t page_size = getpagesize();
    size_t required_pages =phdr[i].p_memsz/page_size+1; // Adjust this value as needed
    size_t allocation_size = page_size * required_pages;
    
            load_addr = mmap(
                (void*)phdr[i].p_vaddr,
                phdr[i].p_memsz,
                PROT_READ | PROT_WRITE | PROT_EXEC,
                MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED,
                fd,
                phdr[i].p_offset
            );      
            // printf("load_addr: %p\n", load_addr);
            if (load_addr == MAP_FAILED) {
                printf("Memory mapping failed");
                loader_cleanup();
                return;
            }

            if (lseek(fd, phdr[i].p_offset, SEEK_SET) == -1 ||
                read(fd, load_addr, phdr[i].p_filesz) != phdr[i].p_filesz) {
                printf("Error reading segment content");
                loader_cleanup();
                return;
            }
        // }
    // }

    size_t entry_offset = (size_t)ehdr->e_entry - phdr[0].p_vaddr;
    int (*navigated_entry_point)() = (int (*)())((char*) ehdr->e_entry);
    close(fd);

    int result = navigated_entry_point();
    // printf("Returned from _start\n");
    printf("User _start return value = %d\n", result);
    if (first_successful) {
        page_allocations+=required_pages;
        internal_fragmentation = (allocation_size-phdr[i].p_memsz )/1024;
        first_successful = 0;  // Reset the flag
    }
    loader_cleanup();
}

int main(int argc, char** argv) {
    if (argc != 2) {
        printf("Usage: %s <ELF Executable> \n", argv[0]);
        exit(1);
    }
    signal(SIGSEGV, segmentation_fault_handler);
    int i=0;
    sigsetjmp(jump_buffer, 1);
    printf("i: %d\n", i);
        // Move the ehdr declaration and initialization here
    Elf32_Ehdr *ehdr;
    int fd = open(argv[1], O_RDONLY);
    if (fd == -1) {
        printf("Error opening file");
        return 1;
    }
    ehdr = (Elf32_Ehdr*)malloc(sizeof(Elf32_Ehdr));
    if (!ehdr) {
        printf("Memory allocation failed");
        close(fd);
        return 1;
    }
    if (read(fd, ehdr, sizeof(Elf32_Ehdr)) != sizeof(Elf32_Ehdr)) {
        printf("Error reading ELF header");
        free(ehdr);
        close(fd);
        return 1;
    }
    close(fd);

    while (i < ehdr->e_phnum) {
        printf("i: %d\n", i);
        load_and_run_elf(argv, i++);
    }
    printf("page_faults: %d\n", page_faults);
    printf("page_allocations: %d\n", page_allocations);
    printf("internal_fragmentation: %zu\n", internal_fragmentation);
loader_cleanup();
    return 0;
}
