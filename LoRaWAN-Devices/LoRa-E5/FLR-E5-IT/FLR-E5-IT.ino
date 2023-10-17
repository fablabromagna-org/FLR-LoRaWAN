/* LoRaWAN-L5-Device v1.1
 *
 * Test di connessione dispositivo LoRaWAN usando un modulo Seeed LoRa-E5-HF
 * testato su Raspberry Pi Pico
 *
 *
 * codice derivato dai sorgenti di Maurizio - maurizio.conti@fablabromagna.org  30/9/2023
 * https://github.com/fablabromagna-org/FLR-LoRaWAN/tree/main/LoRaWAN-Devices/LoRa-E5/FLR-TTN-E5
 *
 *
 * Sorgente originale qui.
 * https://wiki.seeedstudio.com/Grove_Wio_E5_TTN_Demo/
 *
 *
 * Rispetto ai sorgeti di Maurizio sono state apportate le seguenti modifiche:
 *
 * - utilizzo di 2 led esterni su grove (verde-D16 / blu-D18)
 * - utilizzo di un sensore grove di temperatura su A0 (poco preciso, ma per i test iniziali va bene)
 * - utilizzo di un potenziometro grove per simulare il valore di umidità %
 * - implementazione iniziale del livello di batteria, per ora generata random
 * - parsing, nel messaggio da NS, anche del valore di fPort
 * - disattivazione del led verde quando il payload da NS vale 0, e attivazione in tutti gli altri casi
 *
 * inoltre sono stati rimossi tutti i riferimenti a TTN in quando i test sono stati fatti registrando un device su un NS ChirpStack, ma di fatto tutto è indipendente dal NS utilizzato
 *
 * NOTE:
 * - utilizzando per ora un PICO W il BUILTIN_LED non sembra funzionare !
 * - il pacchetto inviato al NS è ora costituito da 6byte, con la temperatura inviata in decimi di gradi celsius
 * - ci sono alcune verifiche da completare per l'init del modulo e per la determinazione del tipo di messaggio inviato, che per ora è Confirmed
 *
 */

#include <Arduino.h>

// Se togli questo commento, si attivano delle Serial.print
// in giro per il programma che aiutano a capirci qualcosa in più
// quando le cose non vanno...
#define DEBUG_ON

static char recv_buf[512];
static bool is_exist = false;
static bool is_join = false;
static int led = 0;

const int B = 4275; // B value of the thermistor
const int R0 = 100000; // R0 = 100k
const int pinTempSensor = A0; // Grove - Temperature Sensor connected to A0

const int pinHumiSensor = A1; // Grove - Humidity Sensor connected to A1

const int pinGreenLed = D16; // Grove - green led connected to D16
const int pinBlueLed = D18; // Grove -  blur led connected to D18

int led_status = LOW;

static int at_send_check_response(char* p_ack, int timeout_ms, char* p_cmd, ...)
{
    int ch;
    int num = 0;
    int index = 0;
    int startMillis = 0;

    va_list args;
    memset(recv_buf, 0, sizeof(recv_buf));

    va_start(args, p_cmd);
    Serial2.printf(p_cmd, args);

#ifdef DEBUG_ON
    Serial.printf(p_cmd, args);
#endif

    va_end(args);

    delay(200);
    startMillis = millis();

    if (p_ack == NULL) {
        Serial.println("Il ritorno (p_ack) non può essere nullo");
        return 0;
    }

    do {
        // Questo è il sistema migliore per leggere la seriale.
        // Un carattere alla volta, senza nessun delay(), fin che ce n'è!
        while (Serial2.available() > 0)
            recv_buf[index++] = Serial2.read();

        // Se dentro al messaggio trovi la frase di ack, tutto OK!
        if (strstr(recv_buf, p_ack) != NULL) {

#ifdef DEBUG_ON
            Serial.print(recv_buf);
#endif

            return 1;
        }
        // Alcuni messaggi vengono dati a più mandate, non basta leggere una stringa
        // serve tempo... poi però se il tempo è troppo, bisogna tornare
    } while (millis() - startMillis < timeout_ms);

    // Se sei qui, sei in timeout
    Serial.print("mi aspettavo: ");
    Serial.println(p_ack);
    Serial.println("è arrivato: ");
    Serial.println(recv_buf);
    return 0;
}

