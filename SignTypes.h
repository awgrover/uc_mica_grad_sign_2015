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

  private:
    byte v_red; // -1 means "not calc'd yet"
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
  (unsigned int) sqrt( (float)r*r + (float)g*g + (float)b*b);
  }
  
void RGB::along_our_min_to_their_min(RGB &other) {
  // keep our min the same, update some component to their min, and set the remaining one to "avg brightness"
  print( "  along ");print(this);print(" to nearest corner with other");println();

  if (this->red() <= this->blue() && this->red() <= this->green()) {
    print( "    our red");println();
    if (other.blue() <= other.green()) {
      byte from_brightness = this->brightness(); // aka "from" before we update
      byte want_brightness = (from_brightness + other.brightness())/2;
      print( "    (to blue) @ brightness "); print(from_brightness);print("/");print(other.brightness());print(" = ");print(want_brightness);println();
      this->blue(other.blue()); // other's min
      this->green( sqrt(abs(pow((float)want_brightness,2) - (pow((float)this->red(),2) - pow((float)this->blue(),2))) )); // "avg" brightness
      }
    else {
      print( "    (to green)");println();
      this->green(other.green());
      // this->blue( sqrt( ((float)(this->blue()+other.blue())/2)^2 + ((float)(this->green()+other.green())/2)^2) );
      }
    }
  else if (this->blue() <= this->red() && this->blue() <= this->green()) {
    print( "    blue");
    if (other.red() <= other.green()) {
      print( "    (red)");
      this->red(other.red());
      this->green( sqrt( ((this->red()+other.red())/2)^2 + ((this->green()+other.green())/2)^2) );
      }
    }

  print( "    corner: this");println();
  }

byte RGB::brightness() {
  (byte) sqrt(pow((float)this->red(),2) + pow((float)this->green(),2) + pow((float)this->blue(),2));
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
    print(rgb.blue()),print(")\n");
  }

void print(RGB *rgb) { print(*rgb); }

