//-------- Check State --------
String checkState(double value) {

if ((value >= 0) && (value < low)) {
 return stateOff;
 }

if ((value >= low) && (value < hi)) {
    return stateOn;
  }

 return overCurrent;
}
