/**
 * @file ethernet_frame.c
 * @author Giordii (admin@giordii.dev)
 * @brief Parser di un frame Ethernet
 * @version 0.9-gasa
 * @date 2025-11-14
 * @copyright Copyright (c) 2025
 * 
 */

#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define ETH_MAC_LEN 6U // byte degli indirizzi MAC (u=unsgn int)
#define ETH_TYPE_LEN 2U // byte del EtherType (unsgn int)
#define MIN_FRAME_BYTES 14 // 6 + 6+ 2
#define MAX_FRAME_BYTES 1514 // 14 + 1500 (no FCS)
#define MAX_INPUT_CHARS (MAX_FRAME_BYTES * 3) // ogni byte in testo diventa 2 char hex + space

char banner[] = "\n"
"                                                                        \n"
"o     o      .oo .oPYo.                                                 \n"
"8b   d8     .P 8 8    8                                                 \n"
"8`b d'8    .P  8 8      oPYo. .oPYo. .oPYo. .oPYo. .oPYo. .oPYo. .oPYo. \n"
"8 `o' 8   oPooo8 8      8  `' 8    8 Yb..   8    ' 8    8 8    8 8oooo8 \n"
"8     8  .P    8 8    8 8     8    8   'Yb. 8    . 8    8 8    8 8.     \n"
"8     8 .P     8 `YooP' 8     `YooP' `YooP' `YooP' `YooP' 8YooP' `Yooo' \n"
"..::::....:::::..:.....:..:::::.....::.....::.....::.....:8 ....::.....:\n"
"::::::::::::::::::::::::::::::::::::::::::::::::::::::::::8 ::::::::::::\n"
"::::::::::::::::::::::::::::::::::::::::::::::::::::::::::..::::::::::::\n";

/** 
 * @brief nuovo tipo di dato per frame eth (struct)
 * typedef 
 *      perchè permette di usare il nome scelto dopo la dichiarazione (EthFrame) senza scrivere "struct"
 * 
 * struct
 *      tipo di dato composto che consente di raggruppare variabili di tipi diversi sotto un unico nome
 * 
 * scorrimnto tra campi
 *     utilizzando gli offset (ogni var certa dimensione, struct salvate in spazio contiguo) 
 * 
 */
typedef struct {
    uint8_t dst_mac[ETH_MAC_LEN]; // mac destinazione
    uint8_t src_mac[ETH_MAC_LEN]; // mac sorgente
    uint16_t ether_type; // tipo di protocollo
    uint8_t payload[1500]; // dati
    size_t payload_len; // lunghezza dati
    size_t total_len; // lunghezza totale frame
} EthFrame; // nome del nuovo tipo di dato

/**
 * @brief map per codici ethtype a stringhe protocolli
 * 
 */
typedef struct {
    uint16_t value;
    const char *name; // il testo pointed RO, ma puntatore no (const char const *name) per puntatore fisso
} EthString;

/**
 * @brief esempi forniti dal professore
 * @details protocolli comuni come IP (v4 e v6), ARP, VLAN
 * const pk dict RO (nn devo cambiarlo dopo)
 * in memoria di sola lettura (.rodata)
 *
 */
const EthString examples[] = {
    {0x0800, "IPv4"},
    {0x0806, "ARP"},
    {0x0842, "Wake-on-LAN"},
    {0x8100, "802.1Q VLAN"},
    {0x86DD, "IPv6"},
};

/**
 * lookup_ethertype
 * -----------------
 * @brief cerca il valore in hex e ritorna il protocollo corrispondente
 */
const char *lookup_ethertype(uint16_t value) {
    size_t count = sizeof(examples) / sizeof(examples[0]); // count degli item in ex dict pk count nn c'è in c :(

    for (size_t i = 0; i < count; ++i) {
        // cerco il valore nella lista
        if (examples[i].value == value) {
            // ritorno il nome linked all'ip
            return examples[i].name;
        }
    }
    // ritorno il literal se non trovo niente
    return "sconosciuto";
}



/**
 * read_input
 * ----------
 * @brief legge l'input solo da file e lo salva in un buffer
 */
int read_input(char *buffer, size_t size, const char *filename) {
    if (!filename) {
        return -1;
    }

    FILE *fp = fopen(filename, "r"); /* gestito dalla libc */
    if (!fp) {
        printf("file error\n");
        return -1;
    }
    if (!fgets(buffer, (int)size, fp)) {
        printf("empty file\n");
        fclose(fp);
        return -1;
    }

    fclose(fp);
    return 0;
}



