//-------- Get Token --------
String getToken() {

  String token = "";

  Serial.print("[HTTP] begin...\n");

  //http.begin("http://192.168.0.113:9880/auth/");

  http.begin("http://server-api.cara.bi/authorize/register/");

  Serial.print("[HTTP] POST auth...\n");

  int httpCode = http.POST(proj);

  // httpCode will be negative on error
  if (httpCode > 0) {
    if (debug) {
      Serial.printf("[HTTP] POST auth... code: %d\n", httpCode);
    }

    if (httpCode == HTTP_CODE_OK) {

      StaticJsonBuffer<512> jsonBuffer;
      JsonObject& root = jsonBuffer.parseObject(http.getString());

      if (!root.success()) {
        Serial.println(F("Parsing failed!"));
        http.end();
        return token;
      }

      token = root["token"].as<char*>();

      // Extract values
      Serial.println(F("Response token:"));
      Serial.println(token);
    }
  } else {
    Serial.printf("[HTTP] POST auth... failed, error: %s\n", http.errorToString(httpCode).c_str());
    http.end();
    return token;
  }

  http.end();
  return token;
}
