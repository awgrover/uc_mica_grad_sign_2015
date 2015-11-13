// Requires
// dotstar library from https://github.com/adafruit/Adafruit_DotStar/archive/master.zip
//   (see instructions at https://learn.adafruit.com/adafruit-dotstar-leds)

#include "tired_of_serial.h"
#include "SignTypes.h"

#include <Adafruit_DotStar.h>
// Because conditional #includes don't work w/Arduino sketches...
#include <SPI.h>         // COMMENT OUT THIS LINE FOR GEMMA OR TRINKET
//#include <avr/power.h> // ENABLE THIS LINE FOR GEMMA OR TRINKET

#define NUMPIXELS 60 // Number of LEDs in strip
// Here's how to control the LEDs from any two pins:
#define DATAPIN    4 // "D0"
#define CLOCKPIN   5 // "C0"
Adafruit_DotStar strip = Adafruit_DotStar(NUMPIXELS, DATAPIN, CLOCKPIN, DOTSTAR_BRG);

const unsigned long LOOPLIMIT = 0; // 3000; // debug, msecs to limit
#ifdef DirectPin
const int RedPin = 10;
const int GreenPin = 9;
const int BluePin = 11;
#endif

// #define debug_output
#ifdef debug_output
  #define debug_output2
  #define debug(x) print(x)
  #define debugln() println()
#else
  #define debug(x) 
  #define debugln() 
#endif
// #define debug_output2
#ifdef debug_output2
  #define debug2(x) print(x)
  #define debug2hex(x) print(x,HEX)
  #define debugln2() println()
#else
  #define debug2(x)
  #define debug2hex(x)
  #define debugln2()
#endif

const int Stayingness = 3; // a "group" will only change color 1 out of Stayingess times
const int GroupSize = 3; // how many leds/group
const int GroupCt = NUMPIXELS/GroupSize; // How many sets of leds

group_struct Groups[ GroupCt ];

// r,g,b
// FIXME: ought to all be const, but c++
RGB Purple = RGB( 101, 102, 3 );
RGB Yellow = RGB( 1, 101, 103 );
RGB Orange = RGB( 101, 2, 103 );

// need the list so we can pick random ones
RGB *Colors[] = { &Purple, &Orange, &Yellow };

template <typename T,unsigned S> inline unsigned arraysize(const T (&v)[S]) { return S; }; // magic
const int ColorLen = arraysize(Colors);

// Fade for FadeTime + rand(FadeVariableTime)
const unsigned long FadeTime = 3000;
const unsigned long FadeVariableTime = 1000;

void setup() {
  strip.begin(); // Initialize pins for output
  strip.show();  // Turn all LEDs off ASAP

  Serial.begin(115200);
  debug("Setup led-thingy\n");

    debug( "Random start:\n" );
    for(int i=0; i<GroupCt; i++) {
        group_struct &a_group = Groups[i];
        debug("  group ");debug(i);debug(" ");debug((long)&(Groups[i]));debug(" local ");debug((long)&a_group);debugln();
        a_group.from = random(ColorLen);
        a_group.end_at = 0;
        a_group.to = NULL;
        a_group.duration = 0;

        choose_new_to( a_group );
        // say Dumper(\%a_group);
        debug("Setup group ["); debug(i); debug(" "); debug( Colors[a_group.from] ); debug(" -> "); debug( Colors[a_group.to] );debugln();

        display_group(i, *Colors[ a_group.from ] );
        debug( " fades for "); debug(a_group.duration);debugln();
        }

    debug( "setup done\n");
    debug( "-----\n");
    // while (1) {}
    }

void loop() {
  if (LOOPLIMIT && millis() > LOOPLIMIT) { while(1) {} } // freeze at limit
  

  debug( "\nEACH GROUP @"); debug(millis()); debugln();
  for (int group_i=0; group_i < GroupCt; group_i++) {
    group_struct &group = Groups[group_i];
    unsigned long now = millis();
    if (group.end_at < now) {
      // new to/end_at
      debug( "Reset ["); debug(group_i); debug("] ");debug(Colors[group.from]); debug(" -> ");debug(Colors[group.to]);debugln();
      group.from = group.to;
      choose_new_to( group );
      debug( "       "); debug(group_i); debug("] ");debug(Colors[group.from]); debug(" -> ");debug(Colors[group.to]);debugln();
      // while(1) {};
      continue;
      }

    // we are wasteful, we do everybody each time, AND set them each time
    RGB nextrgb;
    edgepath_next_rgb(group_i, now, nextrgb);
    display_group(group_i, nextrgb);
    }
  //delay(100);
  }


