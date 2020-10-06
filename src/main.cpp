#include <Arduino.h>

const byte numChars = 32;
char receivedChars[numChars]; // an array to store the received data;
boolean newData = false;

void recvWithMarker();

void showNewData();

String ModRTU_CRC(String raw_msg_data);

byte StrtoByte (String str_value);


void setup() {
    Serial.begin(115200);
    Serial.println("Arduino is Ready");
}

void loop() {
    recvWithMarker();
    showNewData();
}

void recvWithMarker() {
    static byte ndx = 0;
    char endMarker = '\n';
    char rc;

    while (Serial.available() > 0 && !newData) {
        rc = Serial.read();

        if (rc != endMarker) {
            receivedChars[ndx] = rc;
            ndx++;
            if (ndx >= numChars) {
                ndx = numChars - 1;
            }
        } else {
            receivedChars[ndx] = '\0'; // terminate the string
            ndx = 0;
            newData = true;
        }
    }
}

void showNewData() {
    if (newData) {
        String raw_msg = receivedChars;
        String raw_msg_data = raw_msg.substring(0, raw_msg.length()-4); //Modbus message without crc code
        String raw_msg_crc = raw_msg.substring(raw_msg.length() - 4, raw_msg.length()); //only Modbus message crc code
        String calculated_crc = "";
        Serial.println("Modbus message: " + raw_msg);
        Serial.println("Modbus data: " + raw_msg_data);
        Serial.println("Modbus CRC: " + raw_msg_crc);
        calculated_crc = ModRTU_CRC(raw_msg_data); //Modbus message without crc code. Reply the calculated crc code
        Serial.println("---------------------------------");
        Serial.println("Modbus CRC Calculated: " + calculated_crc);


        Serial.print("This just in ... ");
        Serial.println(receivedChars);
        newData = false;
    }
}

// Compute the MODBUS RTU CRC
// Compute the MODBUS RTU CRC
String ModRTU_CRC(String raw_msg_data) {
    //Calc raw_msg_data length
    byte raw_msg_data_byte[raw_msg_data.length()/2];
    //Convert the raw_msg_data to a byte array raw_msg_data
    for (int i = 0; i < raw_msg_data.length() / 2; i++) {
        raw_msg_data_byte[i] = StrtoByte(raw_msg_data.substring(2 * i, 2 * i + 2));
    }

    //Calc the raw_msg_data_byte CRC code
    uint16_t crc = 0xFFFF;
    String crc_string = "";
    for (int pos = 0; pos < raw_msg_data.length()/2; pos++) {
        crc ^= (uint16_t)raw_msg_data_byte[pos];          // XOR byte into least sig. byte of crc
        for (int i = 8; i != 0; i--) {    // Loop over each bit
            if ((crc & 0x0001) != 0) {      // If the LSB is set
                crc >>= 1;                    // Shift right and XOR 0xA001
                crc ^= 0xA001;
            }
            else                            // Else LSB is not set
                crc >>= 1;                    // Just shift right
        }
    }
    // Note, this number has low and high bytes swapped, so use it accordingly (or swap bytes)

    //Become crc byte to a capital letter String
    crc_string = String(crc, HEX);
    crc_string.toUpperCase();

    //The crc should be like XXYY. Add zeros if need it
    if(crc_string.length() == 1){
        crc_string = "000" + crc_string;
    }else if(crc_string.length() == 2){
        crc_string = "00" + crc_string;
    }else if(crc_string.length() == 3){
        crc_string = "0" + crc_string;
    }else{
        //OK
    }

    //Invert the byte positions
    crc_string = crc_string.substring(2, 4) + crc_string.substring(0, 2);
    return crc_string;
}

//String to byte --> Example: String = "C4" --> byte = {0xC4}
byte StrtoByte (String str_value){
    char char_buff[3];
    str_value.toCharArray(char_buff, 3);
    byte byte_value = strtoul(char_buff, NULL, 16);
    return byte_value;
}