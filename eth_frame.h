/* =========================================================================
 *  eth_frame.h  --  Tipi e prototipi per il parsing di frame Ethernet / LLC / SNAP
 * ========================================================================= */

#ifndef ETH_FRAME_H
#define ETH_FRAME_H

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>


/* -------------------------------------------------------------------------
 *  VISTA DI BYTE (no copia, solo puntatore al buffer originale)
 * ------------------------------------------------------------------------- */

typedef struct {
    const uint8_t *data;   /* inizio dei dati              */
    size_t         len;    /* numero di byte disponibili   */
} payload_t;               /* l'aggiunta di len lo trasforma in un payload intelligente: 
                            conosce la propria lunghezza*/

/* -------------------------------------------------------------------------
 *  MAC address
 * ------------------------------------------------------------------------- 
* Rispetto a "typedef uint8_t mac_addr_t[6];"
 * migliora chiarezza semantica e sicurezza di tipo, */
typedef struct {
    uint8_t bytes[6];
} mac_addr_t;

/* -------------------------------------------------------------------------
 *  ETH L2: header base (valido per Ethernet II e 802.3)
 * ------------------------------------------------------------------------- */

typedef enum {
    ETH_KIND_ETHERNET_II,
    ETH_KIND_8023_LLC,
    ETH_KIND_8023_SNAP
} frame_type_t;

typedef struct {
    mac_addr_t dst;
    mac_addr_t src;
    uint16_t   type_or_len;   /* Ethertype (>=0x0600) oppure lunghezza (<0x0600) */
    payload_t payload;        /* tutto ciò che viene dopo i 14 byte di header   */
    
    /*  I seguenti campi servono per migliorare la gestione del frame: tipo metadati 
    e vengono popolati dalla funzione di parsing */
    frame_type_t frame_type; /* tipo di frame (Ethernet II / 802.3 LLC / 802.3 SNAP) */
    uint16_t total_len;      /* lunghezza totale del frame (header + payload) */
}  eth_frame_t;

/* -------------------------------------------------------------------------
 *  LLC
 * ------------------------------------------------------------------------- */

typedef struct {
    uint8_t dsap;
    uint8_t ssap;
    uint8_t control;
    payload_t payload;   /* dati incapsulati dentro LLC (es. SNAP) */
} llc_pdu_t;

/* -------------------------------------------------------------------------
 *  SNAP
 * ------------------------------------------------------------------------- */

typedef struct {
    uint8_t  oui[3];
    uint16_t protocol_id;   /* spesso un Ethertype */
    payload_t payload;      /* dati incapsulati dentro SNAP (es. IPv4, ARP, ... o protocolli proprietari) */
} snap_pdu_t;

/* -------------------------------------------------------------------------
 *  DESCRIZIONE PROTOCOLLI (per tabella Ethertype / SNAP)
 * ------------------------------------------------------------------------- */

typedef struct {
    uint16_t value;         /* Ethertype o protocol_id SNAP */
    const char *name;       /* stringa descrittiva          */
} proto_desc_t;

/* =========================================================================
 *  FUNZIONI DA IMPLEMENTARE (compito)
 * ========================================================================= */

/* ---------------- HEX → BYTE BUFFER ------------------------------------- */
/**
 * Converte una stringa esadecimale in un buffer di byte.
 *  @param hex      : stringa ASCII (senza 0x, con spazi)
 *  @param buf      : buffer di output
 *  @param bufsize  : dimensione massima di buf
 *
 * Ritorna len (numero di byte scritti in buf) se OK, -1 se:
 *  - lunghezza stringa dispari
 *  - caratteri non validi
 *  - bufsize insufficiente
 */
int hex_to_bytes(const char *hex,
                 uint8_t *buf,
                 size_t bufsize);

