/*
 * Test di connessione TTN usando un modulo Seeed LoRa-E5-HF
 * testato su Raspberry Pi Pico
 * maurizio.conti@fablabromagna.org  30/9/2023
 * 
 * Sorgente originale qui.
 * https://wiki.seeedstudio.com/Grove_Wio_E5_TTN_Demo/
 */

#include <Arduino.h>

// Se togli questo commento, si attivano delle Serial.print 
// in giro per il programma che aiutano a capirci qualcosa in più
// quando le cose non vanno...
//#define DEBUG_ON
 
static char recv_buf[512];
static bool is_exist = false;
static bool is_join = false;
static int led = 0;
 
static int at_send_check_response(char *p_ack, int timeout_ms, char *p_cmd, ...) {
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
        Serial.println( "Il ritorno (p_ack) non può essere nullo");
        return 0;
    }

    do {
        // Questo è il sistema migliore per leggere la seriale.
        // Un carattere alla volta, senza nessun delay(), fin che ce n'è!
        while ( Serial2.available() > 0 )
            recv_buf[index++] = Serial2.read();

        // Se dentro al messaggio trovi la frase di ack, tutto OK!
        if ( strstr(recv_buf, p_ack) != NULL ){        

#ifdef DEBUG_ON    
Serial.print( recv_buf );
#endif

            return 1;
        }
    // Alcuni messaggi vengono dati a più mandate, non basta leggere una stringa
    // serve tempo... poi però se il tempo è troppo, bisogna tornare  
    } while (millis() - startMillis < timeout_ms);
    
    // Se sei qui, sei in timeout
    Serial.print("mi aspettavo: ");
    Serial.println( p_ack);
    Serial.println("è arrivato: ");
    Serial.println( recv_buf );
    return 0;
}
 
// Decodifica il messaggio di ritorno dal network server
static void recv_prase(char *p_msg) {
    
    if (p_msg == NULL)
        return;

    char *p_start = NULL;
    int data=0, rssi=0, snr=0;

#ifdef DEBUG_ON    
Serial.println(p_msg);
#endif

    // TTN, ad ogni chiamata, risponde con un pacchetto i valori di RSSI e SN
    // che ha raccolto direttamente dal gateway aggiungendo un payload nostro (se vogliamo)

    // Ad esempio:
    // se nella dashboard TTN relativa all' "End device" clicchi il pulsante 
    // "Messagging -> Downlink -> Schedule Downlink"
    // 
    // il payload che hai messo arriva qui con un preambolo RX:
    p_start = strstr(p_msg, "RX");
    if (p_start && (1 == sscanf(p_start, "RX: \"%d\"\r\n", &data)))
    {
        // in "data" arriva il payload inserito nella dashboard TTN...
        Serial.println( "Payload: " + String(data) );

        if (data)
            digitalWrite(LED_BUILTIN, HIGH);
        else
            digitalWrite(LED_BUILTIN, LOW);
    }

    // qui decodifichiamo RSSI (potenza del segnale)
    p_start = strstr(p_msg, "RSSI");
    if (p_start && (1 == sscanf(p_start, "RSSI %d,", &rssi)))
        Serial.println( "rssi: " + String(rssi) );
    
    // qui decodifichiamo SNR (rapporto segnale/rumore)
    p_start = strstr(p_msg, "SNR");
    if (p_start && (1 == sscanf(p_start, "SNR %d", &snr)))
        Serial.println( "snr: " + String(snr) );
}

