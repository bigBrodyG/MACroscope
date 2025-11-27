/**
 * @file eth_frame.c
 * @author Giordii (https://giordii.dev)
 * @brief Parser di frame Ethernet - FASE 1
 * @version 1.0 // per dio
 * @date 2025-11-20
 * @copyright Copyright (c) 2025 // fa ti! dopo un progetto cosi...
 * -100 salute mentale added? "eh si"
 */


// --> INCLUSION ZONE <--
#include "eth_frame.h"
#include "tables.h"
#include <string.h>
// ----------------------



/**
 * hex_value
 * ---------
 * @brief char to hex value
 * @param c char
 * @return 0-15, -1 se non valido
 * top funzione inutile 
 */
static int hex_value(char c)
{
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return 10 + (c - 'a');
    if (c >= 'A' && c <= 'F') return 10 + (c - 'A');
    return -1; // char non valido
}

/**
 * @brief hex to byte buffer
 * 
 * @param hex ASCII con hex (0-9, a-f, A-F)
 * @param buf buffer per i byte
 * @param bufsize max size buffer
 * @return int byte scritti se ok, -1 se errore
 */
int hex_to_bytes(const char *hex, uint8_t *buf, size_t bufsize) {
    size_t out_len = 0;
    int nob = -1;

    while (*hex) {
        if (isspace((unsigned char)*hex)) {
            ++hex; // aumento hex prima di continuare
            continue;
        }

        int nib = hex_value(*hex);
        if (nib < 0) return -1;

        if (nob < 0) {
            nob = nib;
        } else {
            if (out_len >= bufsize) return -1;
            buf[out_len++] = (uint8_t)((nob << 4) | nib);
            nob = -1;
        }
        ++hex;
    }
    return (nob < 0) ? (int)out_len : -1;
}

/**
 * @brief read frame from file in hex
 * 
 * @param fp file opened
 * @param buf buffer di dest per i byte del frame
 * @param bufsize dimensione massima del buffer
 * @return int lunghezza del frame letto se ok, 0 se EOF, -1 se errore di formato
 */
int read_frame_from_text_file(FILE *fp, uint8_t *buf, size_t bufsize)
{
    char line[4096];

    while (fgets(line, sizeof(line), fp)) {
        char *p = line;

        // skip spazi iniziali
        while (isspace((unsigned char)*p)) {
            ++p;
        }

        // salta commenti o righe vuote
        if (*p == '\0' || *p == '#') {
            continue;
        }

        // converto la riga in byte
        int len = hex_to_bytes(p, buf, bufsize);
        if (len < 0) {
            return -1; // formato errato
        }
        return len;
    }

    return 0; // EOF senza frame valido
}

/**
 * @brief parsa primi 14 byte come header Ethernet
 * 
 * estrae MAC dest/src, type/len, inizializza payload_t
 * deduce tipo frame (Ethernet II se type_or_len >= 0x0600, altrimenti 802.3)
 * 
 * @param buf puntatore a buffer completo (L2 + payload)
 * @param eth struttura dove salvare header parsato
 * @return int lunghezza totale se ok, -1 se buffer troppo corto (< 14 byte)
 */
int parse_ethernet(const payload_t *buf, eth_frame_t *eth)
{
    if (buf == NULL || eth == NULL) return -1;
    if (buf->len < 14) return -1; // header minimo

    // copio MAC dst e src (6 byte ciascuno)
    memcpy(eth->dst.bytes, buf->data, 6);
    memcpy(eth->src.bytes, buf->data + 6, 6);

    // type/len in big-endian (2 byte)
    eth->type_or_len = (uint16_t)((buf->data[12] << 8) | buf->data[13]);

    // payload = vista su dati successivi (no copia, solo ptr)
    eth->payload.data = buf->data + 14;
    eth->payload.len  = buf->len  - 14;
    eth->total_len    = (uint16_t)buf->len;

    // deduco tipo frame (Ethernet II >=0x0600, altrimenti 802.3)
    eth->frame_type = (eth->type_or_len >= 0x0600) ? ETH_KIND_ETHERNET_II : ETH_KIND_8023_LLC;

    return (int)buf->len;
}

/**
 * @brief parso PDU LLC (Logical Link Control)
 * 
 * estraggo DSAP, SSAP, control byte e payload successivo (FASE 2 - stub)
 * 
 * @param raw puntatore a dati da parsare (deve iniziare da primo byte LLC)
 * @param llc struttura dove salvo header LLC parsato
 * @return int 0 se ok, -1 se buffer troppo corto (< 3 byte)
 */
int parse_llc(payload_t *raw, llc_pdu_t *llc)
{
    if (raw == NULL || llc == NULL) return -1;
    if (raw->len < 3) return -1; // minimo 3 byte per LLC

    llc->dsap    = raw->data[0]; // DSAP (dest service access point)
    llc->ssap    = raw->data[1]; // SSAP (source service access point)
    llc->control = raw->data[2]; // control field
    llc->payload.data = raw->data + 3;
    llc->payload.len  = raw->len  - 3;

    return 0;
}

