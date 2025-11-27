/**
 * @file tables.h
 * @brief Lookup Table per OUI, SAP ed Ethertype
 * @author Giordii (admin@giordii.dev)
 * @version 1.0
 * @date 2025-11-20
 */

#ifndef TABLES_H
#define TABLES_H

#include <stdint.h>

/* ---------------- TABELLA ETHERTYPE / SNAP ------------------------------ */
/**
 * Restituisce il nome del protocollo associato a un Ethertype / protocol_id.
 *  - value : Ethertype (Ethernet II) oppure protocol_id (SNAP)
 *
 * Ritorna una stringa costante (es. "IPv4", "ARP", "IPv6") se noto,
 * altrimenti "Sconosciuto".
 *
 * L'implementazione userà internamente un array statico di proto_desc_t.
 */
const char *protocol_name(uint16_t value);

/* ---------------- TABELLA OUI ------------------------------------------- */
/**
 * Ricerca del produttore OUI
 *  - oui : primi 24 bit del MAC address
 *
 * Ritorna il nome del produttore se noto, altrimenti "Sconosciuto".
 */
const char *oui_lookup(uint32_t oui);

/* ---------------- TABELLA SAP ------------------------------------------- */
/**
 * Ricerca del nome SAP (LLC)
 *  - sap : valore DSAP o SSAP
 *
 * Ritorna il nome del SAP se noto, altrimenti "Sconosciuto".
 */
const char *sap_lookup(uint8_t sap);

#endif /* TABLES_H */