void initialize_LoRaModule()
{
    Serial.println("Inizio config Modulo...");
    
    if (at_send_check_response((char*)"+AT: OK", 300, (char*)"AT\r\n"))
    {
        is_exist = true;

        at_send_check_response((char*)"+DR", 2000, (char*)"AT+DR\r\n");
        at_send_check_response((char*)"+ID:", 1000, (char*)"AT+ID\r\n");
        at_send_check_response((char*)"+MODE: LWOTAA", 1000, (char*)"AT+MODE=LWOTAA\r\n");
        at_send_check_response((char*)"+DR:", 1000, (char*)"AT+DR=EU868\r\n");
        at_send_check_response((char*)"+CH: NUM,", 1000, (char*)"AT+CH=NUM,0-2\r\n");
        at_send_check_response((char*)"+CLASS: A", 1000, (char*)"AT+CLASS=A\r\n");
        at_send_check_response((char*)"+PORT: 8", 1000, (char*)"AT+PORT=8\r\n");
        at_send_check_response((char*)"+LW: LEN,",  1000, (char*)"AT+LW=LEN\r\n");
        at_send_check_response((char*)"+LW: VER,",  1000, (char*)"AT+LW=VER\r\n");
        
        // Primo sensore fatto in TTN
        //at_send_check_response((char*)"+KEY: APPKEY ABE0", 1000, (char*)"AT+KEY=APPKEY,\"ABE041D8AE25BC63012B29A2E486755 9\"\r\n");
        //at_send_check_response((char*)"ID: DevEui, 70:B3", 1000, (char*)"AT+ID=DevEui,\"70B3D57ED00616CB\"\r\n");
        //at_send_check_response((char*)"ID: AppEui, 12:34", 1000, (char*)"AT+ID=AppEui,\"1234567890ABCDEF\"\r\n");

        // Secondo sensore fatto in TTN
        // -------------------------------------------------------------------- attento, lo spazio non serve ----------\
        at_send_check_response((char*)"+KEY: APPKEY 3947", 1000, (char*)"AT+KEY=APPKEY,\"3947B8A55FBDE2B395C34E612B1BC45 A\"\r\n");
        at_send_check_response((char*)"ID: DevEui, 70:B3", 1000, (char*)"AT+ID=DevEui,\"70B3D57ED00616F6\"\r\n");
        at_send_check_response((char*)"ID: AppEui, 12:34", 1000, (char*)"AT+ID=AppEui,\"1234567890ABCDEF\"\r\n");

        delay(200);
    }
    else
    {
        is_exist = false;
        Serial.print("No E5 module found.\r\n");
        delay(10000);
    }
}

void setup(void) {
    
    // Dammi il tempo di accendere il monito seriale...
    delay(3000);
 
    // Nel Raspberry Pico ci sono due seriali.
    // UART0 (Serial) è collegata alla USB verso il PC
    Serial.begin(115200);
    Serial.print("FabLab Romagna E5 LORAWAN TEST on The Things Network\r\n");

    // UART1 (Serial2) è collegata al nostro modulo sui PIN 4 e 5
    Serial2.setTX(4);
    Serial2.setRX(5);
    Serial2.begin(9600);

    initialize_LoRaModule();
    
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH);
}
 
void loop(void){

    float temp = 27, humi = 65;
    int ok = 0;
    
    if (is_exist) {
        
        if (!is_join)
        {
            Serial.println( "Modulo configurato." );
            Serial.println( "Tento un JOIN..." );
            ok = at_send_check_response((char*)"+JOIN: Network joined", 12000, (char*)"AT+JOIN\r\n");
            if (ok)
            {
                is_join = true;
                Serial.println( "JOIN riuscito.");
            }
            else
            {
                is_join = false;
                Serial.println( "...JOIN non riuscito!\r\n" );
                delay(5000);
            }
        }
        else
        {
            Serial.println("Spedisco i dati...");
            Serial.println("Humidity: " + String(humi) + ", Temperature: " + String(temp) + " *C");
            Serial.println("");
            
            char cmd[128];
            sprintf(cmd, "AT+CMSGHEX=\"%04X%04X\"\r\n", (int)temp, (int)humi);

            ok = at_send_check_response( (char*)"+CMSGHEX: Done", 12000, cmd );
            if (ok) {
                Serial.println("Ricevo dal server");
                recv_prase(recv_buf);
                Serial.println("");
            }
            else
                Serial.print("Spedizione del messaggio fallita...\r\n\r\n");

            delay(10000);
        }
    }
    else
    {
        Serial.println( "LoRa-E5 not found..." );
        delay(1000);
    }
}