/**
 * @brief parsa header SNAP (SubNetwork Access Protocol)
 * 
 * estraggo OUI (3 byte), protocol_id (2 byte) e payload successivo
 * 
 * @param raw puntatore a dati da parsare (deve iniziare da primo byte SNAP)
 * @param snap struttura dove salvo header SNAP parsato
 * @return int 0 se ok, -1 se buffer troppo corto (< 5 byte)
 */
int parse_snap(payload_t *raw, snap_pdu_t *snap)
{
    if (raw == NULL || snap == NULL || raw->len < 5) return -1;

    memcpy(snap->oui, raw->data, 3);
    snap->protocol_id = (uint16_t)((raw->data[3] << 8) | raw->data[4]);
    snap->payload.data = raw->data + 5;
    snap->payload.len  = raw->len  - 5;

    return 0;
}

/**
 * @brief stampa MAC con decodifica opzionale
 * 
 * @param mac indirizzo MAC da stampare
 * @param separator char tra i byte (':' '-' ' ')
 * @param decode se != 0, stampa anche decodifica (bcast/mcast/unicast + vendor)
 */
void print_mac(const mac_addr_t *mac, char separator, int decode)
{
    // stampo MAC (formato XX:XX:XX:XX:XX:XX o simile)
    for (int i = 0; i < 6; ++i) {
        printf("%02X", mac->bytes[i]);
        if (i < 5) putchar(separator);
    }

    if (!decode) return;

    // check broadcast (tutti 0xFF) - usa memcmp per efficienza
    static const uint8_t bcast[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    if (memcmp(mac->bytes, bcast, 6) == 0) {
        printf("  (Broadcast)");
        return;
    }

    // bit 0 primo byte: multicast (1) o unicast (0)
    // bit 1 primo byte: locale (1) o universale (0)
    uint8_t first = mac->bytes[0];
    printf("  (%s, %s", (first & 0x01) ? "Multicast" : "Unicast",
                        (first & 0x02) ? "Locale" : "Universale");

    // OUI = primi 3 byte (vendor ID)
    uint32_t oui = ((uint32_t)mac->bytes[0] << 16) |
                   ((uint32_t)mac->bytes[1] << 8)  |
                    (uint32_t)mac->bytes[2];
    const char *vendor = oui_lookup(oui);
    if (vendor && vendor[0] && strcmp(vendor, "Sconosciuto") != 0) {
        printf(", %s", vendor);
    }
    putchar(')');
}

/**
 * @brief stampa il riepilogo completo di un frame Ethernet
 * 
 * mostra MAC destinazione/sorgente (con decodifica), Type/Length, tipo di frame e dimensione payload
 * 
 * @param eth puntatore alla struttura eth_frame_t da stampare
 */
void print_eth_summary(const eth_frame_t *eth)
{
    printf("Destinazione : ");
    print_mac(&eth->dst, ':', 1);
    printf("\nSorgente     : ");
    print_mac(&eth->src, ':', 1);

    // Type/Length
    printf("\nType/Length  : 0x%04X", eth->type_or_len);
    if (eth->type_or_len >= 0x0600) {
        printf("  (Ethertype: %s)", protocol_name(eth->type_or_len));
    } else {
        printf("  (Lunghezza payload: %u)", eth->type_or_len);
    }

    // tipo frame e payload size
    static const char *ftypes[] = {"802.3 LLC", "Ethernet II", "802.3 SNAP"};
    printf("\nTipo frame   : %s\n", ftypes[eth->frame_type]);
    printf("Payload size : %zu byte\n", eth->payload.len);
}

/**
 * @brief stampa riepilogo PDU LLC
 * 
 * mostro DSAP, SSAP (con lookup nome) e control byte
 * 
 * @param llc puntatore a struttura llc_pdu_t da stampare
 */
void print_llc_summary(const llc_pdu_t *llc)
{
    printf("LLC -> DSAP: 0x%02X (%s), SSAP: 0x%02X (%s), CTRL: 0x%02X\n",
           llc->dsap, sap_lookup(llc->dsap),
           llc->ssap, sap_lookup(llc->ssap),
           llc->control);
}

/**
 * @brief stampa riepilogo PDU SNAP
 * 
 * mostro OUI (con lookup vendor), protocol_id (con lookup protocollo)
 * 
 * @param snap puntatore a struttura snap_pdu_t da stampare
 */
void print_snap_summary(const snap_pdu_t *snap)
{
    uint32_t oui = ((uint32_t)snap->oui[0] << 16) |
                   ((uint32_t)snap->oui[1] << 8)  |
                    (uint32_t)snap->oui[2];
    printf("SNAP -> OUI: %02X-%02X-%02X (%s), PID: 0x%04X (%s)\n",
           snap->oui[0], snap->oui[1], snap->oui[2],
           oui_lookup(oui),
           snap->protocol_id, protocol_name(snap->protocol_id));
}