void choose_new_to(group_struct &group) {
    // Assumes 'from' is set, chooses a random 'to' and 'end_at'
    group.end_at = millis() + FadeTime + random(FadeVariableTime);
    group.duration = group.end_at-millis();
    // 'to' is one of the _other_ colors, so add 1|2 and do modulus to force it right
    // Only change on 1 out of stayingness times
    if (random(Stayingness) == 0) {
      group.to = (random(ColorLen-1) + 1 + group.from) % ColorLen;
      debug2("Change ");debug2((long)&group);debugln2();
      }
    else {
        group.from = group.to;
        debug2("NoChange ");debug2((long)&group);debugln2();
        }
    group.corner.red(-1); // flag for "not calc'd yet"
    }

void display_group(int groupi, RGB &rgb) {
    // Make the led(s) show the value
    debug( "display group ["); debug(groupi); debug("] to "); debug(rgb); debugln();
    // convert from a group_index to a 0..index
    int fromi = groupi * GroupSize;
    int toi = fromi + GroupSize;
    // display n=GroupSize leds:
    for (uint16_t pixel_i=fromi; pixel_i<toi; pixel_i++) {
        debug2("== [");debug2(pixel_i); debug2("] ");debug2(rgb);debug2(" @ ");debug2(millis());debug(" #");debug2hex(rgb.as_uint());debugln2();

        #ifdef DirectPin
        analogWrite(RedPin, rgb.red());
        analogWrite(GreenPin, rgb.green());
        analogWrite(BluePin, rgb.blue());
        #endif

        strip.setPixelColor(pixel_i, rgb.as_uint());
        }
    strip.show();                     // Refresh strip
    }

void edgepath_next_rgb(int group_i, unsigned long end_at_time, RGB &gradient_color) {
  // We want the "fade" (aka gradient) to follow the closest edges of the rgb triangle
  // So, 'from' is closest to some edge.
  // And, 'to' is closest to some edge.
  // The direction of the first path is towards the closes "corner" of from/to
  // So, move proportionally along the distance of the first edge, then the second.
 
  group_struct &group = Groups[group_i];

  RGB *from = Colors[ group.from ];
  RGB *to = Colors[ group.to ];

  if (group.corner.red()==255) { // flag for not-calc'd
    // calc and cache the corner
    debug( "calc "); debug(*from); debug(" -> "); debug(*to);debugln();

    group.corner.red( from->red());
    group.corner.green( from->green());
    group.corner.blue( from->blue());

    group.corner.along_our_min_to_their_min(*to);
    debug("  ");debug(from); debug(" -> "); debug(group.corner); debug(" -> "); debug(to); debugln();
    debug( "Edge["); debug(group_i); debug("] "); debug(from); debug(" -> "); debug(group.corner);debug(" -> ");debug(to);debugln(); 
    }

  // Now we know $now, and $end_at, $duration, and from->corner->to
  // so, just calc the proper point along those 2 line segments
  proportional_color_along( group,  end_at_time, gradient_color );
  debug( "  next rgb ");debug(gradient_color);debug(" @ "); debug(end_at_time);debugln();

  // has set gradient_color
  }

void proportional_color_along( group_struct &group,  unsigned long end_at_time, RGB &gradient_color ) {
  RGB &from = *Colors[ group.from ];
  RGB &to = *Colors[ group.to ];

  debug( "  prop color of ");debug(from); debug(" -> "); debug(group.corner); debug(" -> ");debug(to);debugln();
  unsigned int distance_to_corner = from.distance(group.corner);
  unsigned int total_len = distance_to_corner + group.corner.distance(to);
  unsigned long duration = group.duration;
  unsigned long elapsed_t = duration - (group.end_at - end_at_time); // FIXME: if end_at < end_at_time, then calc next 'to'
  // FIXME: check math-under/overflow:
  unsigned int distance_to_now = total_len * ((float)elapsed_t / duration); // i.e. dist proportional to elapsed time
  debug("    ");debug(total_len);debug(" : ");debug(distance_to_now);debug(" = ");debug(duration);debug(" : ");debug(elapsed_t);debugln();

  if (distance_to_now <= distance_to_corner) {
    gradient_color.proportional_rgb( from, group.corner, distance_to_now, distance_to_corner);
    }
  else if (distance_to_now <= total_len) {
    unsigned int distance_from_corner = total_len - distance_to_corner;
    debug("2:");
    gradient_color.proportional_rgb( group.corner, to, distance_to_now-distance_to_corner, distance_from_corner);
    }
  else {
    debug("past! FREEZE");debugln();while(1) {};
    }
  }
