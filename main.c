/**
 * @file main.c
 * @author Giordii (admin@giordii.dev)
 * @brief Parser di frame Ethernet - FASE 2
 * @version 2.0 // per dio
 * @date 2025-11-27
 * How to? si. 
 * Uso:
 *   ./macroscope              - legge da stdin
 *   ./macroscope frames.txt   - legge dal file
 *   ./macroscope -h           - mostra l'aiuto
 */

// --> INCLUSION ZONE <--
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "eth_frame.h"
// ----------------------

#define MAX_FRAME_BYTES 1518 // MTU + Ethernet header

/**
 * @brief occhio alla guida
 * @param prog_name Nome del programma
 */
static void print_usage(const char *prog_name) // cancro dopo aver scoperto di aver chiamato il exe in modo diverso
{
    printf("Uso: %s [options] [file]\n"
           "\n"
           "Parser di frame Ethernet (Ethernet II / 802.3 LLC / 802.3 SNAP)\n"
           "\n"
           "Opzioni:\n"
           "  -h, --help       Mostra questo messaggio di aiuto\n"
           "\n"
           "Argomenti:\n"
           "  [file]           Legge i frame dal file specificato\n"
           "                   Se omesso, legge da standard input\n"
           "\n"
           "Formato file:\n"
           "  - Un frame per riga in formato esadecimale\n"
           "  - Righe vuote o che iniziano con '#' sono ignorate\n"
           "  - Gli spazi tra i byte sono opzionali\n"
           "\n"
           "Esempi:\n"
           "  %s frames.txt          # Legge da file\n"
           "  %s                     # Legge da stdin\n"
           "  echo \"AA BB ... FF\" | %s\n",
           prog_name, prog_name, prog_name, prog_name);
}


/**
 * @brief cli parser
 * @param argc Numero argomenti
 * @param argv Array argomenti
 * @param fp_out Puntatore a FILE* per output
 * @return 0 se ok, 1 se errore, 2 se aiuto mostrato
 * si ci voleva anche questo (sono le 02:43 AM)
 */

static int cli_parse(int argc, char *argv[], FILE **fp_out)
{
    // no arg --> input on /dev/stdin
    if (argc == 1) {
        *fp_out = stdin;
        printf("Inserisci il tuo frame (HEX) \t (Ctrl+D --> exit):\n");
        return 0;
    }

    // LEGGI la prossima volta...
    if (argc > 2) {
        fprintf(stderr, "...troppi argomenti\n\n"); // /dev/stderr (tanto è /dev/stdout)
        print_usage(argv[0]);
        return 1;
    }

    // man paaage? SI
    if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
        print_usage(argv[0]);
        return 2; // error code per l'help --> "help shown"
    }

    // provo ad aprire il file
    *fp_out = fopen(argv[1], "r");
    if (!*fp_out) {
        fprintf(stderr, "impossibile aprire il file (avrai scritto il nome sbagliato...    ...pefforza) '%s'\n", argv[1]);
        perror("fopen");
        return 1;
    }

    return 0;
}

/**
 * @brief main function
 * 
 * ora arriva il bello...
 * 
 * @param argc n argomenti
 * @param argv array argomenti
 * @return 0 se ok, 1 se errore
 */
int main(int argc, char *argv[])
{
    payload_t frame; // struct payload_t per frm completo
    eth_frame_t eth; // struct per l'header 
    llc_pdu_t llc; // struct per l'LLC
    snap_pdu_t snap; // struct per SNAP
    FILE *fp = NULL;
    uint8_t buf[MAX_FRAME_BYTES]; // buffer per frame
    int len; // 4byte... (troppo)
    int cli_result; // return code da cli_parse

    cli_result = cli_parse(argc, argv, &fp); // chiamiamo l'ultima ora di sofferenza
    if (cli_result == 2) {
        return 0; // nn ha gasato
    }
    if (cli_result != 0) {
        return 1; // nn ha gasato
    }

    // ora c'e la gabola del multi frame in singolo file...
    while (1) {
        len = read_frame_from_text_file(fp, buf, sizeof(buf)); // leggo un frame, se c'e sarà len frame altrimeti 0 o -1

        if (len == 0) {
            break;
        } // basta. abbiamo finito, by god

        if (len < 0) {
            fprintf(stderr, "Kernel Panic\n");
            continue;
        }

        frame.data = buf; // quanto le godo le struct
        frame.len  = (size_t)len;

        if (parse_ethernet(&frame, &eth) < 0) {
            fprintf(stderr, "error\n");
            continue;
        }

        // tostring? SI
        print_eth_summary(&eth);

        // Questo va sofferto...
        if (eth.frame_type == ETH_KIND_8023_LLC) {
            if (parse_llc(&eth.payload, &llc) >= 0) {
                print_llc_summary(&llc);

                if (llc.dsap == 0xAA && llc.ssap == 0xAA) {
                    if (parse_snap(&llc.payload, &snap) == 0) {
                        print_snap_summary(&snap);
                        eth.frame_type = ETH_KIND_8023_SNAP;
                    } else {
                        fprintf(stderr, "SNAP found ma parsing failed\n");
                    }
                }
            } else {
                fprintf(stderr, "frame 802.3 found ma parsing LLC failed\n");
            }
        }

        putchar('\n');
    }

    if (fp != stdin) {
        fclose(fp);
    }

    printf("see author site? (y/n): ");
    char risposta;
    if (scanf(" %c", &risposta) == 1 && (risposta == 'y' || risposta == 'Y')) {
        printf("Opening https://giordii.dev...\n");
        system("xdg-open https://giordii.dev 2>/dev/null || open https://giordii.dev 2>/dev/null || start https://giordii.dev");
    }
    printf("\n\nbtw not repeat this crazy project again, PLEASE\n");
    return 0;
}
