#include <Arduino.h>
#include <WiFiEspAT.h>
#include "HX711.h"
#include <SoftwareSerial.h>
#include <ESP_Mail_Client.h>

// define pin out
const int planetHumid = A0;
const int waterRate = A1;
const int LOADCELL_SCK_PIN = 6;
const int LOADCELL_DOUT_PIN = 7;
const int relay4plant = 8;
const int relay4drinking = 9;
SoftwareSerial esp01Serial(10, 11); // RX, TX

// global variable
int wifiState = WL_IDLE_STATUS; // WL_IDLE_STATUS => the Wifi radio's status
struct wifi_config
{
  char ssid[20] = "ssid";
  char pwd[20] = "password";
} wifiConfig;
HX711 scale;
bool cupLeft = true;
int load_cup = 0;
int load_cupAndWater = 0;
#define SMTP_HOST "smtp.gmail.com"
#define SMTP_PORT esp_mail_smtp_port_587
#define AUTHOR_EMAIL "sender mail address"
#define AUTHOR_PASSWORD "mail password"
#define RECIPIENT_EMAIL "receiver mail address"
SMTPSession smtp;
bool sentMailFlag = false;

// function prototype
void initWifi();
void initLoadCell();
void checkWifiState();
void checkCup();
void waterCup();
void waterPlant();
void waterRate();
void smtpCallback(SMTP_Status status);
void sendMail();

// test unit
void testLoadCell();
void testHumid();
void testWaterRate();

void setup()
{
  Serial.begin(115200);
  while (!Serial)
  {
  }

  pinMode(waterRate, INPUT);
  pinMode(planetHumid, INPUT);
  pinMode(relay4plant, OUTPUT);
  pinMode(relay4drinking, OUTPUT);
  digitalWrite(relay4plant, LOW);
  digitalWrite(relay4drinking, LOW);

  initLoadCell();

  /*  Set the network reconnection option */
  MailClient.networkReconnect(true);
}

void loop()
{
  checkWifiState();
  checkCup();
  waterPlant();
  waterRate();

  // testLoadCell();
  // testHumid();
  // testWaterRate();

  yield();
}

void initWifi()
{
  WiFi.init(esp01Serial);
  WiFi.setPersistent(); // set the following WiFi connection as persistent

  while (wifiState != WL_CONNECTED)
  {
    wifiState = WiFi.begin(wifiConfig.ssid, wifiConfig.pwd);
    Serial.println("\nwifi connecting");
    delay(500);
  }
  delay(1000);
}

void initLoadCell()
{
  const int scale_factor = -422; // 比例參數，從校正程式中取得

  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  scale.set_scale(scale_factor); // 設定比例參數
  scale.tare();                  // 歸零
  Serial.println("ok");
}
void checkWifiState()
{
  wifiState = WiFi.status();
  if (wifiState != WL_CONNECTED)
  {
    Serial.println("Wifi disconnected");
    // reconnct wifi
    while (wifiState != WL_CONNECTED)
    {
      wifiState = WiFi.begin(wifiConfig.ssid, wifiConfig.pwd);
      delay(500);
    }
    delay(1000);
  }
}

void checkCup()
{
  // take out cup
  if (cupLeft == false && scale.get_units(10) < 2)
  {
    cupLeft = true;
  }

  // put on cup
  if (cupLeft == true && scale.get_units(10) >= 20)
  {
    cupLeft = false;
    waterCup();
    Serial.println("Water rate low, Sending Mail");
  }
}

void waterCup()
{
  // load_cup = scale.get_units(10);
  // load_cupAndWater = load_cup + 150;
  // while (scale.get_units(10) < load_cupAndWater)
  // {
  //   digitalWrite(relay4drinking, HIGH);
  //   Serial.print("load:\t");
  //   Serial.println(scale.get_units(10));
  // }
  // digitalWrite(relay4drinking, LOW);
  digitalWrite(relay4drinking, HIGH);
  delay(12000);
  digitalWrite(relay4drinking, LOW);
}

void waterPlant()
{
  Serial.print("humid:\t");
  Serial.println(analogRead(planetHumid));
  if (analogRead(planetHumid) > 600) // todo
  {
    digitalWrite(relay4plant, HIGH);
    delay(2000);
    Serial.println("water plant finished");
  }

  digitalWrite(relay4plant, LOW);
}

void waterRate()
{
  if (digitalRead(waterRate))
  {
    sendMail();
  }
}

void testLoadCell()
{
  Serial.print("load:\t");
  Serial.println(scale.get_units(10));
}

void testHumid()
{
  Serial.print("humid:\t");
  Serial.println(analogRead(planetHumid));
}

void testWaterRate()
{
  Serial.print("rate:\t");
  Serial.println(digitalRead(waterRate));
  delay(200);
}

