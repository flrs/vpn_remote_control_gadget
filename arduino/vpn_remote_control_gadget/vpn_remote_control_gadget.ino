#include <GxEPD.h>
#include "SPI.h"
#include <WiFi.h>
#include <ArduinoHttpClient.h>
#include <rBase64.h>
#include <ArduinoJson.h>
#include "StringSplitter.h"
#include "credentials.h"

#include <GxGDEH0213B73/GxGDEH0213B73.h>
#include <Fonts/FreeSansBold9pt7b.h>

#include <GxIO/GxIO_SPI/GxIO_SPI.h>
#include <GxIO/GxIO.h>

#define SPI_MOSI 23
#define SPI_MISO -1
#define SPI_CLK 18

#define ELINK_SS 5
#define ELINK_BUSY 4
#define ELINK_RESET 16
#define ELINK_DC 17

#define BUTTON_PIN 39
#define MAX_CLIENTS 5

GxIO_Class io(SPI, ELINK_SS, ELINK_DC, ELINK_RESET);
GxEPD_Class display(io, ELINK_RESET, ELINK_BUSY);

String client_names[MAX_CLIENTS];
String client_usernames[MAX_CLIENTS];
String client_pws[MAX_CLIENTS];
String client_protos[MAX_CLIENTS];
int client_addresses[MAX_CLIENTS];

String asus_token;
String orig_client_list;

int selected_client = 0;
int connected_client = 0;

unsigned long last_menu_interaction = 0;

WiFiClient wifi;
HttpClient client = HttpClient(wifi, server_address, 80);

const unsigned char arrow[] = {
        0x00, 0x00,
        0xc0, 0x00,
        0xf0, 0x00,
        0xfe, 0x00,
        0xff, 0x80,
        0xff, 0xe0,
        0xff, 0xe0,
        0xff, 0x80,
        0xfe, 0x00,
        0xf0, 0x00,
        0xc0, 0x00,
        0x00, 0x00
};

const unsigned char vpn[] = {
        0xff, 0xff, 0xff, 0xfe,
        0xff, 0xff, 0xff, 0xfe,
        0xff, 0xff, 0xff, 0xfe,
        0xff, 0xff, 0xff, 0xfe,
        0xff, 0xff, 0xff, 0xfe,
        0xff, 0xff, 0xff, 0xfe,
        0xff, 0xff, 0xff, 0xfe,
        0xff, 0xff, 0xff, 0xfe,
        0xff, 0xff, 0xff, 0xfe,
        0xf9, 0xc8, 0x37, 0xbe,
        0xf9, 0xc8, 0x13, 0x9e,
        0xf8, 0x8b, 0x91, 0x9e,
        0xfc, 0x98, 0x10, 0x9e,
        0xfc, 0x18, 0x30, 0x1e,
        0xfe, 0x3b, 0xf2, 0x1e,
        0xfe, 0x3b, 0xf3, 0x1e,
        0xff, 0x7b, 0xf3, 0x9e,
        0x7f, 0xff, 0xff, 0xfe,
        0x7f, 0xff, 0xff, 0xfe,
        0x7f, 0xff, 0xff, 0xfe,
        0x7f, 0xff, 0xff, 0xfc,
        0x3f, 0xff, 0xff, 0xfc,
        0x3f, 0xff, 0xff, 0xf8,
        0x1f, 0xff, 0xff, 0xf8,
        0x0f, 0xff, 0xff, 0xf0,
        0x07, 0xff, 0xff, 0xe0,
        0x03, 0xff, 0xff, 0xc0,
        0x00, 0xff, 0xff, 0x00,
        0x00, 0x3f, 0xfc, 0x00,
        0x00, 0x1f, 0xf0, 0x00,
        0x00, 0x07, 0xe0, 0x00,
        0x00, 0x01, 0x80, 0x00
};

