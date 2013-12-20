module spike() {
  translate([-10,-15,10]) hull() {
    cube([20,30,1]);
    translate([9,14,30]) cylinder(2,2,0);
  }
}

module round_spike() {
  translate([0,0,10]) cylinder(30,10,0);
}

module star() {
  difference() {
    union() {
      // Create all the points
      for (i = [0 : 72 : 360]) {
        rotate(i, [1,0,0]) spike();
      }
      // Centre solid
      hull() {
        for (i = [0 : 72 : 360]) {
          rotate(i, [1,0,0]) translate([0,0,10]) linear_extrude(height=1) circle(r=10.5);
        }
      }
    }
    // The cut-out for the LED strip
    // Use either the following line...
    rotate([0,90,0]) translate([-4,-9,-20]) cube([8,18,40]);
    // ...or this union (if you want to double-up the star
    // LEDs to feed it back through the star)
    //union() {
    //  translate([0,0,6]) rotate([0,90,0]) translate([-2.5,-8,-20]) cube([5,16,40]);
    //  translate([0,0,-6]) rotate([0,90,0]) translate([-2.5,-8,-20]) cube([5,16,40]);
    //}
  }
}

// Now split it in half (and flip it) for easier 3D printing
rotate([0,90,0])
difference() {
 star();
 //translate([0,-50,-50]) cube([30,100,100]);
}
// Reference cylinder (to check we're the right side of the origin :-)
//cylinder(100,4,4);