/**
 * remove_spaces
 * -------------
 * @brief riumuove gli spazi dalla stringa di input
 * 
 * @param src la stringa potenzialmente piena di " " 
 * @param dst stringa strippata
 */
 void remove_spaces(const char *src, char *dst) {
    while (*src) {
        if (!isspace((unsigned char)*src)) {
            *dst++ = *src;
        }
        src++;
    }
    *dst = '\0';
}

/**
 * hex_value
 * ---------
 * @brief converto il char in hex
 * 
 * @param c char da convertire
 * @return hex value or -1 if error
 */

 int hex_value(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return 10 + (c - 'a');
    if (c >= 'A' && c <= 'F') return 10 + (c - 'A');
    return -1;
}



/**
 * parse_hex_string
 * ----------------
 * 
 * eseguito dopo remove_spaces
 * 
 * @brief converte la hex string in array di byte
 * 
 * @param hex stringa esadecimale
 * @param out array di byte
 * @param out_len numero di byte scritti
 * @return int bool like, 0 se ok, -1 se errore
 */
int parse_hex_string(const char *hex, uint8_t *out, size_t *out_len) {
    size_t len = strlen(hex);
    if (len == 0) {
        printf("bruh\n");
        return -1;
    }
    // faccio tutte le vaiidazioni del caso
    
    // !-- numero di nibble pari --!
    if (len % 2 != 0) {
        printf("n nibble dispari (%zu)\n", len);
        return -1;
    }
    // !-- almeno 14 byte totali; --!
    size_t byte_len = len / 2; // un byte = 2 char hex
    if (byte_len < MIN_FRAME_BYTES) {
        printf("Frame too short (%zu byte, min %u)\n", byte_len, MIN_FRAME_BYTES);
        return -1;
    }
    // !-- payload ≤ 1500 byte (quindi len ≥ 14 e len-14 ≤ 1500). --!
    if (byte_len > MAX_FRAME_BYTES) {
        printf("Frame too long (%zu byte, max %u)\n", byte_len, MAX_FRAME_BYTES);
        return -1;
    }

    /* scorro di 2 posizioni e prendo i 2 hex char per trasformarli in un byte (1 byte = 2 hex char = 2 nibbl3) */
    for (size_t i = 0; i < byte_len; ++i) {
        int n1 = hex_value(hex[2 * i]); // 2*0 = 0, 2*1 = 2
        int n2 = hex_value(hex[2 * i + 1]); // 2*0+1 = 1, 2*1+1 = 3
        
        out[i] = (uint8_t)((n1 << 4) | n2); // sposto n1 di un nibble a sinistra e faccio or con n2 (unione )
    }

    *out_len = byte_len;
    return 0;
}


/**
 * fill_frame
 * ----------
 * @brief inserisco tutti i byte nellla struct
 * 
 * @param raw array di byte del frame quasi parsato
 * @param len n byte in raw
 * @param frame puntatore a struct
 */
void fill_frame(const uint8_t *raw, size_t len, EthFrame *frame) {
    for (size_t i = 0; i < ETH_MAC_LEN; ++i) { // prendo i 6 byte del MAC
        frame->dst_mac[i] = raw[i];
    }
    for (size_t i = 0; i < ETH_MAC_LEN; ++i) { // prendo 6 byte dell'altro mac
        frame->src_mac[i] = raw[ETH_MAC_LEN + i];
    }
    // 2 byte ether type
    frame->ether_type = (uint16_t)((raw[12] << 8) | raw[13]); // cast esplicito a 16 bit unsigned | predo il byte 12 (6+6+1) e shift di 8 or byte 13
    frame->payload_len = len - MIN_FRAME_BYTES; // len payload = tot len - 14 byte (6+6+2)
    frame->total_len = len; // len totale del frame

    for (size_t i = 0; i < frame->payload_len; ++i) {
        frame->payload[i] = raw[MIN_FRAME_BYTES + i]; // offset di 14 poi copia bit a bit
    }
}


