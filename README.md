# usb-control

Pre jej spustenie je potrebné:
1. nainštalovať knižnicu potrebnú pre SQLite databázu, napríklad pomocou
   sudo dnf install libsqlite3x-devel
    prípadne
   sudo apt-get install libsqlite3-dev
    (podľa operačného systému)

2. preložiť zdrojový kód pomocou príkazu "make"

3. vytvoriť vlastnú databázu pravidiel, ktoré sa môžu skladať z maximálne 8 atribútov
   ./rules -a *   - pridá nové pravidlo
   ./rules -s     - zobrazí všetky pravidlá
   ./rules -x id  - vymaže pravidlo z databázy s konkrétnym ID pravidla

    príklad pravidiel môže vyzerať napríklad takto:
   ./rules -a -i 08 -o 3                       - na USB porte číslo 3 je možné pripojiť USB flash disk
   ./rules -a -d 00 -e 00 -i 03 -u 01 -o 1     - na USB porte číslo 1 je možné pripojiť HID zariadenie (najčastejšie myš, klávesnica)

4. spustenie samotnej aplikácie pomocou
   sudo ./usb-control