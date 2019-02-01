//-------- Chech Tag --------
String checkTag(String tagID, double Irms) {

  String tag = emptyTag;

  if (tagID != emptyTag) {
    tag = tagID;
  }
  if (Irms < 1.1) {
    tag = emptyTag;
  }
  return tag;
}
