# MACroscope

Parser di frame ethernet di livello 2 - **FASE 2 Completata**


### Struttura dei file

```
eth_frame.h    - Tipi di dati e prototipi per parsing Ethernet/LLC/SNAP
eth_frame.c    - Implementazione funzioni parsing e stampa
tables.h       - Prototipi per lookup OUI/SAP/Ethertype
tables.c       - Tabelle statiche di lookup
main.c         - Programma principale
frames.txt     - File di test con frame di esempio
Makefile       - Build automation
```

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


#### Linux (nativo)
```bash
make              # Compila per Linux
make run          # Compila ed esegue con frames.txt
```

#### Windows (cross-compilation con MinGW)
```bash
make windows      # Compila macroscope.exe per Windows
```

#### Entrambe le piattaforme
```bash
make all-platforms   # Compila sia Linux che Windows
```

### Pulizia
```bash
make clean        # Rimuove tutti gli eseguibili (Linux + Windows)
make win-clean    # Rimuove solo l'eseguibile Windows
```

### Esecuzione

#### Su Linux
```bash
# Con file di test
./macroscope frames.txt

# Da input tastiera
./macroscope

# Con help
./macroscope --help

# Con pipe
echo "AA BB CC DD EE FF 00 11 22 33 44 55 08 00 ..." | ./macroscope
```

#### Su Windows
```cmd
REM Con file di test
macroscope.exe frames.txt

REM Da input tastiera
macroscope.exe

REM Con help
macroscope.exe --help
```

### Requisiti per cross-compilation Windows

Per compilare l'eseguibile Windows su Linux è necessario MinGW:

**Debian/Ubuntu:**
```bash
sudo apt install mingw-w64
```

**Arch Linux:**
```bash
sudo pacman -S mingw-w64-gcc
```

**Fedora:**
```bash
sudo dnf install mingw64-gcc
```

## Funzionalità FASE 2

✅ Lettura frame da file di testo o stdin  
✅ Conversione stringa esadecimale → byte buffer  
✅ Parsing header Ethernet (MAC dst/src, Ethertype)  
✅ Decodifica MAC con rilevamento Broadcast/Multicast/Unicast  
✅ Lookup OUI per identificazione produttore  
✅ Riconoscimento tipo frame (Ethernet II / 802.3 LLC / 802.3 SNAP)  
✅ Parsing LLC (DSAP, SSAP, Control)  
✅ Parsing SNAP (OUI, Protocol ID)  
✅ Lookup SAP per protocolli LLC  
✅ Lookup Ethertype per protocollo incapsulato  
✅ Gestione commenti e righe vuote nei file di input  
✅ Interfaccia CLI con help (`-h`, `--help`)  
✅ Compilazione per Linux e Windows (cross-platform)  

## Tipi di Frame Supportati

| Tipo | Identificazione | Descrizione |
|------|----------------|-------------|
| **Ethernet II** | Type/Length ≥ 0x0600 | Frame standard con Ethertype |
| **802.3 LLC** | Type/Length < 0x0600 | Frame IEEE 802.3 con LLC header |
| **802.3 SNAP** | LLC con DSAP=SSAP=0xAA | Frame 802.3 con SNAP encapsulation |

## File di Test

Il file `frames.txt` contiene 9 frame di esempio:
- 4 frame Ethernet II (ARP, IPv4, ICMP, IPv6)
- 2 frame 802.3 LLC (NetBIOS, Spanning Tree)
- 3 frame 802.3 SNAP (IPv4, ARP, AppleTalk)

## Licenza

MIT
