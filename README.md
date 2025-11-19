# MACroscope

Parser di frame ethernet di livello 2

## Richiesta

Analizzare i frame ethernet e restituire informazioni relative come MAC sorgente, destinatario e EthType.

Leggere da tastiera un frame Ethernet II in esadecimale, verificarne la validità e popolare una struct con:

- MAC destinazione (6 byte),
- MAC sorgente (6 byte),
- EtherType (2 byte, big-endian),
- payload (0..1500 byte).

Stampare poi una vista formattata dei campi.

**(Opzionale)**: se passato un nome file da riga di comando, leggere la stringa esadecimale da file .txt

### Struttura del frame Ethernet II

```
+---------------+----------------+---------------+---------------------+------------+
| Dst MAC (6B)  | Src MAC (6B)   | EtherType (2) | Payload (0..1500 B) | CRC (4B)   |
+---------------+----------------+---------------+---------------------+------------+
byte 0..5       byte 6..11        12..13          14..(n-1)               opzionale
```

Esempi di EtherType:
- `0x0800` IPv4
- `0x0806` ARP
- `0x0842` Wake-ON-LAN
- `0x8100` VLAN
- `0x86DD` IPv6

### Requisiti minimi

- **Input**: stringa di byte esadecimali da tastiera, ammessi spazi o contigui (es. `FF FF FF FF FF FF 00 11 22 33 44 55 08 06 ...` oppure `FFFFFFFFFFFF0011223344550806...`).
- **Validazione**:
    - numero di nibble pari;
    - almeno 14 byte totali;
    - payload ≤ 1500 byte (quindi len ≥ 14 e len-14 ≤ 1500).
- **Parsing**: estrarre campi come da schema "Struttura Frame Ethernet II" (EtherType è big-endian)

### Esempi di input (per test)

**ARP (broadcast, EtherType 0x0806)**
```
FF FF FF FF FF FF 00 11 22 33 44 55 08 06 00 01 08 00 06 04 00 01 00 11 22 33 44 55 C0 A8 01 0A 00 00 00 00 00 00 C0 A8 01 01
```

**IPv4 (0x0800)**
```
AA BB CC DD EE FF 00 11 22 33 44 55 08 00 45 00 00 14 00 00 40 00 40 06 00 00 C0 A8 01 0A C0 A8 01 01
```

...fatevi generare altri frame da LLM

#### Esempio di output:

```plaintext
Inserisci frame in esadecimale (byte separati da spazi o contigui):
FF FF FF FF FF FF 00 11 22 33 44 55 08 06 00 01 08 00 06 04 00 01 00 11 22 33 44 55 C0 A8 01 0A 00 00 00 00 00 00 C0 A8 01 01

=== FRAME ETHERNET DECODIFICATO ===
Destinazione : FF:FF:FF:FF:FF:FF  (Broadcast) 
Sorgente     : 00:11:22:33:44:55  (gruppo universale)
EtherType    : 0x0806 (ARP)
Payload size : 28 bytes

Payload (esadecimale/ASCII):
00 01 08 00 06 04 00 01 00 11 22 33 44 55 C0 A8 01 0A
00 00 00 00 00 00 C0 A8 01 01
---------------------------------
Frame totale: 42 byte
```

## Come usarlo

Compilare ed eseguire il programma su sistemi Unix-like:
```bash
make
./macroscope
```

Oppure su Windows:
```powershell
C:\Users\admin> macroscope.exe
```

## Licenza

MIT