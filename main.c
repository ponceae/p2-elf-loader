/*
 * CS 261: Main driver
 *
 * Name: Adrien Ponce
 */

#include "p1-check.h"
#include "p2-load.h"

#define PHDR_SIZE 20

int main (int argc, char **argv)
{
    bool print_header = false;
    bool print_segments = false;
    bool print_membrief = false;
    bool print_memfull = false;
    char *filename;

    // grab contents of command line
    if (!parse_command_line_p2(argc, argv, &print_header, &print_segments,
                               &print_membrief, &print_memfull, &filename)) {
        exit(EXIT_FAILURE);
    }

	// begin ELF memory reading
    if (filename != NULL) {

        // open Mini-ELF file
        FILE *file = fopen(filename, "r");
        if (!file) {
            printf("Failed to read file\n");
            exit(EXIT_FAILURE);
        }

        // p1 read header
        elf_hdr_t hdr;
        if (!read_header(file, &hdr)) {
            printf("Failed to read file\n");
            exit(EXIT_FAILURE);
        }

        // p1 output
        if (print_header) {
            dump_header(hdr);
        }

        // p2 read each program header from the Mini-ELf file
        struct elf_phdr phdrs[hdr.e_num_phdr];
        uint32_t offset;
        int i;
        // loop through program headers
        for (i = 0, offset = hdr.e_phdr_start; i < hdr.e_num_phdr; i++, offset += PHDR_SIZE) {
            if (!read_phdr(file, offset, &phdrs[i])) {
                printf("Failed to read file\n");
                exit(EXIT_FAILURE);
            }
        }

        // p2 output print segments (no loading)
        if (print_segments) {
            dump_phdrs(hdr.e_num_phdr, phdrs);
        }

        // p2 read each segment of each program header from the Mini-ELF file
        byte_t* memory = (byte_t*)calloc(MEMSIZE, 1);
        for (int i = 0; i < hdr.e_num_phdr; i++) {
            if (!load_segment(file, memory, phdrs[i])) {
                printf("Failed to read file\n");
                exit(EXIT_FAILURE);
            }

			// show the brief memory of the phdr
            if (print_membrief) {
                dump_memory(memory, phdrs[i].p_vaddr, phdrs[i].p_vaddr + phdrs[i].p_filesz);
            }
        }

		// print the full memory of the phdr
        if (print_memfull) {
            dump_memory(memory, 0, MEMSIZE);
        }
		
		// end ELF read
        free(memory);
		fclose(file);
    }

    return EXIT_SUCCESS;
}

