/*
 * CS 261 PA2: Mini-ELF loader
 *
 * Name: Adrien Ponce
 */

#include "p2-load.h"

/**********************************************************************
 *                         REQUIRED FUNCTIONS
 *********************************************************************/

bool read_phdr (FILE *file, uint16_t offset, elf_phdr_t *phdr)
{
    if (file == NULL || phdr == NULL || !offset) {
        return false;
    }

    if (fseek(file, offset, SEEK_SET) != 0) {           // seek so file starts at offset
        return false;
    }

    if (fread(phdr, sizeof(elf_phdr_t), 1, file) != 1) {
        return false;
    }

    if (phdr->magic == 0xDEADBEEF) {
        return true;
    }

    return false;
}

bool load_segment (FILE *file, byte_t *memory, elf_phdr_t phdr)
{
    // brief error checking
    if (file == NULL || memory == NULL || phdr.p_offset < 0) {
        return false;
    }
	
    if (phdr.p_filesz + phdr.p_vaddr > MEMSIZE || phdr.p_vaddr > MEMSIZE
		|| phdr.p_vaddr < 0) {
        return false;
    }

    if (phdr.p_filesz == 0) {
        return true;
    }

    if (phdr.p_offset >= MEMSIZE) {
        return false;
    }

    if (fseek(file, phdr.p_offset, SEEK_SET) != 0) {    // seek to the offset of the phdr
        return false;
    }

    if (fread(&memory[phdr.p_vaddr], phdr.p_filesz, 1, file) != 1 
		&& phdr.p_filesz) {
        return false;
    }

    return true;
}

/**********************************************************************
 *                         OPTIONAL FUNCTIONS
 *********************************************************************/

void usage_p2 (char **argv)
{
    printf("Usage: %s <option(s)> mini-elf-file\n", argv[0]);
    printf(" Options are:\n");
    printf("  -h      Display usage\n");
    printf("  -H      Show the Mini-ELF header\n");
    printf("  -a      Show all with brief memory\n");
    printf("  -f      Show all with full memory\n");
    printf("  -s      Show the program headers\n");
    printf("  -m      Show the memory contents (brief)\n");
    printf("  -M      Show the memory contents (full)\n");
}

bool parse_command_line_p2 (int argc, char **argv,
                            bool *print_header, bool *print_segments,
                            bool *print_membrief, bool *print_memfull,
                            char **filename)
{
    // brief error checking for NULL
    if (argv == NULL || print_header == NULL || print_segments == NULL
            || print_membrief == NULL || print_memfull == NULL || filename == NULL) {
        return false;
    }

    // parameter parsing w/ getopt()
    int c;
    while ((c = getopt(argc, argv, "hHafsmM")) != -1) {
        switch (c) {
            case 'h':                   // display usage
                usage_p2(argv);
                return true;
            case 'H':                   // display Mini-Elf header
                *print_header = true;
                break;
            case 'a':                   // display H, s, m flags
                *print_header = true;
                *print_segments = true;
                *print_membrief = true;
                break;
            case 'f':                   // display H, s, M flags
                *print_header = true;
                *print_segments = true;
                *print_memfull = true;
                break;
            case 's':                   // display program headers
                *print_segments = true;
                break;
            case 'm':                   // display brief memory contents
                *print_membrief = true;
                break;
            case 'M':                   // display full memory contents
                *print_memfull = true;
                break;
            default:                    // display usage (default)
                usage_p2(argv);
                return false;
        }
    }

    if (optind != argc-1) {
        // no filename (or extraneous input)
        usage_p2(argv);
        return false;
    }

    *filename = argv[optind];   // save filename

    // -m and -M flag invalid at the same time
    if (*print_memfull && *print_membrief) {
        usage_p2(argv);
        return false;
    }

    return true;
}

void dump_phdrs (uint16_t numphdrs, elf_phdr_t phdr[])
{
    printf(" Segment   Offset    VirtAddr  FileSize  Type      Flag\n");    // print top of table

    for (int i = 0; i < numphdrs; i++) {
        printf("  %02d", i);                        // print segments
        printf("       0x%04x", phdr[i].p_offset);  // print offset
        printf("    0x%04x", phdr[i].p_vaddr);      // print virtual address
        printf("    0x%04x", phdr[i].p_filesz);     // print file size
        // enum handling
        elf_segtype_t type = phdr[i].p_type;
        printf("    ");
        switch (type) {
            case 0:
                printf("DATA");
                printf("      ");
                break;
            case 1:
                printf("CODE");
                printf("      ");
                break;
            case 2:
                printf("STACK");
                printf("     ");
                break;
            case 3:
                printf("HEAP");
                printf("      ");
                break;
            case 4:
                printf("UNKNOWN");
                printf("   ");
                break;
        }
        // flag handling (1 = X, 2 = W, R = 4) (RWX)
        uint16_t flag = phdr[i].p_flag;
        switch (flag) {
            case 1:
                printf("  X");
                break;
            case 2:
                printf(" W ");
                break;
            case 3:
                printf(" WX");
                break;
            case 4:
                printf("R  ");
                break;
            case 5:
                printf("R X");
                break;
            case 6:
                printf("RW ");
                break;
            case 7:
                printf("RWX");
                break;
            default:
                printf("   ");
                break;
        }
        printf("\n");
    }
}

void dump_memory (byte_t *memory, uint16_t start, uint16_t end)
{
    printf("Contents of memory from %04x to %04x:", start, end);

    for (int i = start; i < end; i++) {
        if (i % 16 == 0) {
            printf("\n  %04x  ", i);
        } else if (i % 8 == 0) {
            printf(" ");
        }
        if (i % 16 == 0) {
            printf("%02x", memory[i]);
        } else {
            printf(" %02x", memory[i]);
        }
    }
    printf("\n");

}

