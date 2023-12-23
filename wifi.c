
const char* wifi_ap = "R_host";
const char* wifi_pd = "badenka5";


///////////////////////
int wifi_connect() {

  Serial.print("wifi");

  int max_tries = 30;

  WiFi.begin(wifi_ap, wifi_pd);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
    max_tries--;
    if (max_tries < 0) {
      Serial.println("wifi failed, giving up");
      return false;
    }
  }
  Serial.println("wifi ok");
  return true;
}

void wifi_disconnect() {
  WiFi.disconnect();
  int max_tries = 30;

  while ((WiFi.status() != WL_DISCONNECTED) && (max_tries > 0)) {
    delay(500);
    max_tries--;
    Serial.print("+");
  }

  Serial.println(" wifi disconnected (or not:)");
}