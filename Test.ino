#include <LittleFS.h>
#if defined(ESP32)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif

#include <Firebase_ESP_Client.h>
#include <addons/TokenHelper.h>
#include <addons/SDHelper.h>

/* 1. Define the WiFi credentials */
#define WIFI_SSID "2001"                    /* Your Wifi SSID */
#define WIFI_PASSWORD "19821968"            /* Your Wifi Password */

/* 2. Define the API Key */
#define API_KEY "AIzaSyBIKU1XmpKhb2xiYK6cLTxijSSahaqRNiU"

/* 3. Define the user Email and password that alreadey registerd or added in your project */
#define USER_EMAIL "ehab4zu@gmail.com"      /* your gmail */
#define USER_PASSWORD "123456"              /* your password */

/* 4. Define the Firebase storage bucket ID e.g bucket-name.appspot.com */
#define STORAGE_BUCKET_ID "tutorial-c161d.appspot.com"

// Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

bool taskCompleted = false;

void setup()
{
    Serial.begin(115200);

    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    Serial.print("Connecting to Wi-Fi");
    unsigned long ms = millis();
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(300);
    }
    Serial.println();
    Serial.print("Connected with IP: ");
    Serial.println(WiFi.localIP());
    Serial.println();

    Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

    /* Assign the api key (required) */
    config.api_key = API_KEY;

    /* Assign the user sign in credentials */
    auth.user.email = USER_EMAIL;
    auth.user.password = USER_PASSWORD;
    
    config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h
    Firebase.reconnectNetwork(true);
    fbdo.setBSSLBufferSize(4096 /* Rx buffer size in bytes from 512 - 16384 */, 1024 /* Tx buffer size in bytes from 512 - 16384 */);
    config.fcs.download_buffer_size = 2048;

    Firebase.begin(&config, &auth);
    
    //SD_Card_Mounting(); // See src/addons/SDHelper.h
    pinMode(2, OUTPUT);
}

// The Firebase Storage download callback function
void fcsDownloadCallback(FCS_DownloadStatusInfo info)
{
    if (info.status == firebase_fcs_download_status_init)
    {
        Serial.printf("Downloading file %s (%d) to %s\n", info.remoteFileName.c_str(), info.fileSize, info.localFileName.c_str());
    }
    else if (info.status == firebase_fcs_download_status_download)
    {
        Serial.printf("Downloaded %d%s, Elapsed time %d ms\n", (int)info.progress, "%", info.elapsedTime);
    }
    else if (info.status == firebase_fcs_download_status_complete)
    {
        Serial.println("Download completed\n");
    }
    else if (info.status == firebase_fcs_download_status_error)
    {
        Serial.printf("Download failed, %s\n", info.errorMsg.c_str());
    }
}

void readFile(const char *path)
{
  // open file for read
  File file = LittleFS.open(path, "r");
  if(!file)
  {
    Serial.println("Failed to open file");
    return;
  }
  Serial.println("Read from file:");
  while(file.available())
  {
    // printing file data byte by byte in HEX format
    Serial.print(file.read(), HEX);
    Serial.print(' ');
  }
  file.close();
}

void loop()
{
    // Firebase.ready() should be called repeatedly to handle authentication tasks.
    if (Firebase.ready() && !taskCompleted)
    {
        taskCompleted = true;

        Serial.println("\nDownload file...\n");

        if (!Firebase.Storage.download(&fbdo, STORAGE_BUCKET_ID /* Firebase Storage bucket id */, "Application.bin" /* path of remote file stored in the bucket */, "/new.bin" /* path to local file */, mem_storage_type_flash /* memory storage type, mem_storage_type_flash and mem_storage_type_sd */, fcsDownloadCallback /* callback function */))
            Serial.println(fbdo.errorReason());
        // Reading the file after it has been downloaded
        readFile("/new.bin");
    }
}
