// #include <serial.h>
#include "tired_of_serial.h"
#include "RGB.h"
#include "magic.h"

long LOOPLIMIT = 3000; // debug

const int GroupCt = 1; // 5; // How many sets of leds
const int GroupSize = 1; // 3; // how many leds/group

struct group_struct { 
  // We use indexes into Colors for the Groups, so that we can pick new random "to" later.
  byte from; // Color[i]
  byte to; // Color[i]
  long end_at; // end millis
  long duration; // how long a cyle is millis
  // RGB *corner; // rgb_cached_corner_calc
  };
group_struct Groups[ GroupCt ];
// r,g,b
RGB Purple = RGB( 101, 102, 3 );
RGB Yellow = RGB( 1, 101, 103 );
RGB Orange = RGB( 101, 2, 103 );

// need the list so we can pick random ones
RGB *Colors[] = { &Purple, &Orange, &Yellow };
int ColorLen = arraysize(Colors);

// Fade for $FadeTime + rand($FadeVariableTime)
long FadeTime = 3000;
long FadeVariableTime = 1000;

void setup() {
  Serial.begin(9600);
  print("Setup led-thingy\n");

  print( "Setup array of Groups, from colors:\n" );
  for (int i=0; i<ColorLen; i++) {
    // print "Color i ",Colors[i];
    }
  }

/*


    say "Random start:";
    for(my $i=0; $i<$GroupSize; $i++) {
        my %a_group = ( from => $ARGV[0] // int(rand($ColorLen)), end_at => undef, to => undef, duration => undef ); 
        choose_new_to( \%a_group );

        # my %last;
        # %last = %{ $Colors[$a_group{'from'}] };
        # $a_group{'last'} = \%last;
        say Dumper(\%a_group);
        say "Setup group [$i] ",$Colors[$a_group{'from'}]," -> ",$Colors[$a_group{'to'}];
        $Groups[$i] = \%a_group;

        display_group($i, $Colors[ $a_group{'from'} ] );
        say " fades for ",$a_group{'duration'};
        }

    say "setup done";
    say "-----";
    }

sub xloop() {
  say "|",ard_map(11, 0,3794, 100,0),"|";
  die;
  }
*/

void loop() {
  }