// Decodifica il messaggio di ritorno dal network server
static void recv_parse(char* p_msg)
{

    if (p_msg == NULL)
        return;

    char* p_start = NULL;
    int data = 0, rssi = 0, snr = 0, fport;

#ifdef DEBUG_ON
    Serial.println("----------- RX MSG ----------------");
    Serial.println(p_msg);
    Serial.println("-----------------------------------");
#endif

    // Il modulo L5, ad ogni trasmissione, risponde con un pacchetto i valori di RSSI e SN
    // che ha raccolto direttamente dal gateway aggiungendo.
    // Inoltre nella risposta può essere presente un payload inviato dal NetworkServer
    // L'invio del payload può essere Confirmed o Unconfirmed. Nel primo caso il modulo L5 si occupa diretamente di inviare un ACK al NS

    // Ad esempio:
    // se nella dashboard TTN relativa all' "End device" clicchi il pulsante
    // "Messagging -> Downlink -> Schedule Downlink"
    //
    // La stessa cosa nella dashboard di ChirpStack, nei dettagli del sensore, al tag "Queue"
    //
    // il payload ricevuto arriva qui con un preambolo RX: mentre la fPort con preambolo PORT:
    //
    // Attenzione! il payload viene restituito dal modulo L5 come stringa. Quindi anche un valore esadecimale viene restituito come stringa e quindi il parsing deve tenerne conto.

    // qui decodifichiamo la porta del messaggio RX, se presente
    p_start = strstr(p_msg, "PORT");
    if (p_start && (1 == sscanf(p_start, "PORT: %d ", &fport))) {
        // in "data" arriva il payload inserito nella dashboard TTN...
        Serial.println("fPort: " + String(fport));
    }

    p_start = strstr(p_msg, "RX");
    if (p_start && (1 == sscanf(p_start, "RX: \"%d\"\r\n", &data))) {
        // in "data" arriva il payload inviato dal NetworkServer
        Serial.println("Payload: " + String(data));

        if (data == 0)
            led_status = LOW;
        else
            led_status = HIGH;

        digitalWrite(pinGreenLed, led_status);
    }

    // qui decodifichiamo RSSI (potenza del segnale)
    p_start = strstr(p_msg, "RSSI");
    if (p_start && (1 == sscanf(p_start, "RSSI %d,", &rssi)))
        Serial.println("rssi: " + String(rssi));

    // qui decodifichiamo SNR (rapporto segnale/rumore)
    p_start = strstr(p_msg, "SNR");
    if (p_start && (1 == sscanf(p_start, "SNR %d", &snr)))
        Serial.println("snr: " + String(snr));
}

