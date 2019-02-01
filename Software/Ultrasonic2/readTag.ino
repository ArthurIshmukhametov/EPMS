//-------- Read Tag --------
String readTag() {

  String msg = "";
  int data;

  if (rfid.available() > 0) {
    delay(100);               // Необходима чтобы дать данным пройти через аналоговый буфер
    data = rfid.read();
    if (data == 2) { // Начало передачи
      for (int z = 0 ; z < 12 ; z++) {    // Чтение метки
        data = rfid.read();
        msg += char(data);
      }
    }
    data = rfid.read();
    rfid.flush();
    if (data == 3) {     // Конец передачи
      return msg;
    }
  }                       // Выдача метки
  return emptyTag;           //Выдача, если метка не найдена
}
