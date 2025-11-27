/**
 * @file tables.c
 * @brief Lookup Table statiche per OUI (MAC vendor), SAP (LLC) ed Ethertype
 * @author Rug (https://github.com/dugolott) // fenomeno
 * @version 1.0
 * @date 2025-11-20
 * 
 * Tabelle di lookup per decodificare:
 * - OUI (primi 3 byte del MAC) → nome produttore
 * - SAP (Service Access Point) → protocollo LLC
 * - Ethertype → protocollo livello superiore
 */

#include <stdint.h>
#include "tables.h"
#include "eth_frame.h"
#include <stddef.h>
#include <string.h>

/* -------------------------------------------------------------------------
 *  STRUTTURE DATI INTERNE
 * ------------------------------------------------------------------------- */

/* Entry per la tabella OUI → produttore */
typedef struct {
    uint32_t oui;          /* 24 bit più significativi del MAC */
    const char *vendor;    /* Nome produttore */
} oui_entry_t;

/* -------------------------------------------------------------------------
 *  LUT OUI (valori reali, subset significativo)
 * ------------------------------------------------------------------------- */

static const oui_entry_t oui_table[] = {
    {0x000C29, "VMware, Inc."},
    {0x3C5A37, "Apple, Inc."},
    {0x00163E, "Cisco Systems, Inc."},
    {0xF0DE71, "Intel Corporate"},
    {0xB827EB, "Raspberry Pi Foundation"},
    {0xD850E6, "Samsung Electronics"},
    {0x001E06, "Hewlett-Packard"},
    {0x00155D, "Dell Inc."},
    {0x000D93, "Hon Hai Precision (Foxconn)"},
    {0xF4F5A5, "Xiaomi Communications"},
    {0x70E284, "Google, Inc."},
    {0xBC305B, "ASUSTek Computer Inc."},
    {0x0013EF, "Sony Corporation"},
    {0x0009B0, "LG Electronics"},
    {0xC83A35, "Lenovo Mobile Communication"},
    {0x001122, "CIMSYS Inc"},
    {0xAABBCC, "Unknown Vendor"},
};

static const size_t oui_table_count = sizeof(oui_table) / sizeof(oui_table[0]);

/* -------------------------------------------------------------------------
 *  LUT SAP (Logical Link Control – DSAP/SSAP)
 *  Implementata come array diretto indicizzato da codice SAP (0–255)
 * ------------------------------------------------------------------------- */

static const char *sap_table[256] = {
    [0x00] = "Null LSAP",
    [0x02] = "LLC Sub-layer Management",
    [0x04] = "IBM SNA Path Control",
    [0x06] = "DOD IP",
    [0x08] = "PROWAY-LAN",
    [0x0E] = "PROWAY-LAN (IEC 955)",
    [0x42] = "IEEE 802.1 Bridge Spanning Tree",
    [0xAA] = "SNAP (SubNetwork Access Protocol)",
    [0xE0] = "NetBIOS",
    [0xF0] = "IBM NETBIOS",
    [0xFE] = "ISO Network Layer",
};

static const size_t sap_table_count = 256;

/* -------------------------------------------------------------------------
 *  LUT ETHERTYPE 
 *  Implementata come ...
 * ------------------------------------------------------------------------- */

static const proto_desc_t proto_table[] = {
    {0x0800, "IPv4"},
    {0x0806, "ARP"},
    {0x0842, "Wake ON LAN"},
    {0x22F3, "TRILL"},
    {0x6003, "DECnet"},
    {0x8035, "RARP"},
    {0x8100, "802.1Q VLAN"},
    {0x86DD, "IPv6"},
    {0x8808, "Ethernet Flow Control"},
    {0x8809, "LACP"},
    {0x8847, "MPLS unicast"},
    {0x8848, "MPLS multicast"},
    {0x8863, "PPPoE Discovery"},
    {0x8864, "PPPoE Session"},
    {0x88B8, "PTP"},
    {0x88CC, "LLDP"},
    {0x88E5, "MACsec"},
};
static const size_t proto_count = sizeof(proto_table) / sizeof(proto_table[0]);

/* -------------------------------------------------------------------------
 *  FUNZIONI PUBBLICHE (da implementare)
 * ------------------------------------------------------------------------- */
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
const char *protocol_name(uint16_t value){
    for (size_t i = 0; i < proto_count; ++i) {
        if (proto_table[i].value == value) {
            return proto_table[i].name;
        }
    }
    return "Sconosciuto";
}
const char *oui_lookup(uint32_t oui)
{
    for (size_t i = 0; i < oui_table_count; ++i) {
        if (oui_table[i].oui == oui) {
            return oui_table[i].vendor;
        }
    }

    return "Sconosciuto";
}
const char *sap_lookup(uint8_t sap){
    (void)sap_table_count; /* tabelle dirette -> sz non usata ma mantengo var */
    const char *name = sap_table[sap];
    return name ? name : "Sconosciuto";
}