/* ---------------- LETTURA FRAME DA FILE DI TESTO ------------------------ */
/**
 * Legge un frame da un file di testo in formato esadecimale.
 *
 * Formato atteso:
 *  - una riga = un frame
 *  - caratteri ammessi: 0-9, a-f, A-F, spazi
 *  - righe vuote o che iniziano con '#' sono commenti e vanno ignorate
 *  - spazi vanno ignorati
 *  - possibili errori: lunghezza dispari, caratteri non validi, se il frame non sta in bufsize, errore
 * 
 * Parametri:
 *  @param fp       : file di input già aperto in lettura testo
 *  @param buf      : buffer di destinazione per i byte del frame
 *  @param bufsize  : dimensione massima del buffer
 *
 * Valore di ritorno:
 *  -  len : lunghezza in byte del frame letto se OK
 *  -   0  : EOF raggiunto prima di leggere un frame valido
 *  -   -1 : errore di formato (es. caratteri non validi, lunghezza dispari,
 *         dati che non stanno in bufsize)
 *
 * Implementazione tipica:
 *  - usa fgets() per leggere una riga in un buffer temporaneo
 *  - skippa righe vuote / commenti
 *  - rimuove gli spazi dalla riga
 *  - chiama hex_to_bytes() per ottenere i byte nel buffer
 */
int read_frame_from_text_file(FILE *fp, uint8_t *buf, size_t bufsize);


/* ---------------- PARSING ETHERNET -------------------------------------- */
/**
 * Interpreta i primi 14 byte di raw come header Ethernet.
 *  @param buf       : passo il frame completo (L2 + payload) per indirizzo. Perchè non per copia?
 *  @param eth       : header Ethernet + payload successivo
 *
 * Ritorna len se OK, -1 se raw->len < 14 (unico controllo di integrità).
 *
 * NOTA:
 *  - eth->payload punta al primo byte dopo l'header Ethernet.
 *  - tipo viene inizializzato in base a type_or_len (>=0x0600 o <=1500).
 *  - la distinzione LLC / SNAP vera e propria avverrà guardando i campi LLC.
 */
int parse_ethernet(const payload_t *buf, eth_frame_t *eth);

/* ---------------- PARSING LLC ------------------------------------------- */
/**
 * Decodifica un PDU LLC all'inizio di raw.
 *  - raw  : deve iniziare dal primo byte LLC (DSAP)
 *  - llc  : header LLC + payload interno
 *
 * Ritorna len se OK, -1 se raw->len < 3.
 *
 * NOTA:
 *  - llc->payload parte dal byte successivo ai 3 byte LLC.
 */
int parse_llc(payload_t *raw, llc_pdu_t *llc);

/* ---------------- PARSING SNAP ------------------------------------------ */
/**
 * Decodifica un header SNAP all'inizio di raw.
 *  - raw  : deve iniziare dal primo byte OUI
 *  - snap : header SNAP + payload interno
 *
 * Ritorna 0 se OK, -1 se raw->len < 5.
 *
 * NOTA:
 *  - snap->payload parte dal byte successivo all'header SNAP (5 byte).
 */
int parse_snap(payload_t *raw, snap_pdu_t *snap);



/* =========================================================================
 *  FUNZIONI DI SUPPORTO 
 * ========================================================================= */

/**
 * Stampa un indirizzo MAC nel formato configurabile.
 *
 * @param mac          Puntatore all'indirizzo MAC da stampare.
 * @param separator    Carattere usato tra i byte (es. ':' '-' ' ').
 * @param decode       Se diverso da 0, mostra anche una decodifica:
 *                      - Unicast / Multicast
 *                      - Universale / Locale
 *                      - (opzionale) Nome produttore dall'OUI, se noto
 *
 * Esempio:
 *   print_mac(&mac, ':', 1);
 *   // Output: AA:BB:CC:DD:EE:FF  (Unicast, Universale, Intel Corp.)
 */
void print_mac(const mac_addr_t *mac, char separator, int decode);

/**
 * Stampa il riepilogo dell'header Ethernet.
 *
 * @param eth  Puntatore alla struttura eth_frame_t da stampare
 */
void print_eth_summary(const eth_frame_t *eth);

/**
 * Stampa il riepilogo del PDU LLC.
 *
 * @param llc  Puntatore alla struttura llc_pdu_t da stampare
 */
void print_llc_summary(const llc_pdu_t *llc);

/**
 * Stampa il riepilogo del PDU SNAP.
 *
 * @param snap  Puntatore alla struttura snap_pdu_t da stampare
 */
void print_snap_summary(const snap_pdu_t *snap);

#endif /* ETH_FRAME_H */