const unsigned char tick[] = {
        0x00, 0x0f, 0xf0, 0x00,
        0x00, 0x7f, 0xfe, 0x00,
        0x01, 0xff, 0xff, 0x80,
        0x03, 0xff, 0xff, 0xc0,
        0x07, 0xff, 0xff, 0xe0,
        0x0f, 0xff, 0xff, 0xf0,
        0x1f, 0xff, 0xff, 0xf8,
        0x3f, 0xff, 0xff, 0xfc,
        0x3f, 0xff, 0xff, 0xfc,
        0x7f, 0xff, 0xfe, 0xfe,
        0x7f, 0xff, 0xf8, 0x7e,
        0x7f, 0xff, 0xf0, 0x3e,
        0xff, 0xff, 0xe0, 0x3f,
        0xff, 0xff, 0xc0, 0x7f,
        0xfe, 0x7f, 0x80, 0xff,
        0xfc, 0x3f, 0x01, 0xff,
        0xfc, 0x1e, 0x03, 0xff,
        0xfc, 0x0c, 0x07, 0xff,
        0xfe, 0x00, 0x0f, 0xff,
        0xff, 0x00, 0x1f, 0xff,
        0x7f, 0x80, 0x3f, 0xfe,
        0x7f, 0xc0, 0x7f, 0xfe,
        0x7f, 0xe0, 0xff, 0xfe,
        0x3f, 0xf1, 0xff, 0xfc,
        0x3f, 0xff, 0xff, 0xf8,
        0x1f, 0xff, 0xff, 0xf8,
        0x0f, 0xff, 0xff, 0xf0,
        0x07, 0xff, 0xff, 0xe0,
        0x03, 0xff, 0xff, 0xc0,
        0x01, 0xff, 0xff, 0x00,
        0x00, 0x7f, 0xfe, 0x00,
        0x00, 0x0f, 0xf0, 0x00
};

const unsigned char hourglass[] = {
        0x07, 0xff, 0xff, 0xe0,
        0x07, 0xff, 0xff, 0xe0,
        0x07, 0xff, 0xff, 0xe0,
        0x00, 0x00, 0x00, 0x00,
        0x01, 0xff, 0xff, 0x80,
        0x01, 0xff, 0xff, 0x80,
        0x01, 0xff, 0xff, 0x80,
        0x01, 0xff, 0xff, 0x80,
        0x01, 0xff, 0xff, 0x80,
        0x01, 0xff, 0xff, 0x80,
        0x01, 0xff, 0xff, 0x00,
        0x00, 0x7f, 0xfe, 0x00,
        0x00, 0x3f, 0xfc, 0x00,
        0x00, 0x1f, 0xf8, 0x00,
        0x00, 0x0f, 0xf0, 0x00,
        0x00, 0x07, 0xe0, 0x00,
        0x00, 0x07, 0xe0, 0x00,
        0x00, 0x0f, 0xf0, 0x00,
        0x00, 0x1f, 0xf8, 0x00,
        0x00, 0x3f, 0xfc, 0x00,
        0x00, 0xff, 0xfe, 0x00,
        0x01, 0xff, 0xff, 0x80,
        0x01, 0xff, 0xff, 0x80,
        0x01, 0xff, 0xff, 0x80,
        0x01, 0xff, 0xff, 0x80,
        0x01, 0xff, 0xff, 0x80,
        0x01, 0xff, 0xff, 0x80,
        0x01, 0xff, 0xff, 0x80,
        0x00, 0x00, 0x00, 0x00,
        0x07, 0xff, 0xff, 0xe0,
        0x07, 0xff, 0xff, 0xe0,
        0x07, 0xff, 0xff, 0xe0
};

const unsigned char wifi_signal[] = {
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x07, 0xe0, 0x00,
        0x00, 0x7f, 0xfe, 0x00,
        0x03, 0xff, 0xff, 0xc0,
        0x0f, 0xff, 0xff, 0xf0,
        0x1f, 0xff, 0xff, 0xf8,
        0x7f, 0xff, 0xff, 0xfc,
        0xff, 0xf0, 0x0f, 0xff,
        0x7f, 0x80, 0x01, 0xfe,
        0x3e, 0x00, 0x00, 0x7c,
        0x1c, 0x00, 0x00, 0x38,
        0x00, 0x1f, 0xf8, 0x10,
        0x00, 0x7f, 0xfe, 0x00,
        0x00, 0xff, 0xff, 0x00,
        0x03, 0xff, 0xff, 0x80,
        0x01, 0xff, 0xff, 0x00,
        0x00, 0xfc, 0x3e, 0x00,
        0x00, 0x70, 0x0c, 0x00,
        0x00, 0x20, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x03, 0xc0, 0x00,
        0x00, 0x07, 0xe0, 0x00,
        0x00, 0x07, 0xe0, 0x00,
        0x00, 0x07, 0xe0, 0x00,
        0x00, 0x07, 0xe0, 0x00,
        0x00, 0x03, 0xc0, 0x00,
        0x00, 0x01, 0x80, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00
};

