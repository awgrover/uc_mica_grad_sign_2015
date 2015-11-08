class RGB {
  // use overload '""' => 'stringify';

  public:
    RGB(byte r, byte g, byte b) : v_red(r), v_green(g), v_blue(b) {}
    // ~RGB(); // FIXME: private for arduino, unimpl
    byte red() { return this->v_red; }
    byte green() { return this->v_green; }
    byte blue() { return this->v_blue; }
  private:
    byte v_red;  
    byte v_green;  
    byte v_blue;  


  /*
  sub clone {
    my $self=shift;
    RGB->new( red=>$self->red, blue=>$self->blue, green=>$self->green);
    }

  sub stringify {
    my $self=shift;
    sprintf "<0x%x>(%d, %d, %d)",Scalar::Util::refaddr($self),$self->red,$self->green,$self->blue;
    }

  sub brightness {
    my $self=shift;
    sqrt($self->red**2 + $self->green**2 + $self->blue**2);
    }

  sub along_our_min_to_their_min {
    # keep our min the same, update some component to their min, and set the remaining one to "avg brightness"
    my $self=shift;
    my ($b) = @_;
    say "  along $self to nearest corner with $b";

    if ($self->red <= $self->blue && $self->red <= $self->green) {
      say "    our red";
      if ($b->blue <= $b->green) {
        my $from_brightness = $self->brightness; # aka "from" before we update
        my $want_brightness = ($from_brightness + $b->brightness)/2;
        say "    (to blue) @ brightness $from_brightness/",$b->brightness," = $want_brightness";
        $self->blue($b->blue); # b's min
        $self->green( sqrt(abs($want_brightness**2 - $self->red**2 - $self->blue**2)) ); # "avg" brightness
        }
      else {
        say "    (to green)";
        $self->green($b->green);
        $self->blue( sqrt( (($self->blue+$b->blue)/2)^2 + (($self->green+$b->green)/2)^2) );
        }
      }
    elsif ($self->blue <= $self->red && $self->blue <= $self->green) {
      say "    blue";
      if ($b->red <= $b->green) {
        say "    (red)";
        $self->red($b->red);
        $self->green( sqrt( (($self->red+$b->red)/2)^2 + (($self->green+$b->green)/2)^2) );
        }
      }

    say "    corner: $self";
    }


  sub just_min_component {
    # update ourselves with the min component of $b
    my $self=shift;
    my ($b) = @_;

    say "  min of $b into $self...";
    # could optimize this
    if ($b->red <= $b->blue && $b->red <= $b->green) {
      say "    red";
      $self->red($b->red);
      }
    if ($b->blue <= $b->red && $b->blue <= $b->green) {
      say "    blue";
      $self->blue($b->blue);
      }
    if ($b->green <= $b->red && $b->green <= $b->blue) {
      say "    green";
      $self->green($b->green);
      }
    say "    => $self";
    return $self;
    }  
  
  sub distance {
    # distance to $b
    my $self=shift;
    my ($other) = @_;

    my $r = abs($other->red - $self->red);
    my $g = abs($other->green - $self->green);
    my $b = abs($other->blue - $self->blue);
    sqrt( $b**2 + $g**2 + $b**2);
    }
  
  sub proportional_rgb {
    my $self=shift;
    my ($from, $to, $delta_d, $total_dist) = @_;
    $self->red( ::ard_map($delta_d, 0, $total_dist, $from->red, $to->red) );
    $self->green( ::ard_map($delta_d, 0, $total_dist, $from->green, $to->green) );
    $self->blue( ::ard_map($delta_d, 0, $total_dist, $from->blue, $to->blue) );
    }
  */
  };

