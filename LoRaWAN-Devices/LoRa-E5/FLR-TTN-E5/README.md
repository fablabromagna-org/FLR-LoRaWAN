# Connessione di un modulo Seeed LoRa-E5-HF alla rete TTN

Le basi per questo esempio sono state prese dalla guida ufficiale Seeed
https://wiki.seeedstudio.com/Grove_Wio_E5_TTN_Demo/

Non avendo a disposizione ne il display ne il sensore DHT però, il sorgente è stato snellito, ripulito di alcune sbavature e lasciato il più semplice ed essenziale possibile.

La dashboard TTN

![image](https://github.com/fablabromagna-org/FLR-LoRaWAN/assets/249618/fc44adf3-01bb-4199-a116-c5745a744689)

Un sessione dati...

![image](https://github.com/fablabromagna-org/FLR-LoRaWAN/assets/249618/a55b53db-4a4b-42bd-ba25-49d0635defde)


Esempio di Payload formatter

![image](https://github.com/fablabromagna-org/FLR-LoRaWAN/assets/249618/322b6feb-d21e-459c-97ff-b6921d75d4dc)

Esempio di Payload spedito all'End Device

![image](https://github.com/fablabromagna-org/FLR-LoRaWAN/assets/249618/ec2e3f80-5614-47fe-b3f8-3da0660f1f65)

Lo schema elettrico
![image](https://github.com/fablabromagna-org/FLR-LoRaWAN/assets/249618/4b705cdf-7c39-42c9-a603-334af2540f9b)

Particolare sul fatto che il modulo è collegato sui GPIO 4 e 5 
![image](https://github.com/fablabromagna-org/FLR-LoRaWAN/assets/249618/60265ebf-1c62-4518-9403-9d619087eed4)

I dati dalla seriale del Raspberry Pi Pico

![image](https://github.com/fablabromagna-org/FLR-LoRaWAN/assets/249618/a0abb570-a6c9-4f2d-a9ba-5c401d1b484b)

Questo è invece il nostro LoRa Network server ChirpStack che mostra il Gateway FLR posizionato a San Martino dei Mulini...

![image](https://github.com/fablabromagna-org/FLR-LoRaWAN/assets/249618/cc4add61-9769-4f42-9e2c-5950dada1b3f)

e anche su ChirpStack il sensore si comporta bene

![image](https://github.com/fablabromagna-org/FLR-LoRaWAN/assets/249618/00c9b894-77ff-4829-b934-e42811c4cd0b)

Anche sul nostro server si possono inviare dati verso il sensore

![image](https://github.com/fablabromagna-org/FLR-LoRaWAN/assets/249618/a914829a-a11c-44e9-9d3f-0d3a296f5c63)

Che arrivano allo stesso modo di TTN

![image](https://github.com/fablabromagna-org/FLR-LoRaWAN/assets/249618/76b074f6-b0cf-407d-affb-43c462479fa6)