const unsigned char wifi_error[]{
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0xff, 0xf0, 0x00,
        0x03, 0xff, 0xfe, 0x00,
        0x0f, 0xff, 0xff, 0x80,
        0x3f, 0xff, 0xff, 0xc0,
        0x7f, 0xff, 0xff, 0xe0,
        0xff, 0xc0, 0x1f, 0xf0,
        0x7e, 0x00, 0x07, 0xe0,
        0x3c, 0x00, 0x01, 0xc0,
        0x10, 0x3f, 0xc0, 0xc0,
        0x00, 0xff, 0xf0, 0x00,
        0x01, 0xff, 0xf2, 0x04,
        0x03, 0xff, 0xe7, 0x0e,
        0x01, 0xff, 0xcf, 0x9f,
        0x00, 0xf0, 0x67, 0xfe,
        0x00, 0x40, 0x33, 0xfc,
        0x00, 0x00, 0x01, 0xf8,
        0x00, 0x06, 0x01, 0xf8,
        0x00, 0x0f, 0x83, 0xfc,
        0x00, 0x1f, 0x87, 0xfe,
        0x00, 0x1f, 0x87, 0x9e,
        0x00, 0x0f, 0x83, 0x0c,
        0x00, 0x0f, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00
};


bool connect_to_network() {
    WiFi.begin(ssid, passphrase);

    Serial.print("Establishing connection to WiFi..");
    unsigned long start_connect = millis();
    while (WiFi.status() != WL_CONNECTED && (millis() - start_connect) < 15000) {
        delay(500);
        Serial.print(".");
    }
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("error!");
        return false;
    } else {
        Serial.println("done!");
        return true;
    }
}

bool get_auth_token() {
    Serial.print("Getting auth token...");
    rbase64.encode(login_credentials);
    String postData = "login_authorization=";
    postData.concat(rbase64.result());
    client.beginRequest();
    client.post("/login.cgi");
    client.sendHeader(HTTP_HEADER_CONTENT_TYPE, "application/x-www-form-urlencoded");
    client.sendHeader(HTTP_HEADER_USER_AGENT, "asusrouter-Android-DUTUtil-1.0.0.3.58-163");
    client.sendHeader(HTTP_HEADER_CONTENT_LENGTH, postData.length());
    client.endRequest();
    client.write((const byte *) postData.c_str(), postData.length());

    int statusCode = client.responseStatusCode();
    String response = client.responseBody();

    if (statusCode == 200) {
        DynamicJsonDocument doc(128);
        DeserializationError error = deserializeJson(doc, response);
        if (error) {
            Serial.println("error in deserialization.");
            asus_token = "";
            return false;
        } else {
            Serial.println("done!");
            asus_token = (const char *) doc["asus_token"];
        }
    } else {
        asus_token = "";
        Serial.print("error in request. Return status code: ");
        Serial.println(statusCode);
        return false;
    }
    return true;
}

bool get_server_info() {
    Serial.print("Getting server info...");
    String postData = "hook=nvram_get(vpnc_clientlist)";
    String auth_header = "asus_token=";
    auth_header.concat(asus_token);

    client.beginRequest();
    client.post("/appGet.cgi");
    client.sendHeader(HTTP_HEADER_CONTENT_TYPE, "application/x-www-form-urlencoded");
    client.sendHeader(HTTP_HEADER_USER_AGENT, "asusrouter-Android-DUTUtil-1.0.0.3.58-163");
    client.sendHeader("COOKIE", auth_header);
    client.sendHeader(HTTP_HEADER_CONTENT_LENGTH, postData.length());
    client.endRequest();
    client.write((const byte *) postData.c_str(), postData.length());

    int statusCode = client.responseStatusCode();
    String response = client.responseBody();

    String client_list = "";
    if (statusCode == 200) {
        DynamicJsonDocument doc(512);
        DeserializationError error = deserializeJson(doc, response);
        if (error) {
            Serial.println("error in deserialization.");
            return false;
        } else {
            Serial.println("done!");
            orig_client_list = (const char *) doc["vpnc_clientlist"];
            String client_list = orig_client_list;
            orig_client_list.replace("&#62", ">");
            orig_client_list.replace("&#60", "<");

            client_list.replace("&#62", "¦");
            client_list.replace("&#60", "|");

            StringSplitter *client_list_splitter = new StringSplitter(client_list, '|', -1);
            int no_clients = client_list_splitter->getItemCount();

            for (int client_ct = 0; client_ct < no_clients; client_ct++) {
                String client_item = client_list_splitter->getItemAtIndex(client_ct);
                StringSplitter *client_item_splitter = new StringSplitter(client_item, '¦', -1);
                int no_client_infos = client_item_splitter->getItemCount();

                client_names[client_ct] = client_item_splitter->getItemAtIndex(0);
                client_names[client_ct] = client_names[client_ct].substring(0, client_names[client_ct].length() - 1);

                client_protos[client_ct] = client_item_splitter->getItemAtIndex(1);
                client_protos[client_ct] = client_protos[client_ct].substring(0, client_protos[client_ct].length() - 1);
                client_protos[client_ct].toLowerCase();
                client_addresses[client_ct] = (client_item_splitter->getItemAtIndex(2)).toInt();

                client_usernames[client_ct] = client_item_splitter->getItemAtIndex(3);
                client_usernames[client_ct] = client_usernames[client_ct].substring(0,
                                                                                    client_usernames[client_ct].length() -
                                                                                    1);
                client_pws[client_ct] = client_item_splitter->getItemAtIndex(4);
            }
        }
    } else {
        Serial.print("error in request. Return status code: ");
        Serial.println(statusCode);
        return false;
    }
    return true;
}


