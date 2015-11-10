// #include <serial.h>
#include "tired_of_serial.h"
#include "SignTypes.h"

const unsigned long LOOPLIMIT = 3000; // debug, msecs to limit
const int RedPin = 10;
const int GreenPin = 9;
const int BluePin = 11;
#define debug print

const int GroupCt = 1; // 5; // How many sets of leds
const int GroupSize = 1; // 3; // how many leds/group

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
  Serial.begin(9600);
  print("Setup led-thingy\n");

    print( "Random start:\n" );
    for(int i=0; i<GroupSize; i++) {
        group_struct &a_group = Groups[i];
        print("  group ");print(i);print(" ");print((long)&(Groups[i]));print(" local ");print((long)&a_group);println();
        a_group.from = random(ColorLen);
        a_group.end_at = 0;
        a_group.to = NULL;
        a_group.duration = 0;

        choose_new_to( a_group );
        // say Dumper(\%a_group);
        print("Setup group ["); print(i); print(" "); print( Colors[a_group.from] ); print(" -> "); print( Colors[a_group.to] );println();

        display_group(i, *Colors[ a_group.from ] );
        print( " fades for "); print(a_group.duration);println();
        }

    print( "setup done\n");
    print( "-----\n");
    }

void loop() {
  if (LOOPLIMIT && millis() > LOOPLIMIT) { while(1) {} } // freeze at limit
  

  print( "\nEACH GROUP @"); print(millis()); println();
  for (int group_i=0; group_i < GroupSize; group_i++) {
    group_struct &group = Groups[group_i];
    unsigned long now = millis();
    if (group.end_at < now) {
      // new to/end_at
      group.from = random(ColorLen); 
      choose_new_to( group );
      print( "Reset ["); print(group_i); print("] ");print(Colors[group.from]); print(" -> ");print(Colors[group.to]);println();
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
    group.to = (random(ColorLen-1) + 1 + group.from) % ColorLen;
    group.corner.red(-1); // flag for "not calc'd yet"
    }

void display_group(int groupi, RGB &rgb) {
    // Make the led(s) show the value
    print( "display group ["); print(groupi); print("] to "); print(rgb); println();
    // convert from a group_index to a 0..index
    int fromi = groupi * GroupSize;
    int toi = fromi + GroupSize;
    // display n=GroupSize leds:
    for (int neo_i=fromi; neo_i<toi; neo_i++) {
        print("== [");print(neo_i); print("] ");print(rgb);print(" @ ");print(millis());println();
        analogWrite(RedPin, rgb.red());
        analogWrite(GreenPin, rgb.green());
        analogWrite(BluePin, rgb.blue());
        }
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
    print( "calc "); print(*from); print(" -> "); print(*to);println();
    RGB &corner = group.corner;
    corner.along_our_min_to_their_min(*to);
    print("  ");print(from); print(" -> "); print(corner); print(" -> "); print(to); println();
    print( "Edge["); print(group_i); print("] "); print(from); print(" -> "); print(group.corner);print(" -> ");print(to);println(); 
    }

  // Now we know $now, and $end_at, $duration, and from->corner->to
  // so, just calc the proper point along those 2 line segments
  proportional_color_along( group,  end_at_time, gradient_color );
  print( "  next rgb ");print(gradient_color);print(" @ "); print(end_at_time);println();

  // has set gradient_color
  }

void proportional_color_along( group_struct &group,  unsigned long end_at_time, RGB &gradient_color ) {
  RGB &from = *Colors[ group.from ];
  RGB &to = *Colors[ group.to ];

  print( "  prop color of ");print(from); print(" -> "); print(group.corner); print(" -> ");print(to);println();
  unsigned int distance_to_corner = from.distance(group.corner);
  unsigned int total_len = distance_to_corner + group.corner.distance(to);
  unsigned long duration = group.duration;
  unsigned long elapsed_t = duration - (group.end_at - end_at_time); // FIXME: if end_at < end_at_time, then calc next 'to'
  // FIXME: check math-under/overflow:
  unsigned int distance_to_now = total_len * ((float)elapsed_t / duration); // i.e. dist proportional to elapsed time
  print("    ");print(total_len);print(" : ");print(distance_to_now);print(" = ");print(duration);print(" : ");print(elapsed_t);println();

  if (distance_to_now <= distance_to_corner) {
    gradient_color.proportional_rgb( from, group.corner, distance_to_now, distance_to_corner);
    }
  else if (distance_to_now <= total_len) {
    unsigned int distance_from_corner = total_len - distance_to_corner;
    print("2:");
    gradient_color.proportional_rgb( group.corner, to, distance_to_now-distance_to_corner, distance_from_corner);
    }
  else {
    print("past! FREEZE");println();while(1) {};
    }
  }