void sendMail()
{
  /** Enable the debug via Serial port
   * 0 for no debugging
   * 1 for basic level debugging
   *
   * Debug port can be changed via ESP_MAIL_DEFAULT_DEBUG_PORT in ESP_Mail_FS.h
   */
  smtp.debug(1);

  /* Set the callback function to get the sending results */
  smtp.callback(smtpCallback);

  /* Declare the Session_Config for user defined session credentials */
  Session_Config config;

  /* Set the session config */
  config.server.host_name = SMTP_HOST;
  config.server.port = SMTP_PORT;
  config.login.email = AUTHOR_EMAIL;
  config.login.password = AUTHOR_PASSWORD;

  /** Assign your host name or you public IPv4 or IPv6 only
   * as this is the part of EHLO/HELO command to identify the client system
   * to prevent connection rejection.
   * If host name or public IP is not available, ignore this or
   * use loopback address "127.0.0.1".
   *
   * Assign any text to this option may cause the connection rejection.
   */
  config.login.user_domain = F("127.0.0.1");

  /*
  Set the NTP config time
  For times east of the Prime Meridian use 0-12
  For times west of the Prime Meridian add 12 to the offset.
  Ex. American/Denver GMT would be -6. 6 + 12 = 18
  See https://en.wikipedia.org/wiki/Time_zone for a list of the GMT/UTC timezone offsets
  */
  config.time.ntp_server = F("time.stdtime.gov.tw");
  config.time.gmt_offset = 3;
  config.time.day_light_offset = 0;

  /* The full message sending logs can now save to file */
  /* Since v3.0.4, the sent logs stored in smtp.sendingResult will store only the latest message logs */
  // config.sentLogs.filename = "/path/to/log/file";
  // config.sentLogs.storage_type = esp_mail_file_storage_type_flash;

  /* Declare the message class */
  SMTP_Message message;

  /* Set the message headers */
  message.sender.name = F("ESP Mail");
  message.sender.email = AUTHOR_EMAIL;
  message.subject = F("Test sending html Email");
  message.addRecipient(F("Admin"), RECIPIENT_EMAIL);

  String htmlMsg = "<p>WARNING the water rate is <span style=\"color:#ff0000;\">LOWWW</span>.</p><p>Plz refill the tank ASAP.</p>";
  message.html.content = htmlMsg;

  /** The html text message character set e.g.
   * us-ascii
   * utf-8
   * utf-7
   * The default value is utf-8
   */
  message.html.charSet = F("us-ascii");

  /** The content transfer encoding e.g.
   * enc_7bit or "7bit" (not encoded)
   * enc_qp or "quoted-printable" (encoded)
   * enc_base64 or "base64" (encoded)
   * enc_binary or "binary" (not encoded)
   * enc_8bit or "8bit" (not encoded)
   * The default value is "7bit"
   */
  message.html.transfer_encoding = Content_Transfer_Encoding::enc_7bit;

  /** The message priority
   * esp_mail_smtp_priority_high or 1
   * esp_mail_smtp_priority_normal or 3
   * esp_mail_smtp_priority_low or 5
   * The default value is esp_mail_smtp_priority_low
   */
  message.priority = esp_mail_smtp_priority::esp_mail_smtp_priority_low;

  /** The Delivery Status Notifications e.g.
   * esp_mail_smtp_notify_never
   * esp_mail_smtp_notify_success
   * esp_mail_smtp_notify_failure
   * esp_mail_smtp_notify_delay
   * The default value is esp_mail_smtp_notify_never
   */
  // message.response.notify = esp_mail_smtp_notify_success | esp_mail_smtp_notify_failure | esp_mail_smtp_notify_delay;

  /* Set the custom message header */
  message.addHeader(F("Message-ID: <abcde.fghij@gmail.com>"));

  /* Set the TCP response read timeout in seconds */
  // smtp.setTCPTimeout(10);

  /* Connect to the server */
  if (!smtp.connect(&config))
  {
    MailClient.printf("Connection error, Status Code: %d, Error Code: %d, Reason: %s\n", smtp.statusCode(), smtp.errorCode(), smtp.errorReason().c_str());
    return;
  }

  if (!smtp.isLoggedIn())
  {
    Serial.println("Error, Not yet logged in.");
  }
  else
  {
    if (smtp.isAuthenticated())
      Serial.println("Successfully logged in.");
    else
      Serial.println("Connected with no Auth.");
  }

  /* Start sending Email and close the session */
  if (!MailClient.sendMail(&smtp, &message))
    MailClient.printf("Error, Status Code: %d, Error Code: %d, Reason: %s\n", smtp.statusCode(), smtp.errorCode(), smtp.errorReason().c_str());

  // to clear sending result log
  // smtp.sendingResult.clear();
  sentMailFlag = true;

  MailClient.printf("Free Heap: %d\n", MailClient.getFreeHeap());
}

/* Callback function to get the Email sending status */
void smtpCallback(SMTP_Status status)
{
  /* Print the current status */
  Serial.println(status.info());

  /* Print the sending result */
  if (status.success())
  {
    // MailClient.printf used in the examples is for format printing via debug Serial port
    // that works for all supported Arduino platform SDKs e.g. SAMD, ESP32 and ESP8266.
    // In ESP8266 and ESP32, you can use Serial.printf directly.

    Serial.println("----------------");
    MailClient.printf("Message sent success: %d\n", status.completedCount());
    MailClient.printf("Message sent failed: %d\n", status.failedCount());
    Serial.println("----------------\n");

    for (size_t i = 0; i < smtp.sendingResult.size(); i++)
    {
      /* Get the result item */
      SMTP_Result result = smtp.sendingResult.getItem(i);

      // In case, ESP32, ESP8266 and SAMD device, the timestamp get from result.timestamp should be valid if
      // your device time was synched with NTP server.
      // Other devices may show invalid timestamp as the device time was not set i.e. it will show Jan 1, 1970.
      // You can call smtp.setSystemTime(xxx) to set device time manually. Where xxx is timestamp (seconds since Jan 1, 1970)

      MailClient.printf("Message No: %d\n", i + 1);
      MailClient.printf("Status: %s\n", result.completed ? "success" : "failed");
      MailClient.printf("Date/Time: %s\n", MailClient.Time.getDateTimeString(result.timestamp, "%B %d, %Y %H:%M:%S").c_str());
      MailClient.printf("Recipient: %s\n", result.recipients.c_str());
      MailClient.printf("Subject: %s\n", result.subject.c_str());
    }
    Serial.println("----------------\n");

    // You need to clear sending result as the memory usage will grow up.
    smtp.sendingResult.clear();
  }
}