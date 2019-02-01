//-------- Send Status --------
void sendStatus(String token, String params) {

  String payload = "{\"token\": \""
                   + token
                   + "\",\"queryName\":\"SNSZ_IMPORT_SENSOR_DATA\",\"count\":20,\"params\":{\"IN_CLOB\":\""
                   + params
                   + "\"}}";

  //http.begin("http://192.168.0.113:9880/query/");

  http.begin("http://server-api.cara.bi/query/run/");
  Serial.print("[HTTP] POST query...\n");

  int httpCode = http.POST(payload);

  if (httpCode > 0) {
    if (debug) {
      Serial.printf("[HTTP] POST query... code: %d\n", httpCode);
    }

    if (httpCode == HTTP_CODE_OK) {
      Serial.println(httpCode);

      String payload = http.getString();
      Serial.println(payload);
    }
  } else {
    Serial.printf("[HTTP] POST query... failed, error: %s\n", http.errorToString(httpCode).c_str());
  }
  http.end();
}