/**
 * describe_mac
 * ------------
 * @brief spiega il tipo di mac (broadcast, multicast, unicast)
 * 
 * @param mac 6 byte
 * @return literal descr 
 */
 const char *describe_mac(const uint8_t mac[6]) {
    int is_broadcast = 1;
    for (size_t i = 0; i < 6; ++i) {
        if (mac[i] != 0xFF) {
            is_broadcast = 0; // se trovo un byte nn FF no broadcast
            break;
        }
    }
    if (is_broadcast) {
        return "Broadcast";
    }
    
    int is_multicast = (mac[0] & 0x01) != 0;
    int is_local = (mac[0] & 0x02) != 0;
    if (is_multicast) {
        return is_local ? "multicast locale" : "multicast universale";
    }
    return is_local ? "unicast locale" : "unicast universale"; // operatore ternario per dio
}

/**
 * @brief main function, gestisce input, parsing e output
 * 
 * stampa il frame ethernet decodificato, con indirizzi MAC, EtherType e payload
 * @param argc n argomenti
 * @param argv array di argomenti
 * @return int 0 o 1
 */
int main(int argc, char *argv[]) {
    // no arg error managment 
    if (argc > 2) {
        printf("Uso: %s [file_input.txt]\n", argv[0]);
        return -1;
    }

    printf("%s", banner);
    // file o tastiera
    char input_line[MAX_INPUT_CHARS];
    if (argc == 2) {
        if (read_input(input_line, sizeof(input_line), argv[1]) != 0) {
            return -1;
        }
    } else {
        printf("Inserisci frame in esadecimale (byte separati da spazi o contigui):\n");
        size_t index = 0;
        if (scanf("%[^\n]", input_line) != 1) { // scanf legge fino al whitespace ma voglio tutta la riga --> %[^\n] che legge tutto fino a '\n' (escluso)
            printf("Nessun dato letto\n");
            return -1;
        }
        getchar(); // prendo il \n che non ho gestito prima (grossi bug altrimenti)
    }
    
    char sanitized[MAX_INPUT_CHARS]; // dove salvo char * no space
    remove_spaces(input_line, sanitized);
    
    uint8_t bytes[MAX_FRAME_BYTES]; // array di byte del frame
    size_t frame_len = 0;
    if (parse_hex_string(sanitized, bytes, &frame_len) != 0) {
        return -1;
    }

    EthFrame frame;
    memset(&frame, 0, sizeof(frame)); /* azzera tutti i campi: frame e' sullo stack e puntatore &frame passa l'indirizzo */
    fill_frame(bytes, frame_len, &frame);

    printf("\n=== FRAME ETHERNET DECODIFICATO ===\n");

    const char *dst_desc = describe_mac(frame.dst_mac);
    const char *src_desc = describe_mac(frame.src_mac);

    printf("Destinazione : %02X:%02X:%02X:%02X:%02X:%02X  (%s)\n", frame.dst_mac[0], frame.dst_mac[1], frame.dst_mac[2], frame.dst_mac[3], frame.dst_mac[4], frame.dst_mac[5], dst_desc);
    printf("Sorgente     : %02X:%02X:%02X:%02X:%02X:%02X  (%s)\n", frame.src_mac[0], frame.src_mac[1], frame.src_mac[2], frame.src_mac[3], frame.src_mac[4], frame.src_mac[5], src_desc);
    printf("EtherType    : 0x%04X (%s)\n", frame.ether_type, lookup_ethertype(frame.ether_type));
    printf("Payload size : %zu byte\n", frame.payload_len);
    printf("\n-------------------------------------------\n\nPayload (esadecimale/ASCII):\n");
    
    if (frame.payload_len == 0) {
        printf("(vuoto)\n");
    } else { // 16 byte per ln, hex + ascii
        for (size_t i = 0; i < frame.payload_len; i += 16) {
            size_t chunk = (frame.payload_len - i > 16) ? 16 : frame.payload_len - i;
            char ascii[17];
            for (size_t j = 0; j < chunk; ++j) {
                printf("%02X ", frame.payload[i + j]);
                uint8_t c = frame.payload[i + j];
                ascii[j] = isprint(c) ? (char)c : '.';
            }
            ascii[chunk] = '\0';
            for (size_t j = chunk; j < 16; ++j) {
                printf("   ");
            }
            printf("%s\n", ascii);
        }
    }

    printf("---------------------------------\n");
    printf("Frame totale : %zu byte\n", frame.total_len);
    return 0;
}