bool set_server(int server_id, bool deactivate) {
    Serial.print("Now setting server...");
    String query = "";

    query.concat("current_page=Advanced_VPNClient_Content.asp");
    query.concat("&next_page=Advanced_VPNClient_Content.asp");
    query.concat("&modified=0");
    query.concat("&flag=background");
    query.concat("&action_mode=apply");
    query.concat("&action_script=restart_vpncall");
    query.concat("&action_wait=3");
    query.concat("&preferred_lang=EN");
    query.concat("&firmver=3.0.0.4");
    query.concat("&vpnc_dnsenable_x=1");
    query.concat("&vpnc_proto=");
    if (!deactivate) {
        query.concat(client_protos[server_id]);
    }
    query.concat("&vpnc_clientlist=");

    String orig_client_list_changes = orig_client_list;
    orig_client_list_changes.replace(" ", "+");
    orig_client_list_changes = URLEncoder.encode(orig_client_list_changes);
    orig_client_list_changes.replace("%2B", "+");
    query.concat(orig_client_list_changes);
    query.concat("&vpnc_type=PPTP");
    query.concat("&vpn_client_unit=");
    if (!deactivate) {
        query.concat(client_addresses[server_id]);
    }
    for (int i = 0; i < MAX_CLIENTS; i++) {
        String var_name = "vpn_client";
        var_name.concat(i + 1);
        var_name.concat("_username");
        query.concat("&");
        query.concat(var_name);
        query.concat("=");
        query.concat(client_usernames[i]);

        var_name = "vpn_client";
        var_name.concat(i + 1);
        var_name.concat("_password");
        query.concat("&");
        query.concat(var_name);
        query.concat("=");
        query.concat(client_pws[i]);
    }
    query.concat("&vpn_clientx_eas=");
    if (!deactivate) {
        String eas = String(client_addresses[server_id]);
        eas.concat("%2C");
        query.concat(eas);
    }

    String options_list = "";
    for (int i = 0; i < MAX_CLIENTS; i++) {
        options_list.concat("<auto");
    }
    query.concat("&vpnc_pptp_options_x_list=");
    query.concat(URLEncoder.encode(options_list));

    query.concat("&vpnc_des_edit=");
    if (!deactivate) {
        String chosen_client = client_names[server_id];
        chosen_client.replace(" ", "+");
        query.concat(chosen_client);
    }

    query.concat("&selPPTPOption=auto");

    String postData;

    String auth_header = "asus_token=";
    auth_header.concat(asus_token);
    client.beginRequest();
    client.post("/start_apply.htm");
    client.sendHeader(HTTP_HEADER_CONTENT_TYPE, "application/x-www-form-urlencoded");
    client.sendHeader(HTTP_HEADER_USER_AGENT, "asusrouter-Android-DUTUtil-1.0.0.3.58-163");
    client.sendHeader("COOKIE", auth_header);
    client.sendHeader("Accept-Encoding", "gzip, deflate");
    client.sendHeader("Accept", "*/*");
    client.sendHeader("Connection", "keep-alive");
    client.sendHeader(HTTP_HEADER_CONTENT_LENGTH, query.length());
    client.endRequest();
    client.write((const byte *) query.c_str(), query.length());

    int status_code = client.responseStatusCode();
    String response = client.responseBody();
    if (status_code == 200) {
        if (!deactivate) {
            Serial.print("done and connected to ");
            Serial.print(client_names[server_id]);
            Serial.println("!");
        } else {
            Serial.println("done and disconnected from VPN.");
        }
        connected_client = server_id;
        return true;
    } else {
        Serial.print("error, error code: ");
        Serial.println(status_code);
        return false;
    }
}