void initialize_LoRaModule()
{
    Serial.println("Inizio config Modulo...");

    if (at_send_check_response((char*)"+AT: OK", 300, (char*)"AT\r\n")) {
        is_exist = true;

        at_send_check_response((char*)"+DR", 2000, (char*)"AT+DR\r\n");
        at_send_check_response((char*)"+ID:", 1000, (char*)"AT+ID\r\n");
        at_send_check_response((char*)"+MODE: LWOTAA", 1000, (char*)"AT+MODE=LWOTAA\r\n");
        at_send_check_response((char*)"+DR:", 1000, (char*)"AT+DR=EU868\r\n");
        at_send_check_response((char*)"+CH: NUM,", 1000, (char*)"AT+CH=NUM,0-2\r\n");
        at_send_check_response((char*)"+CLASS: A", 1000, (char*)"AT+CLASS=A\r\n");
        at_send_check_response((char*)"+PORT: 8", 1000, (char*)"AT+PORT=8\r\n");
        at_send_check_response((char*)"+LW: LEN,", 1000, (char*)"AT+LW=LEN\r\n");
        at_send_check_response((char*)"+LW: VER,", 1000, (char*)"AT+LW=VER\r\n");

        // Dati identificativi del sensore e dell'applicazione
        // -------------------------------------------------------------------- attento, lo spazio non serve ----------
        at_send_check_response((char*)"+KEY: APPKEY 98FB", 1000, (char*)"AT+KEY=APPKEY,\"98fb76b567c9f6d20baa16e07990caac\"\r\n");
        at_send_check_response((char*)"ID: DevEui, FC:A7", 1000, (char*)"AT+ID=DevEui,\"fca76b9eb3acf647\"\r\n");
        at_send_check_response((char*)"ID: AppEui, 7B:9A", 1000, (char*)"AT+ID=AppEui,\"7b9a65aa20e645319b1298c03a31bc3b\"\r\n");

        delay(200);
    } else {
        is_exist = false;
        Serial.print("No E5 module found.\r\n");
        delay(10000);
    }
}

void setup(void)
{

    // Dammi il tempo di accendere il monito seriale...
    delay(3000);

    // Nel Raspberry Pico ci sono due seriali.
    // UART0 (Serial) è collegata alla USB verso il PC
    Serial.begin(115200);
    Serial.print("FabLab Romagna E5 LORAWAN TEST - FLR Sensor IT1\r\n");

    // UART1 (Serial2) è collegata al nostro modulo sui PIN 4 e 5
    Serial2.setTX(4);
    Serial2.setRX(5);
    Serial2.begin(9600);

    initialize_LoRaModule();

    pinMode(D25, OUTPUT);
    pinMode(pinGreenLed, OUTPUT);
    pinMode(pinBlueLed, OUTPUT);

    digitalWrite(D25, HIGH);
    digitalWrite(pinGreenLed, HIGH);
    // digitalWrite(pinBlueLed, HIGH);
}

void loop(void)
{

    int a = analogRead(pinTempSensor);
    float R = 1023.0 / a - 1.0;
    R = R0 * R;

    float temp = 1.0 / (log(R / R0) / B + 1 / 298.15) - 273.15; // convert to temperature via datasheet

    float humi = map(analogRead(pinHumiSensor), 0, 1023, 0, 100);

    uint8_t battery = map(random(10), 0, 10, 30, 100);

    int ok = 0;

    if (is_exist) {
        if (!is_join) {
            Serial.println("Modulo configurato.");
            Serial.println("Tento un JOIN...");
            ok = at_send_check_response((char*)"+JOIN: Network joined", 12000, (char*)"AT+JOIN\r\n");
            if (ok) {
                digitalWrite(pinGreenLed, LOW);
                is_join = true;
                Serial.println("JOIN riuscito.");
            } else {
                digitalWrite(pinGreenLed, HIGH);
                is_join = false;
                Serial.println("...JOIN non riuscito!\r\n");
                delay(5000);
            }
        } else {
            digitalWrite(pinBlueLed, HIGH);
            Serial.println("Spedisco i dati...");
            Serial.println("Humidity: " + String(humi) + ", Temperature: " + String(temp) + " *C, Battery: " + String(battery));
            Serial.println("");

            char cmd[128];
            sprintf(cmd, "AT+CMSGHEX=\"%04X%04X%02X%02X\"\r\n", (int)(temp * 10), (int)humi, battery, led_status);

            ok = at_send_check_response((char*)"+CMSGHEX: Done", 12000, cmd);
            if (ok) {
                Serial.println("\nRicevo dal server");
                recv_parse(recv_buf);
                Serial.println("");
            } else
                Serial.print("Spedizione del messaggio fallita...\r\n\r\n");

            digitalWrite(pinBlueLed, LOW);

            delay(10000);
        }
    } else {
        Serial.println("LoRa-E5 not found...");
        delay(1000);
    }
}
