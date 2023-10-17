# Connessione di un modulo Seeed LoRa-E5-HF alla rete LoRaWAN

Test di connessione dispositivo LoRaWAN usando un modulo Seeed LoRa-E5-HF
testato su Raspberry Pi Pico


codice derivato dai sorgenti di Maurizio - maurizio.conti@fablabromagna.org  30/9/2023
 https://github.com/fablabromagna-org/FLR-LoRaWAN/tree/main/LoRaWAN-Devices/LoRa-E5/FLR-TTN-E5


Sorgente originale qui.
https://wiki.seeedstudio.com/Grove_Wio_E5_TTN_Demo/


## Note di revisione

### versione 1.1

Aggiunto nel payload inviato al NS un ulteriore byte che rappresenta lo stato del led verde, che è possibile comandare da remoto tramite comando di downlink.  In questo modo è possibile vedere se il comando è andato a buon fine, e tracciare comunque lo stato di accensione.
Da notare che, essendo il comando ricevuto dal device solo dopo aver trasmesso i propri dati (DEVICE A), viene applicato subito ma lo stato del led viene notificato solo al successivo invio

### versione 1.0

Rispetto ai sorgenti di Maurizio sono state apportate le seguenti modifiche:

- utilizzo di 2 led esterni su grove (verde-D16 / blu-D18)
- utilizzo di un sensore grove di temperatura su A0 (poco preciso, ma per i test iniziali va bene)
- utilizzo di un potenziometro grove per simulare il valore di umidità %
- implementazione iniziale del livello di batteria, per ora generata random
- parsing, nel messaggio da NS, anche del valore di fPort
- disattivazione del led verde quando il payload da NS vale 0, e attivazione in tutti gli altri casi

inoltre sono stati rimossi tutti i riferimenti a TTN in quando i test sono stati fatti registrando un device su un NS ChirpStack, ma di fatto tutto è indipendente dal NS utilizzato


> NOTE:
> - utilizzando per ora un PICO W il BUILTIN_LED non sembra funzionare !
> - il pacchetto inviato al NS è ora costituito da 6byte, con la temperatura inviata in decimi di gradi celsius
> - ci sono alcune verifiche da completare per l'init del modulo e per la determinazione del tipo di messaggio inviato, che per ora è Confirmed