void draw_menu(bool partial_update) {
    if (!partial_update) {
        for (int i = 0; i < MAX_CLIENTS; i++) {
            display.setCursor(15, 15 + i * 20);
            display.println(client_names[i]);
        }
        display.setCursor(15, 15 + MAX_CLIENTS * 20);
        display.println("Disable VPN");
    } else {
        display.fillRect(0, 0, 15, display.height(), GxEPD_WHITE);
    }
    display.drawBitmap(arrow, 0, 15 + selected_client * 20 - 11, 12, 12, GxEPD_BLACK);
    if (!partial_update) {
        display.update();
    } else {
        display.updateWindow(0, 0, 15, display.height(), true);
    }
    last_menu_interaction = millis();
}

void setup() {
    pinMode(BUTTON_PIN, INPUT);
    Serial.begin(115200);
    Serial.println();
    Serial.println("Starting up.");
    SPI.begin(SPI_CLK, SPI_MISO, SPI_MOSI, ELINK_SS);
    display.init();

    display.setRotation(3);
    display.fillScreen(GxEPD_WHITE);
    display.setTextColor(GxEPD_BLACK);
    display.setFont(&FreeSansBold9pt7b);
    display.update();

    display.fillRect(display.width() - 40, 0, 2, display.height(), GxEPD_BLACK);
    display.updateWindow(display.width() - 40, 0, 2, display.height(), true);

    display.drawBitmap(vpn, display.width() - 32, 0, 32, 32, GxEPD_BLACK);
    display.updateWindow(display.width() - 32, 0, 32, 32, true);

    if (connect_to_network() && get_auth_token()) {
        display.drawBitmap(wifi_signal, display.width() - 32, display.height() / 2 - 16, 32, 32, GxEPD_BLACK);
    } else {
        display.drawBitmap(wifi_error, display.width() - 32, display.height() / 2 - 16, 32, 32, GxEPD_BLACK);
    }
    display.updateWindow(display.width() - 32, 36, 32, 32, true);

    if (asus_token.length() == 0) {
        Serial.println("Startup error: Error fetching router credentials.");
        display.drawBitmap(wifi_error, display.width() - 32, display.height() / 2 - 16, 32, 32, GxEPD_BLACK);
        display.updateWindow(display.width() - 32, 36, 32, 32, true);

    } else {
        get_server_info();
        draw_menu(false);
        Serial.println("Startup successful.");
    }

}


void loop() {
    if (!digitalRead(BUTTON_PIN)) {
        selected_client += 1;
        if (selected_client == MAX_CLIENTS + 1) {
            selected_client = 0;
        }
        while (!digitalRead(BUTTON_PIN)) {
            delay(20);
        }
        draw_menu(true);
    }
    if (millis() - last_menu_interaction > 2000 && connected_client != selected_client) {
        display.drawBitmap(hourglass, display.width() - 32 - 1, display.height() - 32 - 1, 32, 32, GxEPD_BLACK);
        display.updateWindow(display.width() - 32 - 1, display.height() - 32 - 1, 32, 32, true);
        if (set_server(selected_client, selected_client == MAX_CLIENTS)) {
            display.drawBitmap(tick, display.width() - 32 - 1, display.height() - 32 - 1, 32, 32, GxEPD_BLACK);
            display.updateWindow(display.width() - 32 - 2, display.height() - 32 - 1, 33, 32, true);
            if (selected_client != MAX_CLIENTS) {
                display.drawBitmap(vpn, display.width() - 32 - 1, 0, 32, 32, GxEPD_BLACK);
            } else {
                display.fillRect(display.width() - 32 - 1, 0, 32, 32, GxEPD_WHITE);
            }
            display.updateWindow(display.width() - 32 - 1, 0, 32, 32, true);
        }
    }
}
