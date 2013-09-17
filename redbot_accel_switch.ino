#include <RedBot.h>

#define WAIT        10    // How often we wait between samples
#define POINTS      10    // How many points to average for threshold tests
#define DEBOUNCE    5     // How many points to constitute a change
#define THRESHOLD1  512   // Value needed to switch
#define THRESHOLD2  2048  // Value needed to switch
#define OUTPUT1_PIN 4     // An output
#define OUTPUT2_PIN 3     // An output

/* NOTE: The "Redbot" Accelerometer MUST be attached to A4/A5 on an
 *       Arduino. These values are hard-coded into the "Redbot"
 *       library and (for our purposes) should not be changed.
 */
RedBotAccel accel;
uint8_t index;
double dList[POINTS];
double aCurrent, aLast;

boolean switch_A[POINTS];
boolean switch_B[POINTS];

// TODO: this is not a REAL debounce routine, since the count never
//       goes back to '0' when it glitches... but it works pending
//       any better alternatives
boolean aState()
{
  uint8_t count=0;
  for (uint8_t i=0; i<POINTS; i++) {
    if (switch_A[i] == true) count++;
  }
  if (count > DEBOUNCE)
    return true;
  else
    return false;
}

boolean bState()
{
  uint8_t count=0;
  for (uint8_t i=0; i<POINTS; i++) {
    if (switch_B[i] == true) count++;
  }
  if (count > DEBOUNCE)
    return true;
  else
    return false;
}

double averageDisplacement()
{
  uint8_t i;
  double total = 0.0;
  for (i=0; i<POINTS; i++) {
    total += dList[i];
  }
  return (total / (double)POINTS);
}

double getDisplacementVector()
{
  double d=0.0;
  
  // According to the Pythagorean Theorem, x^2 + y^2 = d^2... but then if we
  // extrapolate to include a third coordinate, we essentially just create 2 right
  // triangles, the hypotenuse of the first becomes the base of the second, and
  // the equation boils down to: x^2 + y^2 + z^2 = d^2
  d += sq((double)accel.x);
  d += sq((double)accel.y);
  d += sq((double)accel.z);
  
  return sqrt(d);
}

void setup()
{
  pinMode(OUTPUT1_PIN, OUTPUT);
  pinMode(OUTPUT2_PIN, OUTPUT);
  accel.read();
  
  aCurrent = aLast = 0.0;
  for (index=0; index<POINTS; index++) {
    dList[index] = 0.0;
  }
  index = 0;
}

void loop()
{
  accel.read();  
  
  // Who cares about negative numbers in space...
  dList[index] = getDisplacementVector();
  
  // Average the last bunch of D-vectors...
  aCurrent = averageDisplacement();
  
  // Collect data points for 1st switch
  if (abs(aCurrent - aLast) > THRESHOLD1)
    switch_A[index] = true;
  else
    switch_A[index] = false;
  
  // Collect data points for 2nd switch
  if (abs(aCurrent - aLast) > THRESHOLD2)
    switch_B[index] = true;
  else
    switch_B[index] = false;
  
  // If average is good, turn on pin, else off
  if (aState() == true)
    digitalWrite(OUTPUT1_PIN, HIGH);
  else
    digitalWrite(OUTPUT1_PIN, LOW);
  
  // If average is good, turn on pin, else off
  if (bState() == true)
    digitalWrite(OUTPUT2_PIN, HIGH);
  else
    digitalWrite(OUTPUT2_PIN, LOW);
  
  index++; if ((index % POINTS) == 0) index = 0;
  aLast = aCurrent;
  
  delay(WAIT);
}
