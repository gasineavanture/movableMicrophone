## Seeed Studio XIAO ESP32S3

Seeed Studio XIAO ESP32S3 je majhen mikrokrmilnik, ki ga lahko uporabimo za upravljanje našega višinsko nastavljivega stojala za mikrofon. Ima dvojedrni procesor s frekvenco do 240 MHz ter podpira Wi-Fi in Bluetooth Low Energy (BLE) 5.0.

### Bluetooth Low Energy (BLE)

Ena izmed glavnih prednosti tega mikrokrmilnika je podpora za Bluetooth Low Energy 5.0, ki omogoča brezžično komunikacijo z nizko porabo energije.

Doseg Bluetooth povezave je odvisen od antene, ovir in okolja. ESP32S3 podpira tudi Bluetooth Long Range, vendar proizvajalec ne zagotavlja ene same vrednosti dosega za vse pogoje uporabe.

Seeed Studio pri določenih kompletih z zunanjo anteno navaja doseg več kot 100 metrov. Dejanski doseg našega stojala bomo morali preizkusiti, predvsem zato, ker bo elektronika skrita v notranjosti stojala.

### Upravljanje preko mobilne aplikacije

Za upravljanje stojala lahko izdelamo svojo mobilno aplikacijo, ki se bo preko Bluetooth Low Energy povezala neposredno z mikrokrmilnikom ESP32S3.

Aplikacija bi omogočala:

* Dvigovanje in spuščanje mikrofona.
* Nastavitev mikrofona na določeno višino.
* Shranjevanje vnaprej določenih višin.
* Prikaz trenutne višine stojala.

ESP32S3 bi sprejemal ukaze iz aplikacije in na njihovi podlagi krmilil motor, ki skrbi za nastavljanje višine mikrofona.

Za takšno upravljanje ne potrebujemo internetne povezave, saj lahko aplikacija in mikrokrmilnik komunicirata neposredno preko Bluetooth povezave.



Links:
https://wiki.seeedstudio.com/xiao_esp32s3_bluetooth/#bluetooth-low-energy-ble-usage
