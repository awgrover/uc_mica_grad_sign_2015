// #define debug_output
#ifdef debug_output
  #define debug_output2
  #define debug(x) print(x)
  #define debugln() println()
#else
  #define debug(x)
  #define debugln()
#endif
#ifdef debug_output2
  #define debug2(x) print(x)
  #define debugln2() println()
#else
  #define debug2(x)
  #define debugln2()
#endif

class RGB;

void print(RGB &rgb);
void print(RGB *rgb);
  
class RGB {
  // use overload '""' => 'stringify';

  public:
    RGB() : v_red(-1), v_green(0), v_blue(0) {}
    RGB(byte r, byte g, byte b) : v_red(r), v_green(g), v_blue(b) {}
    // ~RGB(); // FIXME: private for arduino, unimpl
    byte red() { return this->v_red; }
    void red(int v) { this->v_red=v; }
    byte green() { return this->v_green; }
    void green(int v) { this->v_green=v; }
    byte blue() { return this->v_blue; }
    void blue(int v) { this->v_blue=v; }

    void along_our_min_to_their_min(RGB &other);
    unsigned int distance(RGB &other);
    void proportional_rgb(RGB &from, RGB &to, unsigned int delta_d, unsigned int total_dist);
    uint32_t as_uint() { 
      /*
      print("  rgb("); Serial.print( this->red(), HEX); print(","); Serial.print(this->green(), HEX);print(","); Serial.print(this->blue(), HEX);
        print(" uint ");Serial.print(
          ((uint32_t)this->red()<<16) + ((uint32_t)this->green()<<8) + (uint32_t)this->blue()
          ,HEX);
        println();
      */
      return ((uint32_t)this->red()<<16) + ((uint32_t)this->green()<<8) + (uint32_t)this->blue();
      }

  private:
    byte v_red; // 255 means "not calc'd yet", a bit hinky
    byte v_green;  
    byte v_blue;  

    byte brightness();

  };

void RGB::proportional_rgb(RGB &from, RGB &to, unsigned int delta_d, unsigned int total_dist) {
  // adjust us to be proportional, like map()
  this->red( map(delta_d, 0, total_dist, from.red(), to.red()) );
  this->green( map(delta_d, 0, total_dist, from.green(), to.green()) );
  this->blue( map(delta_d, 0, total_dist, from.blue(), to.blue()) );
  }

unsigned int RGB::distance(RGB &other) {
  // distance to other

  byte r = abs(other.red() - this->red());
  byte g = abs(other.green() - this->green());
  byte b = abs(other.blue() - this->blue());
  return (unsigned int) sqrt( (float)r*r + (float)g*g + (float)b*b);
  }
  
void RGB::along_our_min_to_their_min(RGB &other) {
  // keep our min the same, update some component to their min, and set the remaining one to "avg brightness"
  debug( "  along ");debug(this);debug(" to nearest corner with other");debugln();

  byte from_brightness = this->brightness(); // aka "from" before we update
  byte want_brightness = (from_brightness + other.brightness())/2;

  // same pattern 3 times. sigh
  if (this->red() <= this->blue() && this->red() <= this->green()) {
    debug( "    our red");debugln();
    if (other.blue() <= other.green()) {
      debug( "    (to blue) @ brightness "); debug(from_brightness);debug("/");debug(other.brightness());debug(" = ");debug(want_brightness);debugln();
      this->blue(other.blue()); // other's min
      this->green( sqrt(abs((float)want_brightness*want_brightness - pow((float)this->red(),2) - pow((float)this->blue(),2))) ); // "avg" brightness
      }
    else {
      debug( "    (to green) @ brightness "); debug(from_brightness);debug("/");debug(other.brightness());debug(" = ");debug(want_brightness);debugln();
      this->green(other.green());
      this->blue( sqrt(abs((float)want_brightness*want_brightness - pow((float)this->red(),2) - pow((float)this->green(),2))) ); // "avg" brightness
      }
    }

  else if (this->blue() <= this->red() && this->blue() <= this->green()) {
    debug( "    our blue @ brightness "); debug(from_brightness);debug("/");debug(other.brightness());debug(" = ");debug(want_brightness);debugln();
    if (other.red() <= other.green()) {
      debug( "    (to red) @ brightness "); debug(from_brightness);debug("/");debug(other.brightness());debug(" = ");debug(want_brightness);debugln();
      this->red(other.red()); // other's min
      this->green( sqrt(abs((float)want_brightness*want_brightness - pow((float)this->red(),2) - pow((float)this->blue(),2))) ); // "avg" brightness
      }
    else {
      debug( "    (to green) @ brightness "); debug(from_brightness);debug("/");debug(other.brightness());debug(" = ");debug(want_brightness);debugln();
      this->green(other.green()); // other's min
      this->red( sqrt(abs((float)want_brightness*want_brightness - pow((float)this->green(),2) - pow((float)this->blue(),2))) ); // "avg" brightness
      }
    }

  else {
    debug("    our green\n");
    if (other.red() <= other.blue()) {
      debug( "    (to red) @ brightness "); debug(from_brightness);debug("/");debug(other.brightness());debug(" = ");debug(want_brightness);debugln();
      this->red(other.red()); // other's min
      this->blue( sqrt(abs((float)want_brightness*want_brightness - pow((float)this->green(),2) - pow((float)this->red(),2))) ); // "avg" brightness
      }
    else {
      debug( "    (to blue) @ brightness "); debug(from_brightness);debug("/");debug(other.brightness());debug(" = ");debug(want_brightness);debugln();
      this->blue(other.blue());
      this->red( sqrt(abs((float)want_brightness*want_brightness - pow((float)this->blue(),2) - pow((float)this->green(),2))) ); // "avg" brightness
      }
    }

  debug( "    corner: ");debug(this);debugln();
  }

byte RGB::brightness() {
  return (byte) sqrt(pow((float)this->red(),2) + pow((float)this->green(),2) + pow((float)this->blue(),2));
  }

struct group_struct {
  // We use indexes into Colors for the Groups, so that we can pick new random "to" later.
  byte from; // Color[i]
  byte to; // Color[i]
  long end_at; // end millis
  long duration; // how long a cyle is millis
  RGB corner;
  };

// print

void print(RGB &rgb) {
  print("<");printw((int) &rgb, HEX);print(">(");
    print(rgb.red()),print(", ");
    print(rgb.green()),print(", ");
    print(rgb.blue()),print(")");
  }

void print(RGB *rgb) { print(*rgb); }

// void print(uint16_t x) { print("#"); Serial.print( (unsigned int) x); }

