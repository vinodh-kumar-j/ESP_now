#include <Arduino.h>

#ifdef ESP32
#include <WiFi.h>
#else
#include <ESP8266WiFi.h>
#endif
#include "AudioFileSourceICYStream.h"
#include "AudioFileSourceBuffer.h"
#include "AudioGeneratorMP3.h"
#include "AudioOutputI2SNoDAC.h"

// To run, set your ESP8266 build to 160MHz, update the SSID info, and upload.

// Enter your WiFi setup here:
const char *SSID = "Vinu";
const char *PASSWORD = "123456788";

// Uncomment one link (I have added 6 radio streaming link, you can check each)

//flawlessly working radio streaming link
//const char *URL="http://ndr-edge-206c.fra-lg.cdn.addradio.net/ndr/njoy/live/mp3/128/stream.mp3"; //'N-JOY vom NDR - www.n-joy.de'
//const char *URL="http://psrlive2.listenon.in/anirud?station=anirudhradio&cb=1693414180";
const char *URL="http://d111.rndfnk.com/ard/ndr/ndr1niedersachsen/hannover/mp3/128/stream.mp3?aggregator=web&cid=01FCT9XYE3C7Y8087XEWPRC38Z&sid=2UiFNWM8DfzPGNNbW5TbBzQ41mb&token=fEjeJuHPqHmbYmSDN9wTdTb0gKtc-whcDdmZSWWvq2Q&tvf=nF3NlB5LgBdkMTExLnJuZGZuay5jb20";

// It will work but buffer alot
//const char *URL="http://stream.ca.morow.com:8003/morow_med.mp3";
//const char *URL= "http://ndr-ndr1radiomv-schwerin.sslcast.addradio.de/ndr/ndr1radiomv/schwerin/mp3/128/stream.mp3";
//const char *URL="http://mms.hoerradar.de:8000/rst128k";//Radio RST(German)


AudioGeneratorMP3 *mp3;
AudioFileSourceICYStream *file;
AudioFileSourceBuffer *buff;
AudioOutputI2SNoDAC *out;

// Called when a metadata event occurs (i.e. an ID3 tag, an ICY block, etc.
void MDCallback(void *cbData, const char *type, bool isUnicode, const char *string)
{
const char *ptr = reinterpret_cast<const char *>(cbData);
(void) isUnicode; // Punt this ball for now
// Note that the type and string may be in PROGMEM, so copy them to RAM for printf
char s1[32], s2[64];
strncpy_P(s1, type, sizeof(s1));
s1[sizeof(s1)-1]=0;
strncpy_P(s2, string, sizeof(s2));
s2[sizeof(s2)-1]=0;
Serial.printf("METADATA(%s) '%s' = '%s'\n", ptr, s1, s2);
Serial.flush();
}

// Called when there's a warning or error (like a buffer underflow or decode hiccup)
void StatusCallback(void *cbData, int code, const char *string)
{
const char *ptr = reinterpret_cast<const char *>(cbData);
// Note that the string may be in PROGMEM, so copy it to RAM for printf
char s1[64];
strncpy_P(s1, string, sizeof(s1));
s1[sizeof(s1)-1]=0;
Serial.printf("STATUS(%s) '%d' = '%s'\n", ptr, code, s1);
Serial.flush();
}


void setup()
{
Serial.begin(115200);
delay(1000);
Serial.println("Connecting to WiFi");

WiFi.disconnect();
WiFi.softAPdisconnect(true);
WiFi.mode(WIFI_STA);

WiFi.begin(SSID, PASSWORD);

// Try forever
while (WiFi.status() != WL_CONNECTED) {
Serial.println("...Connecting to WiFi");
delay(1000);
}
Serial.println("Connected");

audioLogger = &Serial;
file = new AudioFileSourceICYStream(URL);
file->RegisterMetadataCB(MDCallback, (void*)"ICY");
buff = new AudioFileSourceBuffer(file, 8192);
buff->RegisterStatusCB(StatusCallback, (void*)"buffer");
out = new AudioOutputI2SNoDAC();
mp3 = new AudioGeneratorMP3();
mp3->RegisterStatusCB(StatusCallback, (void*)"mp3");
mp3->begin(buff, out);
}

void loop()
{
static int lastms = 0;

if (mp3->isRunning()) {
if (millis()-lastms > 1000) {
lastms = millis();
Serial.printf("Running for %d ms...\n", lastms);
Serial.flush();
}
if (!mp3->loop()) mp3->stop();
} else {
Serial.printf("MP3 done\n");
delay(1000);
}
}