/*
sub loop() {
    state $loopct;
    exit(1) if $LOOPLIMIT && millis() > $LOOPLIMIT;

    say "\nEACH GROUP @",millis();
    for (my $group_i=0; $group_i < $GroupSize; $group_i++) {
      my $group = $Groups[$group_i];
      my $now = millis();
      if ($group->{'end_at'} < $now) {
        # new to/end_at
        $group->{'from'} = int(rand($ColorLen)); 
        choose_new_to( $group );
        die "Reset [$group_i] ",$Colors[$group->{'from'}]," -> ",$Colors[$group->{'to'}];
        next;
        }

      # we are wasteful, we recalculate everybody each time, AND set them each time
      my $nextrgb;
      # linear_next_rgb($group_i, $now, \$nextrgb);
      $nextrgb = edgepath_next_rgb($group_i, $now);
      display_group($group_i, $nextrgb);
      say $group_i;
      }
    delay(100);
    }

sub choose_new_to {
    # Assumes 'from' is set, chooses a random 'to' and 'end_at'
    my ($group) = @_;
    $group->{'end_at'} = millis() + $FadeTime + int(rand($FadeVariableTime));
    $group->{'duration'} = $group->{'end_at'}-millis();
    # 'to' is one of the _other_ colors, so add 1|2 and do modulus to force it right
    $group->{'to'} = $ARGV[1] // ((int(rand($ColorLen-1)) + 1 + $group->{'from'}) % $ColorLen);
    $group->{'corner'} = undef;
    }

sub display_group {
    # Make the led(s) show the value
    my ($groupi, $rgb) = @_;
    say "display group [$groupi] to ",$rgb;
    # convert from a group_index to a 0..index
    my $from = $groupi * $GroupSize;
    my $to = $from + $GroupSize;
    # Set GroupSize leds:
    for (my $neo_i=$from; $neo_i<$to; $neo_i++) {
        say "== [$neo_i] $rgb @ ",millis();
        }
    }

sub edgepath_next_rgb {
  my ($group_i, $end_at_time) = @_;
  # We want the "fade" (aka gradient) to follow the closest edges of the rgb triangle
  # So, 'from' is closest to some edge.
  # And, 'to' is closest to some edge.
  # The direction of the first path is towards the closes "corner" of from/to
  # So, move proportionally along the distance of the first edge, then the second.
 
  my $group = $Groups[$group_i];

  my $from = $Colors[ $group->{'from'} ];
  my $to = $Colors[ $group->{'to'} ];
  if (!defined $group->{'corner'}) {
    # calc and cache the corner
    say "calc $from -> $to";
    my $corner = $from->clone;
    $corner->along_our_min_to_their_min($to);
    say "  $from -> $corner -> $to";
    $group->{'corner'} = $corner;
    say "Edge[$group_i] $from -> ",$group->{'corner'}," -> $to"; 
    }

  # Now we know $now, and $end_at, $duration, and from->corner->to
  # so, just calc the proper point along those 2 line segments
  my $gradient_color = proportional_color_along( $group,  $end_at_time );
  say "  next rgb $gradient_color @ $end_at_time";

=off
  # "fade" to the first point: the "nearest" corner
  # which is the minimum_component of 'to', 
  # this is || operator
  first = min_component(to), min_component(from)==ident, other is +/- same dist as first dim dist
    find the min_component of (to), calc distance = to[i]-from[i]
    first = from - dist for all components
    find the min_component(from), first[i] = from[i]
  second = to
  total dist = from->first->second # distances are simply b-a
  sofar = map total-distance:duration
  if sofar <= first distance, 
    move linearly from->first per sofar/total-dist
  else
    move linearly first->second per (sofar - first-dist)/second-dist
=cut 
  $gradient_color;
  }

sub proportional_color_along {
  my ($group, $now) = @_;

  my $from = $Colors[ $group->{'from'} ];
  my $to = $Colors[ $group->{'to'} ];

  say "  prop color of $from -> ",$group->{'corner'}," -> $to";
  my $distance_to_corner = $from->distance($group->{'corner'});
  my $total_len = $distance_to_corner + $group->{'corner'}->distance($to);
  my $duration = $group->{'duration'};
  my $elapsed_t = $duration - ($group->{'end_at'} - $now); ## FIXME: if end_at < $now, then calc next 'to'
  my $distance_to_now = $total_len * $elapsed_t / $duration; # i.e. dist proportional to elapsed time
  say "    $total_len : $distance_to_now = $duration : $elapsed_t";

  my $next_rgb = RGB->new;
  if ($distance_to_now <= $distance_to_corner) {
    $next_rgb->proportional_rgb( $from, $group->{'corner'}, $distance_to_now, $distance_to_corner);
    }
  elsif ($distance_to_now <= $total_len) {
    my $distance_from_corner = $total_len - $distance_to_corner;
    $next_rgb->proportional_rgb( $group->{'corner'}, $to, $distance_to_now-$distance_to_corner, $distance_from_corner);
    }
  else {
    die "past!";
    }
  return $next_rgb;
  }

sub linear_next_rgb {
  # Figures the rgb for time $end_at
  my ($group_i, $end_at, $rez) = @_;
  my $group = $Groups[ $group_i ];
  my $target_end_at = $group->{'end_at'};
  my $duration = $group->{'duration'};
  $rez->red = linear_value_at($end_at, $target_end_at, $duration, $Colors[ $group->{'from'} ]->red, $Colors[ $group->{'to'} ]->red);
  $rez->green = linear_value_at($end_at, $target_end_at, $duration, $Colors[ $group->{'from'} ]->green, $Colors[ $group->{'to'} ]->green);
  $rez->blue = linear_value_at($end_at, $target_end_at, $duration, $Colors[ $group->{'from'} ]->blue, $Colors[ $group->{'to'} ]->blue);
  }

sub linear_value_at {
  # figure the value for one of rgb. just linear.
  my ($end_at, $target_end_at, $duration, $from, $to) = @_;
  
  # Nothing to do if from==to
  if ($from==$to) { return $from; };

  my $time_left = $target_end_at - $end_at;

  # No duration left
  if ($time_left <= 0) {
    return $to;
    }

  my $rez =  ard_map($duration-$time_left, 0,$duration, $from, $to, );
  say "  $end_at: [0,$duration]->[$from,$to]( $duration-$time_left=",$duration-$time_left," ) = $rez";
  return $rez;
  }


####
sub millis {
    state $start;
    $start = time() if (!$start) ;

    int((time() - $start) * 1000);
    }

sub ard_map {
  my ($x, $in_min, $in_max, $out_min, $out_max) = @_;
  # say "  [$in_min,$in_max]->[$out_min,$out_max]($x)";
  return int( ((($x - $in_min) * ($out_max - $out_min)) / ($in_max - $in_min)) + $out_min );
  }

sub delay {
  my ($delay) = @_;
  my $start = millis();
  while ($start + $delay >= millis()) {}
  say "delayed $delay ",millis();
  }

$LOOPLIMIT=shift @ARGV;
*/
